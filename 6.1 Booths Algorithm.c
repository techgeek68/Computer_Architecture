#include <stdio.h>
#include <stdlib.h>

#define NUM_BITS 5

// Global arrays for binary representations
int anumcp[NUM_BITS] = {0};     // Multiplier (A) - working copy (shifted during execution)
int bnum[NUM_BITS] = {0};       // Multiplicand (B) in binary
int bcomp[NUM_BITS] = {0};      // Two's complement of B (-B)
int pro[NUM_BITS] = {0};        // Product/Accumulator register (P)

// Helper function to print binary array
void print_binary(int arr[]) {
    for (int i = NUM_BITS - 1; i >= 0; i--) {
        printf("%d", arr[i]);
    }
}

// Helper function to compute two's complement (one's complement + 1)
void twos_complement(int src[], int dest[]) {
    int c = 1;
    for (int i = 0; i < NUM_BITS; i++) {
        int flipped = (src[i] == 0) ? 1 : 0;
        dest[i] = (flipped + c) % 2;
        c = (flipped + c) / 2;
    }
}

// Convert decimal to two's complement binary and compute -B
void convert_to_binary(int a, int b) {
    int a1 = abs(a);
    int b1 = abs(b);
    int temp[NUM_BITS] = {0};

    // Convert absolute values to binary
    for (int i = 0; i < NUM_BITS; i++) {
        anumcp[i] = a1 % 2;
        bnum[i] = b1 % 2;
        a1 /= 2;
        b1 /= 2;
    }

    // Compute two's complement of B
    twos_complement(bnum, bcomp);

    // If A is negative, convert anumcp to two's complement
    if (a < 0) {
        for (int i = 0; i < NUM_BITS; i++) temp[i] = anumcp[i];
        twos_complement(temp, anumcp);
    }

    // If B is negative, swap bnum and bcomp
    if (b < 0) {
        for (int i = 0; i < NUM_BITS; i++) {
            int t = bnum[i];
            bnum[i] = bcomp[i];
            bcomp[i] = t;
        }
    }
}

// Add a number (num[]) to the product register (pro[])
void add(int num[]) {
    int c = 0;
    for (int i = 0; i < NUM_BITS; i++) {
        int sum = pro[i] + num[i] + c;
        pro[i] = sum % 2;
        c = sum / 2;
    }

    // Display current state: P : Q
    print_binary(pro);
    printf(":");
    print_binary(anumcp);
}

// Arithmetic Right Shift on [P : Acp]
void arshift() {
    int sign_bit = pro[NUM_BITS - 1];
    int lsb_pro = pro[0];

    // Shift product register right, preserving sign bit
    for (int i = 0; i < NUM_BITS - 1; i++) {
        pro[i] = pro[i + 1];
    }
    pro[NUM_BITS - 1] = sign_bit;

    // Shift multiplier copy right, MSB gets LSB of product
    for (int i = 0; i < NUM_BITS - 1; i++) {
        anumcp[i] = anumcp[i + 1];
    }
    anumcp[NUM_BITS - 1] = lsb_pro;

    // Display shifted state
    printf("\nAR-SHIFT: ");
    print_binary(pro);
    printf(":");
    print_binary(anumcp);
}

// Convert 10-bit two's complement binary result to decimal
int binary_to_decimal() {
    int decimal = 0;

    // Calculate decimal value using bit shifting
    for (int i = 0; i < NUM_BITS; i++) {
        decimal += anumcp[i] << i;
        decimal += pro[i] << (i + NUM_BITS);
    }

    // Handle sign bit (MSB of pro)
    if (pro[NUM_BITS - 1] == 1) {
        decimal -= (1 << (NUM_BITS * 2));
    }

    return decimal;
}

int main() {
    int a, b, q = 0;

    // Input validation: 5-bit two's complement range is -16 to +15
    do {
        printf("Enter two numbers to multiply (range: -16 to 15):\n");
        printf("Enter A (multiplier):   ");
        scanf("%d", &a);
        printf("Enter B (multiplicand): ");
        scanf("%d", &b);

        if (a < -16 || a > 15 || b < -16 || b > 15) {
            printf("Error: Numbers must be in the range -16 to 15.\n\n");
        }
    } while (a < -16 || a > 15 || b < -16 || b > 15);

    printf("\nExpected product = %d x %d = %d\n", a, b, a * b);

    // Convert to binary and compute two's complement of B
    convert_to_binary(a, b);

    // Display binary representations
    printf("\nBinary Equivalents (%d-bit two's complement):\n", NUM_BITS);
    printf("A        = ");
    print_binary(anumcp);
    printf("  (Multiplier)\n");

    printf("B        = ");
    print_binary(bnum);
    printf("  (Multiplicand)\n");

    printf("-B (B'+1)= ");
    print_binary(bcomp);
    printf("  (Two's complement of B)\n");

    printf("\nInitial:  P = 00000, Q = ");
    print_binary(anumcp);
    printf(", Q-1 = 0\n");

    // Booth's Algorithm: iterate over each bit of the multiplier
    for (int i = 0; i < NUM_BITS; i++) {
        int q0 = anumcp[0];
        printf("\nStep %d (Q[0]=%d, Q-1=%d): ", i + 1, q0, q);

        if (q0 == q) {
            printf("No operation (same bits)");
        } else if (q0 == 1 && q == 0) {
            printf("P = P - B (Subtract)\nSUB B:    ");
            add(bcomp);
        } else {
            printf("P = P + B (Add)\nADD B:    ");
            add(bnum);
        }
        arshift();

        q = q0;
    }

    // Display final product
    printf("\n\nProduct (binary):  ");
    print_binary(pro);
    printf(" ");
    print_binary(anumcp);

    int decimal_result = binary_to_decimal();
    printf("\nProduct (decimal): %d\n", decimal_result);
    printf("Expected product:  %d\n", a * b);
    printf("Verification:      %s\n", (decimal_result == a * b) ? "MATCHED" : "MISMATCH");

    return 0;
}
