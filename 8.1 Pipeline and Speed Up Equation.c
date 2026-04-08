#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// ===================== TYPES =====================
typedef union { float f; uint32_t u; } FPU;
typedef struct { int sign, exp; uint32_t mant; } FPC;

// ===================== IEEE 754 FUNCTIONS =====================

FPC fp_split(float v) {
    FPU u = {.f = v};
    return (FPC){u.u >> 31, (u.u >> 23) & 0xFF, u.u & 0x7FFFFF};
}

float fp_join(FPC c) {
    FPU u = {.u = ((uint32_t)c.sign << 31) | ((uint32_t)c.exp << 23) | (c.mant & 0x7FFFFF)};
    return u.f;
}

void fp_binary(uint32_t v, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        putchar('0' + ((v >> i) & 1));
        if (i == 20 || i == 12 || i == 4) putchar(' ');
    }
}

void fp_print(float v, const char *label) {
    FPC c = fp_split(v);
    printf("  %-10s = %13.6f\n", label, v);
    printf("    Sign: %d | Exp: ", c.sign);
    fp_binary(c.exp, 8);
    printf(" (%+4d) | Mant: ", c.exp - 127);
    fp_binary(c.mant, 23);
    printf("\n");
}

// ===================== 5-STAGE FP PIPELINE =====================

float fp_pipeline(float a, float b, char op) {
    FPC ca = fp_split(a), cb = fp_split(b);
    int ea = ca.exp - 127, eb = cb.exp - 127;
    
    printf("\n  +==============================================================+\n");
    printf("  |        5-STAGE FLOATING POINT PIPELINE EXECUTION            |\n");
    printf("  +==============================================================+\n");
    printf("  |  Operation: %.6f %c %.6f\n", a, op, b);
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  IEEE 754 REPRESENTATION                                     |\n");
    printf("  +--------------------------------------------------------------+\n");
    fp_print(a, "Operand A");
    fp_print(b, "Operand B");
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  PIPELINE: [CMP] -> [ALN] -> [OPR] -> [NRM] -> [RND]        |\n");
    printf("  +--------------------------------------------------------------+\n");
    
    // STAGE 1: Compare
    int diff = ea - eb, swap = (diff < 0);
    if (swap) { FPC t = ca; ca = cb; cb = t; diff = -diff; }
    int rexp = swap ? eb : ea;
    
    printf("  |  S1-COMPARE: exp_a=%+d, exp_b=%+d, diff=%d, swap=%s\n", 
           ea, eb, diff, swap ? "yes" : "no");
    
    // STAGE 2: Align
    uint32_t ma = (1 << 23) | ca.mant;
    uint32_t mb = ((1 << 23) | cb.mant) >> diff;
    
    printf("  |  S2-ALIGN:   mant_large=%u, mant_small=%u (>>%d)\n", ma, mb, diff);
    
    // STAGE 3: Operate
    int eff_sub = (op == '+') ? (ca.sign != cb.sign) : (ca.sign == cb.sign);
    uint32_t mr; int sr;
    
    if (eff_sub) {
        mr = (ma >= mb) ? ma - mb : mb - ma;
        sr = (ma >= mb) ? ca.sign : !ca.sign;
    } else {
        mr = ma + mb;
        sr = ca.sign;
    }
    
    printf("  |  S3-OPERATE: %s, result=%u, sign=%c\n", 
           eff_sub ? "SUB" : "ADD", mr, sr ? '-' : '+');
    
    // STAGE 4: Normalize
    int shift = 0;
    if (mr == 0) rexp = -127;
    else if (mr >> 24) { mr >>= 1; rexp++; shift = -1; }
    else while (mr && !(mr >> 23)) { mr <<= 1; rexp--; shift++; }
    
    printf("  |  S4-NORMAL:  shift=%+d, exp=%+d, mant=%u\n", shift, rexp, mr);
    
    // STAGE 5: Round
    mr &= 0x7FFFFF;
    
    printf("  |  S5-ROUND:   final_mant=%u, final_exp=%d\n", mr, rexp + 127);
    
    // Result
    float result = fp_join((FPC){sr, rexp + 127, mr});
    float expected = (op == '+') ? a + b : a - b;
    
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  RESULT                                                      |\n");
    printf("  +--------------------------------------------------------------+\n");
    fp_print(result, "Result");
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  Pipeline: %15.6f                                    \n", result);
    printf("  |  Expected: %15.6f                                    \n", expected);
    printf("  |  Error:    %15.2e                                    \n", fabs(result - expected));
    printf("  +==============================================================+\n");
    
    return result;
}

