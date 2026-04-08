#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_BITS 5

// Global arrays for binary representations
int anum[NUM_BITS] = {0};       // Multiplier (A) - original bits
int anumcp[NUM_BITS] = {0};     // Multiplier (A) - working copy (shifted during execution)
int bnum[NUM_BITS] = {0};       // Multiplicand (B) in binary
int bcomp[NUM_BITS] = {0};      // Two's complement of B (-B)
int pro[NUM_BITS] = {0};        // Product/Accumulator register (P)
int res[NUM_BITS] = {0};        // Temporary result array for additions

// Convert decimal to two's complement binary and compute -B
void convert_to_binary(int a, int b) {
    int a1 = abs(a);
    int b1 = abs(b);
    int i, c, temp;
    int acomp[NUM_BITS] = {0};
    int com[NUM_BITS] = {1, 0, 0, 0, 0};  // Represents +1 for two's complement calculation

    // Convert absolute values to binary and compute one's complement simultaneously
    for (i = 0; i < NUM_BITS; i++) {
        anum[i] = a1 % 2;
        anumcp[i] = anum[i];
        bnum[i] = b1 % 2;

        // One's complement (flip bits)
        bcomp[i] = (bnum[i] == 0) ? 1 : 0;
        acomp[i] = (anum[i] == 0) ? 1 : 0;

        a1 /= 2;
        b1 /= 2;
    }

    // Compute two's complement of B: bcomp = one's complement of B + 1
    c = 0;
    for (i = 0; i < NUM_BITS; i++) {
        res[i] = bcomp[i] + com[i] + c;
        c = res[i] / 2;
        res[i] = res[i] % 2;
    }
    for (i = 0; i < NUM_BITS; i++) {
        bcomp[i] = res[i];
    }

    // If A is negative, convert anum to two's complement
    if (a < 0) {
        c = 0;
        for (i = 0; i < NUM_BITS; i++) {
            res[i] = acomp[i] + com[i] + c;
            c = res[i] / 2;
            res[i] = res[i] % 2;
        }
        for (i = 0; i < NUM_BITS; i++) {
            anum[i] = res[i];
            anumcp[i] = res[i];
        }
    }

    // If B is negative, swap bnum and bcomp
    // (bnum should hold the two's complement of negative B,
    //  and bcomp should hold -B which is the positive value)
    if (b < 0) {
        for (i = 0; i < NUM_BITS; i++) {
            temp = bnum[i];
            bnum[i] = bcomp[i];
            bcomp[i] = temp;
        }
    }
}

// Add a number (num[]) to the product register (pro[])
void add(int num[]) {
    int i, c = 0;

    for (i = 0; i < NUM_BITS; i++) {
        res[i] = pro[i] + num[i] + c;
        c = res[i] / 2;
        res[i] = res[i] % 2;
    }

    // Copy result back to product register and display
    for (i = 0; i < NUM_BITS; i++) {
        pro[i] = res[i];
    }

    // Display current state: P : Q
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", pro[i]);
    }
    printf(":");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", anumcp[i]);
    }
}

// Arithmetic Right Shift on [P : Acp]
void arshift() {
    int i;
    int sign_bit = pro[NUM_BITS - 1];  // Preserve the sign bit
    int lsb_pro = pro[0];               // LSB of P shifts into MSB of Q

    // Shift product register right, preserving sign bit
    for (i = 0; i < NUM_BITS - 1; i++) {
        pro[i] = pro[i + 1];
    }
    pro[NUM_BITS - 1] = sign_bit;  // Sign extension

    // Shift multiplier copy right, MSB gets LSB of product
    for (i = 0; i < NUM_BITS - 1; i++) {
        anumcp[i] = anumcp[i + 1];
    }
    anumcp[NUM_BITS - 1] = lsb_pro;

    // Display shifted state
    printf("\nAR-SHIFT: ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", pro[i]);
    }
    printf(":");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", anumcp[i]);
    }
}

