#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define VECTOR_SIZE 4
#define MATRIX_SIZE 2
#define NUM_INSTRUCTIONS 4

// Flynn's Classification Demonstration
void flynn_classification_demo() {
    printf("\nFlynn's Classification:\n");
    printf("1. SISD - Single Instruction, Single Data\n");
    printf("2. SIMD - Single Instruction, Multiple Data\n");
    printf("3. MISD - Multiple Instruction, Single Data\n");
    printf("4. MIMD - Multiple Instruction, Multiple Data\n");
}

// Parallel Processing Simulation
void *parallel_task(void *arg) {
    int task_id = *(int *)arg;
    printf("Parallel Task %d executing on a separate functional unit.\n", task_id);
    return NULL;
}

void parallel_processing_demo() {
    pthread_t threads[2];
    int task_ids[2];

    printf("\nEnter task IDs for parallel processing (2 tasks):\n");
    for (int i = 0; i < 2; i++) {
        printf("Enter task ID for Task %d: ", i + 1);
        scanf("%d", &task_ids[i]);
    }

    for (int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, parallel_task, &task_ids[i]);
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }
}

// Floating-Point Addition and Subtraction in Pipelining
void pipelined_operation(float a, float b, char op) {
    printf("\nPipelined Floating-Point Operation:\n");
    printf("Stage 1 (IF): Fetch operands %.2f and %.2f\n", a, b);
    printf("Stage 2 (ID): Decode operation (%s)\n", op == '+' ? "addition" : "subtraction");
    printf("Stage 3 (EX): Execute operation (result = %.2f)\n", op == '+' ? a + b : a - b);
    printf("Stage 4 (WB): Write result to memory/register\n");
}

// Speedup Equation Demonstration
void speedup_equation_demo(int n, int k) {
    float speedup = ((float)k * n) / (k + n - 1);
    printf("\nSpeedup Equation: S = (k * n) / (k + n - 1)\n");
    printf("S = (%d * %d) / (%d + %d - 1) = %.2f\n", k, n, k, n, speedup);
}

// Instruction Pipelining Simulation
void instruction_pipeline_demo() {
    char *stages[] = {"IF", "ID", "EX", "WB"};
    char *instructions[NUM_INSTRUCTIONS] = {"LOAD A", "ADD B", "STORE C", "MULT D"};

    printf("\nInstruction Pipeline Simulation:\n");
    printf("Stages: IF=Instruction Fetch, ID=Instruction Decode, EX=Execute, WB=Write Back\n\n");

    for (int cycle = 0; cycle < NUM_INSTRUCTIONS + 3; cycle++) {
        printf("Cycle %d: ", cycle + 1);
        for (int i = 0; i < NUM_INSTRUCTIONS; i++) {
            int stage = cycle - i;
            if (stage >= 0 && stage < 4) {
                printf("[%s -> %s] ", instructions[i], stages[stage]);
            }
        }
        printf("\n");
    }
}

// Vector Addition (SIMD Demonstration)
void vector_addition_demo() {
    float vecA[VECTOR_SIZE], vecB[VECTOR_SIZE], vecResult[VECTOR_SIZE];

    printf("\nVector Addition (SIMD Demonstration):\n");
    printf("Enter %d elements for Vector A:\n", VECTOR_SIZE);
    for (int i = 0; i < VECTOR_SIZE; i++) {
        printf("  A[%d]: ", i);
        scanf("%f", &vecA[i]);
    }

    printf("Enter %d elements for Vector B:\n", VECTOR_SIZE);
    for (int i = 0; i < VECTOR_SIZE; i++) {
        printf("  B[%d]: ", i);
        scanf("%f", &vecB[i]);
    }

    printf("\nApplying single ADD instruction across all elements (SIMD):\n");
    for (int i = 0; i < VECTOR_SIZE; i++) {
        vecResult[i] = vecA[i] + vecB[i];
        printf("  Result[%d] = %.2f + %.2f = %.2f\n", i, vecA[i], vecB[i], vecResult[i]);
    }
}

// Matrix Multiplication (SIMD Demonstration)
void matrix_multiplication_demo() {
    float matA[MATRIX_SIZE][MATRIX_SIZE], matB[MATRIX_SIZE][MATRIX_SIZE];
    float matResult[MATRIX_SIZE][MATRIX_SIZE];

    printf("\nMatrix Multiplication (%dx%d) - SIMD Demonstration:\n", MATRIX_SIZE, MATRIX_SIZE);

    printf("Enter elements for Matrix A (%dx%d):\n", MATRIX_SIZE, MATRIX_SIZE);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("  A[%d][%d]: ", i, j);
            scanf("%f", &matA[i][j]);
        }
    }

    printf("Enter elements for Matrix B (%dx%d):\n", MATRIX_SIZE, MATRIX_SIZE);
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("  B[%d][%d]: ", i, j);
            scanf("%f", &matB[i][j]);
        }
    }

    printf("\nPerforming Matrix Multiplication:\n");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matResult[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }

    printf("Resultant Matrix:\n");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("  Result[%d][%d] = %.2f", i, j, matResult[i][j]);
        }
        printf("\n");
    }
}

int main() {
    float a, b;
    char op;
    int n, k;

    // Flynn's Classification Demonstration
    flynn_classification_demo();

    // Parallel Processing Demo
    parallel_processing_demo();

    // Pipelined Floating-Point Operation
    printf("\nEnter two operands for pipelined operation:\n");
    printf("Operand 1: ");
    scanf("%f", &a);
    printf("Operand 2: ");
    scanf("%f", &b);
    printf("Enter operation ('+' for addition, '-' for subtraction): ");
    scanf(" %c", &op);

    pipelined_operation(a, b, op);

    // Speedup Equation
    printf("\nEnter number of instructions (n): ");
    scanf("%d", &n);
    printf("Enter number of pipeline stages (k): ");
    scanf("%d", &k);

    speedup_equation_demo(n, k);

    // Instruction Pipelining Demo
    instruction_pipeline_demo();

    // Vector Addition Demo
    vector_addition_demo();

    // Matrix Multiplication Demo
    matrix_multiplication_demo();

    return 0;
}