// ===================== SPEEDUP ANALYSIS =====================

void speedup_analysis(int k, int n, int stalls) {
    int t_seq = k * n, t_pipe = k + n - 1 + stalls;
    float S = (float)t_seq / t_pipe;
    
    printf("\n  +==============================================================+\n");
    printf("  |               SPEEDUP EQUATION ANALYSIS                      |\n");
    printf("  +==============================================================+\n");
    printf("  |  PARAMETERS                                                  |\n");
    printf("  |    Stages (k) = %-4d   Operations (n) = %-5d  Stalls = %-3d  \n", k, n, stalls);
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  TIME CALCULATION                                            |\n");
    printf("  |    T_seq  = k x n         = %d x %d = %d cycles\n", k, n, t_seq);
    printf("  |    T_pipe = k + n - 1 + s = %d + %d - 1 + %d = %d cycles\n", k, n, stalls, t_pipe);
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  SPEEDUP FORMULA                                             |\n");
    printf("  |              k x n           %5d                            |\n", t_seq);
    printf("  |    S = --------------- = --------- = %.4f                   |\n", S);
    printf("  |         k + n - 1 + s       %5d                            |\n", t_pipe);
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  METRICS                                                     |\n");
    printf("  |    Speedup      = %.4f x                                    |\n", S);
    printf("  |    Efficiency   = %.2f %%                                    |\n", S / k * 100);
    printf("  |    Throughput   = %.4f ops/cycle                            |\n", (float)n / t_pipe);
    printf("  |    CPI          = %.4f cycles/op                            |\n", (float)t_pipe / n);
    printf("  |    Cycles Saved = %d                                        |\n", t_seq - t_pipe);
    printf("  |    Max Speedup  = %d (as n -> inf)                          |\n", k);
    printf("  +==============================================================+\n");
}

// ===================== SPEEDUP TABLE =====================

