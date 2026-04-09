#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef union { float f; uint32_t u; } FPU;
typedef struct { int sign, exp; uint32_t mant; } FPC;

FPC fp_split(float v) {
    FPU u = {.f = v};
    return (FPC){u.u >> 31, (u.u >> 23) & 0xFF, u.u & 0x7FFFFF};
}

float fp_join(FPC c) {
    FPU u = {.u = ((uint32_t)c.sign << 31) | ((uint32_t)c.exp << 23) | (c.mant & 0x7FFFFF)};
    return u.f;
}

void fp_print(float v, const char *label) {
    FPC c = fp_split(v);
    printf("%s = %.6f (Sign: %d, Exp: %+d, Mant: %u)\n", label, v, c.sign, c.exp - 127, c.mant);
}

float fp_pipeline(float a, float b, char op) {
    FPC ca = fp_split(a), cb = fp_split(b);
    int ea = ca.exp - 127, eb = cb.exp - 127;
    
    printf("\nOperation: %.6f %c %.6f\n", a, op, b);
    fp_print(a, "Operand A");
    fp_print(b, "Operand B");
    
    int diff = ea - eb, swap = (diff < 0);
    if (swap) { FPC t = ca; ca = cb; cb = t; diff = -diff; }
    int rexp = swap ? eb : ea;
    
    uint32_t ma = (1 << 23) | ca.mant;
    uint32_t mb = ((1 << 23) | cb.mant) >> diff;
    
    int eff_sub = (op == '+') ? (ca.sign != cb.sign) : (ca.sign == cb.sign);
    uint32_t mr; int sr;
    
    if (eff_sub) {
        mr = (ma >= mb) ? ma - mb : mb - ma;
        sr = (ma >= mb) ? ca.sign : !ca.sign;
    } else {
        mr = ma + mb;
        sr = ca.sign;
    }
    
    int shift = 0;
    if (mr == 0) rexp = -127;
    else if (mr >> 24) { mr >>= 1; rexp++; shift = -1; }
    else while (mr && !(mr >> 23)) { mr <<= 1; rexp--; shift++; }
    
    mr &= 0x7FFFFF;
    
    float result = fp_join((FPC){sr, rexp + 127, mr});
    float expected = (op == '+') ? a + b : a - b;
    
    fp_print(result, "Result");
    printf("Pipeline: %.6f, Expected: %.6f, Error: %.2e\n", result, expected, fabs(result - expected));
    
    return result;
}

void speedup_analysis(int k, int n, int stalls) {
    int t_seq = k * n, t_pipe = k + n - 1 + stalls;
    float S = (float)t_seq / t_pipe;
    
    printf("\nStages=%d, Operations=%d, Stalls=%d\n", k, n, stalls);
    printf("T_seq=%d, T_pipe=%d\n", t_seq, t_pipe);
    printf("Speedup=%.4f, Efficiency=%.2f%%, Throughput=%.4f ops/cycle\n", S, S / k * 100, (float)n / t_pipe);
    printf("CPI=%.4f, Cycles Saved=%d, Max Speedup=%d\n", (float)t_pipe / n, t_seq - t_pipe, k);
}

void speedup_table(int k) {
    int nv[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
    
    printf("\nSpeedup Table (k=%d stages)\n", k);
    printf("%6s %8s %8s %8s %8s %8s\n", "n", "T_seq", "T_pipe", "Speedup", "Eff%", "Save%");
    
    for (int i = 0; i < 10; i++) {
        int n = nv[i], ts = k * n, tp = k + n - 1;
        float s = (float)ts / tp;
        printf("%6d %8d %8d %8.4f %7.2f %7.1f\n", n, ts, tp, s, s / k * 100, (1 - (float)tp / ts) * 100);
    }
}

void pipeline_diagram(int k, int n) {
    const char *stg[] = {"IF","ID","EX","MM","WB","S6","S7","S8"};
    int stages = k > 8 ? 8 : k, ops = n > 10 ? 10 : n;
    int cycles = stages + ops - 1;
    
    printf("\nPipeline: k=%d stages, n=%d operations\n", k, n);
    
    for (int i = 0; i < ops; i++) {
        printf("I%-2d:", i + 1);
        for (int c = 0; c < cycles; c++) {
            int s = c - i;
            printf(" %-2s", (s >= 0 && s < stages) ? stg[s] : ".");
        }
        printf("\n");
    }
    printf("Speedup=%.2fx, Eff=%.1f%%\n", (float)(k * n) / (k + n - 1), ((float)(k * n) / (k + n - 1)) / k * 100);
}

void clear_buf() { int c; while ((c = getchar()) != '\n' && c != EOF); }

int get_int(const char *p, int min, int max) {
    int v;
    do {
        printf("%s", p);
        if (scanf("%d", &v) != 1) { clear_buf(); v = min - 1; }
    } while (v < min || v > max);
    return v;
}

float get_float(const char *p) {
    float v;
    printf("%s", p);
    while (scanf("%f", &v) != 1) { clear_buf(); printf("%s", p); }
    return v;
}

char get_op() {
    char op;
    printf("Operation (+/-): ");
    scanf(" %c", &op);
    return (op == '-') ? '-' : '+';
}

int main() {
    printf("Pipelined FP Arithmetic & Speedup Analysis\n");
    
    int choice;
    do {
        printf("\n1:FP Calc  2:Speedup  3:Table  4:Diagram  0:Exit\n");
        choice = get_int("Select: ", 0, 4);
        
        switch (choice) {
            case 1: {
                float a = get_float("Operand A: ");
                char op = get_op();
                float b = get_float("Operand B: ");
                fp_pipeline(a, b, op);
                break;
            }
            case 2: {
                int k = get_int("Stages (1-10): ", 1, 10);
                int n = get_int("Operations (1-10000): ", 1, 10000);
                int s = get_int("Stalls (0-1000): ", 0, 1000);
                speedup_analysis(k, n, s);
                break;
            }
            case 3:
                speedup_table(get_int("Stages (1-10): ", 1, 10));
                break;
            case 4: {
                int k = get_int("Stages (1-8): ", 1, 8);
                int n = get_int("Operations (1-10): ", 1, 10);
                pipeline_diagram(k, n);
                break;
            }
        }
    } while (choice);
    
    return 0;
}
