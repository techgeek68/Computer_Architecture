#include <stdio.h>

#define NUM_INSTRUCTIONS 4

// Pipeline Stage Simulations

// Stage 1: Fetch the instruction from memory
int fetch_instruction() {
    printf("  Stage 1 (IF): Fetching instruction from memory...\n");
    return 0x12345678;  // Simulated instruction opcode
}

// Stage 2: Decode instruction and read operands from register file
void decode_instruction(int instruction, float a, float b) {
    printf("  Stage 2 (ID): Decoding instruction (0x%X), reading operands: %.2f, %.2f\n",
           instruction, a, b);
}

// Stage 3: Execute operation (handled in the addition/subtraction functions)

// Stage 4: Write result to register file
void write_result_to_register_file(float result) {
    printf("  Stage 4 (WB): Writing result %.2f to register file.\n", result);
}


// Floating Point Pipelined Operations

void floating_point_addition(float a, float b, float *result) {
    printf("\n--- Pipelined Floating-Point Addition ---\n");

    // Stage 1: Fetch
    int instruction = fetch_instruction();

    // Stage 2: Decode and read operands
    decode_instruction(instruction, a, b);

    // Stage 3: Execute addition
    *result = a + b;
    printf("  Stage 3 (EX): Performing addition: %.2f + %.2f = %.2f\n", a, b, *result);

    // Stage 4: Write back
    write_result_to_register_file(*result);
}

void floating_point_subtraction(float a, float b, float *result) {
    printf("\n--- Pipelined Floating-Point Subtraction ---\n");

    // Stage 1: Fetch
    int instruction = fetch_instruction();

    // Stage 2: Decode and read operands
    decode_instruction(instruction, a, b);

    // Stage 3: Execute subtraction
    *result = a - b;
    printf("  Stage 3 (EX): Performing subtraction: %.2f - %.2f = %.2f\n", a, b, *result);

    // Stage 4: Write back
    write_result_to_register_file(*result);
}


// Speedup Equation Demonstration

void speedup_equation_demo(int k, int n) {
    float time_without_pipeline = (float)(k * n);
    float time_with_pipeline = (float)(k + n - 1);
    float speedup = time_without_pipeline / time_with_pipeline;

    printf("\n--- Speedup Equation ---\n");
    printf("Formula: S = (k * n) / (k + n - 1)\n");
    printf("Number of pipeline stages (k) = %d\n", k);
    printf("Number of instructions    (n) = %d\n", n);
    printf("Time without pipelining       = k * n = %d * %d = %.0f cycles\n", k, n, time_without_pipeline);
    printf("Time with pipelining          = k + n - 1 = %d + %d - 1 = %.0f cycles\n", k, n, time_with_pipeline);
    printf("Speedup S = %.0f / %.0f = %.2f\n", time_without_pipeline, time_with_pipeline, speedup);
    printf("Theoretical maximum speedup   = k = %d\n", k);
}

// Instruction Pipeline Simulation (Space Time Diagram)


void instruction_pipeline_demo(int k) {
    char *stages[] = {"IF", "ID", "EX", "MEM", "WB"};
    char *instructions[NUM_INSTRUCTIONS] = {"LOAD R1", "ADD R2", "STORE R3", "SUB R4"};

    // Use minimum of k and 5 (maximum defined stages)
    int num_stages = k < 5 ? k : 5;
    int total_cycles = NUM_INSTRUCTIONS + num_stages - 1;

    printf("\n--- Instruction Pipeline Simulation (Space-Time Diagram) ---\n");
    printf("Pipeline Stages: ");
    for (int i = 0; i < num_stages; i++) {
        printf("%s", stages[i]);
        if (i < num_stages - 1) printf(" -> ");
    }
    printf("\n\n");

    // Print header
    printf("%-12s", "");
    for (int c = 0; c < total_cycles; c++) {
        printf("C%-4d", c + 1);
    }
    printf("\n");

    // Print pipeline execution for each instruction
    for (int i = 0; i < NUM_INSTRUCTIONS; i++) {
        printf("%-12s", instructions[i]);
        for (int c = 0; c < total_cycles; c++) {
            int stage = c - i;
            if (stage >= 0 && stage < num_stages) {
                printf("%-5s", stages[stage]);
            } else {
                printf("%-5s", "---");
            }
        }
        printf("\n");
    }

    printf("\nTotal cycles (non-pipelined) = %d * %d = %d cycles\n",
           num_stages, NUM_INSTRUCTIONS, num_stages * NUM_INSTRUCTIONS);
    printf("Total cycles (pipelined)     = %d + %d - 1 = %d cycles\n",
           num_stages, NUM_INSTRUCTIONS, total_cycles);
}


// Main Function

int main() {
    float a, b;
    float result_addition, result_subtraction;
    int k, n;
    char op;
    int choice;

    printf("============================================\n");
    printf("  PIPELINING LAB: Speedup Equation and\n");
    printf("  Floating-Point Operations with Pipelining\n");
    printf("============================================\n");

    // --- Floating-Point Pipelined Operations ---
    printf("\nEnter two floating-point operands:\n");
    printf("Operand a: ");
    scanf("%f", &a);
    printf("Operand b: ");
    scanf("%f", &b);

    // Perform pipelined addition
    floating_point_addition(a, b, &result_addition);
    printf("  Final Result: a + b = %.2f + %.2f = %.2f\n", a, b, result_addition);

    // Perform pipelined subtraction
    floating_point_subtraction(a, b, &result_subtraction);
    printf("  Final Result: a - b = %.2f - %.2f = %.2f\n", a, b, result_subtraction);

    // --- Speedup Equation ---
    printf("\nEnter the number of pipeline stages (k): ");
    scanf("%d", &k);
    printf("Enter the number of instructions (n): ");
    scanf("%d", &n);

    speedup_equation_demo(k, n);

    // --- Instruction Pipeline Simulation ---
    instruction_pipeline_demo(k);

    return 0;
}