void speedup_table(int k) {
    int nv[] = {1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
    
    printf("\n  +==============================================================+\n");
    printf("  |              SPEEDUP TABLE (k = %d stages)                   |\n", k);
    printf("  +--------+----------+----------+----------+----------+---------+\n");
    printf("  |   n    |  T_seq   |  T_pipe  |  Speedup |   Eff.   | Savings |\n");
    printf("  +--------+----------+----------+----------+----------+---------+\n");
    
    for (int i = 0; i < 10; i++) {
        int n = nv[i], ts = k * n, tp = k + n - 1;
        float s = (float)ts / tp;
        printf("  | %6d | %8d | %8d | %8.4f | %7.2f%% | %6.1f%% |\n",
               n, ts, tp, s, s / k * 100, (1 - (float)tp / ts) * 100);
    }
    
    printf("  +--------+----------+----------+----------+----------+---------+\n");
    printf("  |   inf  |   inf    |   inf    | %8d |  100.00%% | 100.0%% |\n", k);
    printf("  +==============================================================+\n");
}

// ===================== PIPELINE DIAGRAM =====================

void pipeline_diagram(int k, int n) {
    const char *stg[] = {"IF","ID","EX","MM","WB","S6","S7","S8"};
    int stages = k > 8 ? 8 : k, ops = n > 10 ? 10 : n;
    int cycles = stages + ops - 1;
    int t_seq = k * n, t_pipe = k + n - 1;
    
    printf("\n  +==============================================================+\n");
    printf("  |              PIPELINE EXECUTION DIAGRAM                      |\n");
    printf("  +==============================================================+\n");
    printf("  |  Config: k=%d stages, n=%d operations\n", k, n);
    printf("  |  Stages: ");
    for (int i = 0; i < stages; i++) printf("%s%s", stg[i], i < stages - 1 ? "->" : "\n");
    printf("  +--------------------------------------------------------------+\n");
    
    // Header
    printf("  |      |");
    for (int c = 1; c <= cycles; c++) printf("C%-2d|", c);
    printf("\n  |  ----+");
    for (int c = 0; c < cycles; c++) printf("---+");
    printf("\n");
    
    // Grid
    for (int i = 0; i < ops; i++) {
        printf("  |  I%-2d |", i + 1);
        for (int c = 0; c < cycles; c++) {
            int s = c - i;
            printf("%-3s|", (s >= 0 && s < stages) ? stg[s] : " . ");
        }
        printf("\n");
    }
    
    // Active row
    printf("  |  ----+");
    for (int c = 0; c < cycles; c++) printf("---+");
    printf("\n  |  Act |");
    for (int c = 0; c < cycles; c++) {
        int act = 0;
        for (int i = 0; i < ops; i++) if (c - i >= 0 && c - i < stages) act++;
        printf(" %d |", act);
    }
    
    printf("\n  +--------------------------------------------------------------+\n");
    printf("  |  T_seq=%d  T_pipe=%d  Speedup=%.2fx  Eff=%.1f%%\n", 
           t_seq, t_pipe, (float)t_seq / t_pipe, ((float)t_seq / t_pipe) / k * 100);
    printf("  +==============================================================+\n");
}

// ===================== INPUT HELPERS =====================

void clear_buf() { int c; while ((c = getchar()) != '\n' && c != EOF); }

int get_int(const char *p, int min, int max) {
    int v;
    do {
        printf("  %s", p);
        if (scanf("%d", &v) != 1) { clear_buf(); v = min - 1; }
    } while (v < min || v > max);
    return v;
}

float get_float(const char *p) {
    float v;
    printf("  %s", p);
    while (scanf("%f", &v) != 1) { clear_buf(); printf("  %s", p); }
    return v;
}

char get_op() {
    char op;
    printf("  Enter operation (+/-): ");
    scanf(" %c", &op);
    return (op == '-') ? '-' : '+';
}

// ===================== MAIN =====================

int main() {
    printf("\n  ==============================================================\n");
    printf("      PIPELINED FLOATING POINT ARITHMETIC & SPEEDUP ANALYSIS\n");
    printf("  ==============================================================\n");
    printf("    Pipeline: [COMPARE]->[ALIGN]->[OPERATE]->[NORMALIZE]->[ROUND]\n");
    printf("    Speedup:  S = (k x n) / (k + n - 1 + stalls)\n");
    printf("  ==============================================================\n");
    
    int choice;
    do {
        printf("\n  +------------------------+\n");
        printf("  |       MAIN MENU        |\n");
        printf("  +------------------------+\n");
        printf("  | [1] FP Pipeline Demo   |\n");
        printf("  | [2] Speedup Analysis   |\n");
        printf("  | [3] Speedup Table      |\n");
        printf("  | [4] Pipeline Diagram   |\n");
        printf("  | [0] Exit               |\n");
        printf("  +------------------------+\n");
        
        choice = get_int("Select option: ", 0, 4);
        
        switch (choice) {
            case 1: {
                printf("\n  -- FLOATING POINT PIPELINE --\n");
                float a = get_float("Enter operand A: ");
                char op = get_op();
                float b = get_float("Enter operand B: ");
                fp_pipeline(a, b, op);
                break;
            }
            case 2: {
                printf("\n  -- SPEEDUP ANALYSIS --\n");
                int k = get_int("Enter stages (k) [1-10]: ", 1, 10);
                int n = get_int("Enter operations (n) [1-10000]: ", 1, 10000);
                int s = get_int("Enter stalls [0-1000]: ", 0, 1000);
                speedup_analysis(k, n, s);
                break;
            }
            case 3: {
                printf("\n  -- SPEEDUP TABLE --\n");
                int k = get_int("Enter stages (k) [1-10]: ", 1, 10);
                speedup_table(k);
                break;
            }
            case 4: {
                printf("\n  -- PIPELINE DIAGRAM --\n");
                int k = get_int("Enter stages (k) [1-8]: ", 1, 8);
                int n = get_int("Enter operations (n) [1-10]: ", 1, 10);
                pipeline_diagram(k, n);
                break;
            }
        }
    } while (choice != 0);
    
    printf("\n  Goodbye!\n\n");
    return 0;
}