// Convert 10-bit two's complement binary result to decimal
int binary_to_decimal() {
    int i, decimal = 0;
    int full_product[NUM_BITS * 2];

    // Combine P and Acp into full product
    for (i = 0; i < NUM_BITS; i++) {
        full_product[i] = anumcp[i];
    }
    for (i = 0; i < NUM_BITS; i++) {
        full_product[i + NUM_BITS] = pro[i];
    }

    // Check sign bit
    if (full_product[NUM_BITS * 2 - 1] == 1) {
        // Negative number: compute two's complement to find magnitude
        int comp[NUM_BITS * 2], c = 1;
        for (i = 0; i < NUM_BITS * 2; i++) {
            comp[i] = (full_product[i] == 0) ? 1 : 0;  // One's complement
        }
        for (i = 0; i < NUM_BITS * 2; i++) {
            comp[i] = comp[i] + c;
            c = comp[i] / 2;
            comp[i] = comp[i] % 2;
        }
        for (i = 0; i < NUM_BITS * 2; i++) {
            decimal += comp[i] * (int)pow(2, i);
        }
        decimal = -decimal;
    } else {
        for (i = 0; i < NUM_BITS * 2; i++) {
            decimal += full_product[i] * (int)pow(2, i);
        }
    }

    return decimal;
}

int main() {
    int a, b, i, q = 0;

    printf("============================================\n");
    printf("    BOOTH'S MULTIPLICATION ALGORITHM\n");
    printf("============================================\n");

    // Input validation: 5-bit two's complement range is -16 to +15
    do {
        printf("\nEnter two numbers to multiply (range: -16 to 15):\n");
        printf("Enter A (multiplier):   ");
        scanf("%d", &a);
        printf("Enter B (multiplicand): ");
        scanf("%d", &b);

        if (a < -16 || a > 15 || b < -16 || b > 15) {
            printf("Error: Numbers must be in the range -16 to 15.\n");
        }
    } while (a < -16 || a > 15 || b < -16 || b > 15);

    printf("\nExpected product = %d x %d = %d\n", a, b, a * b);

    // Convert to binary and compute two's complement of B
    convert_to_binary(a, b);

    // Display binary representations
    printf("\nBinary Equivalents (%d-bit two's complement):\n", NUM_BITS);
    printf("A        = ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", anum[i]);
    }
    printf("  (Multiplier)\n");

    printf("B        = ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", bnum[i]);
    }
    printf("  (Multiplicand)\n");

    printf("-B (B'+1)= ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", bcomp[i]);
    }
    printf("  (Two's complement of B)\n");

    // Initialize product register display
    printf("\nInitial:  P = 00000, Q = ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", anumcp[i]);
    }
    printf(", Q-1 = 0\n");

    printf("\n--- Booth's Algorithm Execution ---\n");

    // Booth's Algorithm: iterate over each bit of the multiplier
    for (i = 0; i < NUM_BITS; i++) {
        int q0 = anumcp[0];  // Examine LSB of current Q register
        printf("\nStep %d (Q[0]=%d, Q-1=%d): ", i + 1, q0, q);

        if (q0 == q) {
            // Q₀ == Q₋₁: No operation, just shift
            printf("No operation (same bits)");
            arshift();
        } else if (q0 == 1 && q == 0) {
            // Q₀=1, Q₋₁=0: Subtract multiplicand (P = P - B)
            printf("P = P - B (Subtract)\n");
            printf("SUB B:    ");
            add(bcomp);
            arshift();
        } else {
            // Q₀=0, Q₋₁=1: Add multiplicand (P = P + B)
            printf("P = P + B (Add)\n");
            printf("ADD B:    ");
            add(bnum);
            arshift();
        }

        q = q0;  // Update Q₋₁ with the bit we just examined
    }

    // Display final product in binary
    printf("\n\n--- Final Result ---\n");
    printf("Product (binary):  ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", pro[i]);
    }
    printf(" ");
    for (i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", anumcp[i]);
    }

    // Convert and display decimal result
    int decimal_result = binary_to_decimal();
    printf("\nProduct (decimal): %d\n", decimal_result);
    printf("Expected product:  %d\n", a * b);

    if (decimal_result == a * b) {
        printf("Verification:      MATCHED \n");
    } else {
        printf("Verification:      MISMATCH \n");
    }

    return 0;
}
