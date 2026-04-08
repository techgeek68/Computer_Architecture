#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ============================================
// Constants and Configuration
// ============================================

#define MAX_STAGES 10
#define MAX_INSTRUCTIONS 20
#define MAX_CYCLES 50

// ============================================
// Data Structures
// ============================================

typedef struct {
    int stages;
    int instructions;
    int stall_cycles;
    float clock_overhead;  // Latch delay as percentage
} PipelineConfig;

typedef struct {
    float speedup;
    float efficiency;
    float throughput;
    float cpi;
    int cycles_pipelined;
    int cycles_non_pipelined;
    int cycles_saved;
} PerformanceMetrics;

// ============================================
// Utility Functions
// ============================================

void print_header(const char *title) {
    printf("\n");
    printf("+");
    for (int i = 0; i < 66; i++) printf("=");
    printf("+\n");
    printf("|  %-64s|\n", title);
    printf("+");
    for (int i = 0; i < 66; i++) printf("=");
    printf("+\n");
}

void print_section(const char *title) {
    printf("\n");
    for (int i = 0; i < 68; i++) printf("-");
    printf("\n  %s\n", title);
    for (int i = 0; i < 68; i++) printf("-");
    printf("\n");
}

void print_separator() {
    printf("  +");
    for (int i = 0; i < 64; i++) printf("-");
    printf("+\n");
}

// ============================================
// Core Speedup Calculations
// ============================================

// Calculate ideal speedup (no stalls, no overhead)
float calculate_ideal_speedup(int k, int n) {
    if (k <= 0 || n <= 0) return 0;
    return (float)(k * n) / (float)(k + n - 1);
}

// Calculate speedup with stalls
float calculate_practical_speedup(int k, int n, int stalls) {
    if (k <= 0 || n <= 0) return 0;
    return (float)(k * n) / (float)(k + n - 1 + stalls);
}

// Calculate speedup with clock overhead
float calculate_speedup_with_overhead(int k, int n, float overhead_percent) {
    if (k <= 0 || n <= 0) return 0;
    float tau_ratio = 1.0 / (1.0 + overhead_percent / 100.0);
    float ideal = calculate_ideal_speedup(k, n);
    return ideal * tau_ratio;
}

// Calculate pipeline efficiency
float calculate_efficiency(int k, int n, int stalls) {
    if (k <= 0 || n <= 0) return 0;
    float speedup = calculate_practical_speedup(k, n, stalls);
    return (speedup / k) * 100.0;
}

// Calculate throughput (instructions per cycle)
float calculate_throughput(int k, int n, int stalls) {
    if (k <= 0 || n <= 0) return 0;
    return (float)n / (float)(k + n - 1 + stalls);
}

// Calculate CPI (Cycles Per Instruction)
float calculate_cpi(int k, int n, int stalls) {
    if (n <= 0) return 0;
    return (float)(k + n - 1 + stalls) / (float)n;
}

// Calculate all performance metrics
PerformanceMetrics calculate_all_metrics(PipelineConfig config) {
    PerformanceMetrics metrics;
    
    int k = config.stages;
    int n = config.instructions;
    int stalls = config.stall_cycles;
    
    metrics.cycles_non_pipelined = k * n;
    metrics.cycles_pipelined = k + n - 1 + stalls;
    metrics.cycles_saved = metrics.cycles_non_pipelined - metrics.cycles_pipelined;
    
    metrics.speedup = calculate_practical_speedup(k, n, stalls);
    metrics.efficiency = calculate_efficiency(k, n, stalls);
    metrics.throughput = calculate_throughput(k, n, stalls);
    metrics.cpi = calculate_cpi(k, n, stalls);
    
    return metrics;
}

// ============================================
// Speedup Equation Demonstrations
// ============================================

void demonstrate_speedup_derivation(int k, int n) {
    print_section("SPEEDUP EQUATION DERIVATION");
    
    printf("\n  Given:\n");
    printf("    * Pipeline stages (k) = %d\n", k);
    printf("    * Number of instructions (n) = %d\n", n);
    
    printf("\n  Step 1: Calculate Non Pipelined Execution Time\n");
    printf("    - Each instruction must complete all %d stages sequentially\n", k);
    printf("    - Time = k x n = %d x %d = %d cycles\n", k, n, k * n);
    
    printf("\n  Step 2: Calculate Pipelined Execution Time\n");
    printf("    - First instruction: takes k = %d cycles to complete\n", k);
    printf("    - Pipeline fill time: k - 1 = %d cycles\n", k - 1);
    printf("    - Remaining %d instructions: 1 cycle each = %d cycles\n", n - 1, n - 1);
    printf("    - Total = k + (n - 1) = %d + %d = %d cycles\n", k, n - 1, k + n - 1);
    
    printf("\n  Step 3: Calculate Speedup\n");
    printf("    S = T_non_pipelined / T_pipelined\n");
    printf("    S = (k x n) / (k + n - 1)\n");
    printf("    S = (%d x %d) / (%d + %d - 1)\n", k, n, k, n);
    printf("    S = %d / %d\n", k * n, k + n - 1);
    printf("    S = %.4f\n", calculate_ideal_speedup(k, n));
    
    printf("\n  Step 4: Theoretical Maximum (as n -> infinity)\n");
    printf("    S_max = lim(n->inf) [k / (1 + (k-1)/n)]\n");
    printf("    S_max = k / (1 + 0) = k = %d\n", k);
    
    printf("\n  Conclusion:\n");
    printf("    - Achieved speedup: %.4fx\n", calculate_ideal_speedup(k, n));
    printf("    - Maximum possible: %dx\n", k);
    printf("    - Percentage of max: %.2f%%\n", (calculate_ideal_speedup(k, n) / k) * 100);
}

void demonstrate_speedup_formulas(int k, int n, int stalls, float overhead) {
    print_section("COMPARISON OF SPEEDUP FORMULAS");
    
    float s_ideal = calculate_ideal_speedup(k, n);
    float s_practical = calculate_practical_speedup(k, n, stalls);
    float s_overhead = calculate_speedup_with_overhead(k, n, overhead);
    float s_combined = calculate_speedup_with_overhead(k, n, overhead) * 
                       ((float)(k + n - 1) / (float)(k + n - 1 + stalls));
    
    printf("\n  Configuration:\n");
    printf("    * Pipeline stages (k) = %d\n", k);
    printf("    * Instructions (n) = %d\n", n);
    printf("    * Stall cycles = %d\n", stalls);
    printf("    * Clock overhead = %.1f%%\n", overhead);
    
    printf("\n  +----------------------------------+------------------+----------+\n");
    printf("  | Formula                          | Equation         | Speedup  |\n");
    printf("  +----------------------------------+------------------+----------+\n");
    printf("  | 1. Ideal (no stalls/overhead)    | (k*n)/(k+n-1)    | %-8.4f |\n", s_ideal);
    printf("  | 2. With Stalls                   | (k*n)/(k+n-1+s)  | %-8.4f |\n", s_practical);
    printf("  | 3. With Clock Overhead           | S * (1/(1+oh))   | %-8.4f |\n", s_overhead);
    printf("  | 4. Combined (Stalls + Overhead)  | Practical Model  | %-8.4f |\n", s_combined);
    printf("  | 5. Theoretical Maximum           | k                | %-8d |\n", k);
    printf("  +----------------------------------+------------------+----------+\n");
    
    printf("\n  Performance Degradation Analysis:\n");
    printf("    - Loss due to pipeline fill/drain: %.2f%%\n", 
           100 - (s_ideal / k) * 100);
    printf("    - Additional loss due to stalls: %.2f%%\n", 
           (s_ideal - s_practical) / k * 100);
    printf("    - Additional loss due to overhead: %.2f%%\n", 
           (s_ideal - s_overhead) / k * 100);
    printf("    - Total loss from ideal (k=%d): %.2f%%\n", k,
           100 - (s_combined / k) * 100);
}

// ============================================
// Speedup vs Parameters Analysis
// ============================================

void analyze_speedup_vs_instructions(int k) {
    print_section("SPEEDUP vs NUMBER OF INSTRUCTIONS");
    
    printf("\n  Pipeline Depth (k) = %d stages\n", k);
    printf("\n  +--------+---------------+------------+------------+-----------+\n");
    printf("  |   n    | Time(cycles)  |  Speedup   | Efficiency | %% of Max  |\n");
    printf("  +--------+---------------+------------+------------+-----------+\n");
    
    int n_values[] = {1, 2, 5, 10, 20, 50, 100, 500, 1000, 10000};
    int num_values = 10;
    
    for (int i = 0; i < num_values; i++) {
        int n = n_values[i];
        int time_pipelined = k + n - 1;
        float speedup = calculate_ideal_speedup(k, n);
        float efficiency = (speedup / k) * 100;
        
        printf("  | %-6d | %-13d | %-10.4f | %-10.2f | %-9.2f |\n",
               n, time_pipelined, speedup, efficiency, efficiency);
    }
    printf("  +--------+---------------+------------+------------+-----------+\n");
    
    printf("\n    Observations:\n");
    printf("    1. Speedup increases with more instructions\n");
    printf("    2. Speedup asymptotically approaches k = %d\n", k);
    printf("    3. Efficiency = Speedup / k (expressed as percentage)\n");
    printf("    4. With n=%d instructions, efficiency reaches %.2f%%\n",
           10000, calculate_ideal_speedup(k, 10000) / k * 100);
}

void analyze_speedup_vs_stages(int n) {
    print_section("SPEEDUP vs NUMBER OF PIPELINE STAGES");
    
    printf("\n  Number of Instructions (n) = %d\n", n);
    printf("\n  +--------+---------------+------------+------------+----------+\n");
    printf("  |   k    | Time(cycles)  |  Speedup   | Max Poss.  | Achieved |\n");
    printf("  +--------+---------------+------------+------------+----------+\n");
    
    for (int k = 2; k <= 12; k += 2) {
        int time_pipelined = k + n - 1;
        float speedup = calculate_ideal_speedup(k, n);
        float percent = (speedup / k) * 100;
        
        printf("  | %-6d | %-13d | %-10.4f | %-10d | %-7.2f%% |\n",
               k, time_pipelined, speedup, k, percent);
    }
    printf("  +--------+---------------+------------+------------+----------+\n");
    
    printf("\n    Observations:\n");
    printf("    1. Deeper pipelines have higher maximum speedup potential\n");
    printf("    2. But achieved percentage decreases with depth (for fixed n)\n");
    printf("    3. Very deep pipelines need many instructions to be efficient\n");
    printf("    4. Optimal depth depends on typical instruction count\n");
}

void analyze_speedup_vs_stalls(int k, int n) {
    print_section("IMPACT OF STALLS ON SPEEDUP");
    
    printf("\n  Configuration: k = %d stages, n = %d instructions\n", k, n);
    printf("  Ideal speedup (0 stalls) = %.4f\n", calculate_ideal_speedup(k, n));
    
    printf("\n  +--------+--------+------------+------------+-------------+\n");
    printf("  | Stalls | Total  |  Speedup   | Efficiency | Loss from   |\n");
    printf("  |        | Cycles |            |            | Ideal       |\n");
    printf("  +--------+--------+------------+------------+-------------+\n");
    
    float ideal = calculate_ideal_speedup(k, n);
    int stall_values[] = {0, 1, 2, 5, 10, 20, 50, 100};
    int num_values = 8;
    
    for (int i = 0; i < num_values; i++) {
        int stalls = stall_values[i];
        int total_cycles = k + n - 1 + stalls;
        float speedup = calculate_practical_speedup(k, n, stalls);
        float efficiency = (speedup / k) * 100;
        float loss = ((ideal - speedup) / ideal) * 100;
        
        printf("  | %-6d | %-6d | %-10.4f | %-10.2f%% | %-11.2f%% |\n",
               stalls, total_cycles, speedup, efficiency, loss);
    }
    printf("  +--------+--------+------------+------------+-------------+\n");
    
    printf("\n    Observations:\n");
    printf("    1. Each stall cycle reduces overall speedup\n");
    printf("    2. Impact of stalls is more severe with fewer instructions\n");
    printf("    3. Stall reduction is critical for pipeline efficiency\n");
}

// ============================================
// Pipeline Visualization (Gantt Chart)
// ============================================

void visualize_pipeline_gantt(int k, int n) {
    print_section("PIPELINE EXECUTION GANTT CHART");
    
    // Limit display size for readability
    int display_n = (n > 8) ? 8 : n;
    int display_k = (k > 6) ? 6 : k;
    int total_cycles = display_n + display_k - 1;
    
    char *stage_names[] = {"IF", "ID", "EX", "M1", "M2", "WB"};
    char instr_labels[8][12] = {"I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8"};
    
    printf("\n  Configuration: %d stages, %d instructions", k, n);
    if (n > 8 || k > 6) printf(" (showing %d stages, %d instructions)", display_k, display_n);
    printf("\n\n");
    
    // Print cycle header
    printf("  %-8s", "Cycle:");
    for (int c = 1; c <= total_cycles; c++) {
        printf(" %-4d", c);
    }
    printf("\n");
    
    // Print separator
    printf("  ");
    for (int i = 0; i < 8 + total_cycles * 5; i++) printf("-");
    printf("\n");
    
    // Print each instruction's execution
    for (int i = 0; i < display_n; i++) {
        printf("  %-8s", instr_labels[i]);
        for (int c = 1; c <= total_cycles; c++) {
            int stage = c - i - 1;
            if (stage >= 0 && stage < display_k) {
                printf("[%-2s] ", stage_names[stage]);
            } else {
                printf(" ..  ");
            }
        }
        printf("\n");
    }
    
    // Print separator
    printf("  ");
    for (int i = 0; i < 8 + total_cycles * 5; i++) printf("-");
    printf("\n");
    
    // Print stage utilization per cycle
    printf("  %-8s", "Active:");
    for (int c = 1; c <= total_cycles; c++) {
        int active = 0;
        for (int i = 0; i < display_n; i++) {
            int stage = c - i - 1;
            if (stage >= 0 && stage < display_k) active++;
        }
        printf(" %-4d", active);
    }
    printf("\n");
    
    // Calculate and display metrics
    printf("\n  Execution Summary:\n");
    printf("    - Non-pipelined time: %d x %d = %d cycles\n", k, n, k * n);
    printf("    - Pipelined time: %d + %d - 1 = %d cycles\n", k, n, k + n - 1);
    printf("    - Speedup: %.4fx\n", calculate_ideal_speedup(k, n));
    printf("    - Cycles saved: %d cycles (%.1f%% reduction)\n", 
           k * n - (k + n - 1), 
           (float)(k * n - (k + n - 1)) / (k * n) * 100);
}

// ============================================
// Space-Time Diagram
// ============================================

void visualize_space_time_diagram(int k, int n) {
    print_section("SPACE-TIME DIAGRAM (RESOURCE UTILIZATION)");
    
    int display_n = (n > 10) ? 10 : n;
    int display_k = (k > 5) ? 5 : k;
    int total_cycles = display_n + display_k - 1;
    
    char *stage_names[] = {"IF ", "ID ", "EX ", "MEM", "WB "};
    
    printf("\n  Time (cycles) --->\n");
    printf("  Stage ");
    for (int c = 1; c <= total_cycles; c++) {
        printf("  %2d ", c);
    }
    printf("\n");
    
    printf("  ------+");
    for (int c = 1; c <= total_cycles; c++) {
        printf("----+");
    }
    printf("\n");
    
    // For each stage, show which instruction is using it
    for (int s = 0; s < display_k; s++) {
        printf("  %s  |", stage_names[s]);
        for (int c = 1; c <= total_cycles; c++) {
            int instr = c - s - 1;  // Which instruction is at this stage
            if (instr >= 0 && instr < display_n) {
                printf(" I%-2d|", instr + 1);
            } else {
                printf("    |");
            }
        }
        printf("\n");
    }
    
    printf("  ------+");
    for (int c = 1; c <= total_cycles; c++) {
        printf("----+");
    }
    printf("\n");
    
    printf("\n  Legend: I1, I2, ... = Instruction numbers\n");
    printf("  Observation: Each column shows all stages working in parallel\n");
}

// ============================================
// Throughput Analysis
// ============================================

void analyze_throughput(int k, int n, int stalls) {
    print_section("THROUGHPUT AND CPI ANALYSIS");
    
    float throughput = calculate_throughput(k, n, stalls);
    float cpi = calculate_cpi(k, n, stalls);
    float efficiency = calculate_efficiency(k, n, stalls);
    int total_time = k + n - 1 + stalls;
    
    printf("\n  Configuration:\n");
    printf("    - Pipeline stages (k) = %d\n", k);
    printf("    - Instructions (n) = %d\n", n);
    printf("    - Stall cycles = %d\n", stalls);
    printf("    - Total execution time = %d cycles\n", total_time);
    
    printf("\n  Throughput Calculation:\n");
    printf("    Throughput = Instructions / Total Cycles\n");
    printf("    Throughput = %d / %d\n", n, total_time);
    printf("    Throughput = %.4f instructions/cycle\n", throughput);
    printf("    (Maximum possible = 1.0 instructions/cycle)\n");
    
    printf("\n  CPI (Cycles Per Instruction) Calculation:\n");
    printf("    CPI = Total Cycles / Instructions\n");
    printf("    CPI = %d / %d\n", total_time, n);
    printf("    CPI = %.4f cycles/instruction\n", cpi);
    printf("    (Ideal CPI = 1.0 cycles/instruction)\n");
    
    printf("\n  +---------------------------+----------------+\n");
    printf("  | Metric                    | Value          |\n");
    printf("  +---------------------------+----------------+\n");
    printf("  | Throughput                | %.4f inst/cyc |\n", throughput);
    printf("  | CPI                       | %.4f cyc/inst |\n", cpi);
    printf("  | Efficiency                | %.2f%%          |\n", efficiency);
    printf("  | Peak Throughput           | 1.0 inst/cyc   |\n");
    printf("  | Ideal CPI                 | 1.0 cyc/inst   |\n");
    printf("  +---------------------------+----------------+\n");
}

// ============================================
// Comprehensive Performance Report
// ============================================

void generate_performance_report(PipelineConfig config) {
    print_section("COMPREHENSIVE PERFORMANCE REPORT");
    
    PerformanceMetrics metrics = calculate_all_metrics(config);
    int k = config.stages;
    int n = config.instructions;
    
    printf("\n  PIPELINE CONFIGURATION\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Parameter                        | Value            |\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Pipeline Depth (k)               | %-16d |\n", k);
    printf("  | Number of Instructions (n)       | %-16d |\n", n);
    printf("  | Stall Cycles                     | %-16d |\n", config.stall_cycles);
    printf("  | Clock Overhead                   | %-15.1f%% |\n", config.clock_overhead);
    printf("  +----------------------------------+------------------+\n");
    
    printf("\n  TIMING ANALYSIS\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Metric                           | Cycles           |\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Non Pipelined Execution Time     | %-16d |\n", metrics.cycles_non_pipelined);
    printf("  | Pipelined Execution Time         | %-16d |\n", metrics.cycles_pipelined);
    printf("  | Cycles Saved                     | %-16d |\n", metrics.cycles_saved);
    printf("  | Time Reduction                   | %-15.2f%% |\n", 
           (float)metrics.cycles_saved / metrics.cycles_non_pipelined * 100);
    printf("  +----------------------------------+------------------+\n");
    
    printf("\n  PERFORMANCE METRICS\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Metric                           | Value            |\n");
    printf("  +----------------------------------+------------------+\n");
    printf("  | Speedup                          | %-16.4f |\n", metrics.speedup);
    printf("  | Theoretical Maximum Speedup      | %-16d |\n", k);
    printf("  | Pipeline Efficiency              | %-15.2f%% |\n", metrics.efficiency);
    printf("  | Throughput                       | %-12.4f i/c |\n", metrics.throughput);
    printf("  | CPI (Cycles Per Instruction)     | %-16.4f |\n", metrics.cpi);
    printf("  +----------------------------------+------------------+\n");
    
    printf("\n  INTERPRETATION\n");
    printf("  - The pipeline achieves %.2f%% of maximum possible speedup.\n", 
           (metrics.speedup / k) * 100);
    printf("  - On average, %.2f instructions complete per clock cycle.\n", metrics.throughput);
    printf("  - Each instruction takes %.2f clock cycles on average.\n", metrics.cpi);
    if (metrics.efficiency > 90) {
        printf("  - Efficiency is EXCELLENT (>90%%). Pipeline is well utilized.\n");
    } else if (metrics.efficiency > 70) {
        printf("  - Efficiency is GOOD (70-90%%). Some room for improvement.\n");
    } else if (metrics.efficiency > 50) {
        printf("  - Efficiency is MODERATE (50-70%%). Consider optimizations.\n");
    } else {
        printf("  - Efficiency is LOW (<50%%). Pipeline is underutilized.\n");
    }
}

// ============================================
// Interactive Speedup Calculator
// ============================================

void interactive_speedup_calculator() {
    print_section("INTERACTIVE SPEEDUP CALCULATOR");
    
    int k, n, stalls;
    float overhead;
    
    printf("\n  Enter pipeline parameters:\n");
    printf("    Number of stages (k): ");
    scanf("%d", &k);
    printf("    Number of instructions (n): ");
    scanf("%d", &n);
    printf("    Number of stall cycles: ");
    scanf("%d", &stalls);
    printf("    Clock overhead percentage: ");
    scanf("%f", &overhead);
    
    // Validate inputs
    if (k < 1) k = 1;
    if (k > MAX_STAGES) k = MAX_STAGES;
    if (n < 1) n = 1;
    if (stalls < 0) stalls = 0;
    if (overhead < 0) overhead = 0;
    
    PipelineConfig config = {k, n, stalls, overhead};
    
    printf("\n  CALCULATION RESULTS\n");
    printf("  ===================\n");
    
    // Show step-by-step calculation
    printf("\n  Step 1: Non-pipelined time = k x n = %d x %d = %d cycles\n", 
           k, n, k * n);
    printf("  Step 2: Pipelined time (ideal) = k + n - 1 = %d + %d - 1 = %d cycles\n",
           k, n, k + n - 1);
    printf("  Step 3: Pipelined time (with stalls) = %d + %d = %d cycles\n",
           k + n - 1, stalls, k + n - 1 + stalls);
    
    float ideal_speedup = calculate_ideal_speedup(k, n);
    float practical_speedup = calculate_practical_speedup(k, n, stalls);
    float with_overhead = calculate_speedup_with_overhead(k, n, overhead);
    
    printf("\n  Step 4: Speedup Calculations\n");
    printf("    - Ideal Speedup = %d / %d = %.4f\n", k * n, k + n - 1, ideal_speedup);
    printf("    - With Stalls = %d / %d = %.4f\n", k * n, k + n - 1 + stalls, practical_speedup);
    printf("    - With Overhead = %.4f x %.4f = %.4f\n", 
           ideal_speedup, 1.0 / (1.0 + overhead / 100.0), with_overhead);
    
    printf("\n  Step 5: Efficiency = Speedup / k = %.4f / %d = %.2f%%\n",
           practical_speedup, k, (practical_speedup / k) * 100);
    
    printf("\n  Step 6: Throughput = n / Total_Cycles = %d / %d = %.4f inst/cycle\n",
           n, k + n - 1 + stalls, calculate_throughput(k, n, stalls));
    
    printf("\n  Step 7: CPI = Total_Cycles / n = %d / %d = %.4f cycles/inst\n",
           k + n - 1 + stalls, n, calculate_cpi(k, n, stalls));
}

// ============================================
// Compare Multiple Configurations
// ============================================

void compare_configurations() {
    print_section("PIPELINE CONFIGURATION COMPARISON");
    
    // Define several configurations to compare
    PipelineConfig configs[] = {
        {4, 100, 0, 0},      // 4-stage, no stalls
        {5, 100, 0, 0},      // 5-stage, no stalls
        {6, 100, 0, 0},      // 6-stage, no stalls
        {5, 100, 10, 0},     // 5-stage, with stalls
        {5, 100, 0, 10},     // 5-stage, with overhead
        {5, 100, 10, 10}     // 5-stage, stalls + overhead
    };
    int num_configs = 6;
    
    char *descriptions[] = {
        "4-stage, ideal",
        "5-stage, ideal",
        "6-stage, ideal",
        "5-stage, 10 stalls",
        "5-stage, 10% overhead",
        "5-stage, stalls+overhead"
    };
    
    printf("\n  Comparing 6 different pipeline configurations (n=100 instructions):\n");
    printf("\n  +----------------------------+--------+----------+----------+--------+\n");
    printf("  | Configuration              | Stages | Speedup  | Effic.   | CPI    |\n");
    printf("  +----------------------------+--------+----------+----------+--------+\n");
    
    for (int i = 0; i < num_configs; i++) {
        PerformanceMetrics m = calculate_all_metrics(configs[i]);
        printf("  | %-26s | %-6d | %-8.4f | %-7.2f%% | %-6.4f |\n",
               descriptions[i], configs[i].stages, m.speedup, m.efficiency, m.cpi);
    }
    printf("  +----------------------------+--------+----------+----------+--------+\n");
    
    printf("\n Insights:\n");
    printf("    1. More stages increase maximum speedup but decrease efficiency\n");
    printf("    2. Stalls significantly impact performance\n");
    printf("    3. Clock overhead reduces effective speedup\n");
    printf("    4. Combined effects are multiplicative\n");
}

// ============================================
// Amdahl's Law Demonstration
// ============================================

void demonstrate_amdahls_law() {
    print_section("AMDAHL'S LAW AND PIPELINING");
    
    printf("\n  Amdahl's Law: S_overall = 1 / [(1-f) + f/k]\n");
    printf("  Where: f = fraction that can be pipelined, k = pipeline speedup\n");
    
    printf("\n  Scenario: What if only part of execution can be pipelined?\n");
    printf("\n  +------------------+----------+----------+----------+----------+\n");
    printf("  | Pipelined        | k=4      | k=8      | k=16     | k=32     |\n");
    printf("  | Fraction (f)     | Speedup  | Speedup  | Speedup  | Speedup  |\n");
    printf("  +------------------+----------+----------+----------+----------+\n");
    
    float f_values[] = {0.5, 0.75, 0.9, 0.95, 0.99, 1.0};
    int k_values[] = {4, 8, 16, 32};
    
    for (int i = 0; i < 6; i++) {
        float f = f_values[i];
        printf("  | %-16.0f%% |", f * 100);
        for (int j = 0; j < 4; j++) {
            int k = k_values[j];
            float speedup = 1.0 / ((1.0 - f) + f / k);
            printf(" %-8.4f |", speedup);
        }
        printf("\n");
    }
    printf("  +------------------+----------+----------+----------+----------+\n");
    
    printf("\n    Insight:\n");
    printf("    - Even with k=32 pipeline, if only 90%% can be pipelined,\n");
    printf("      maximum speedup is limited to ~7.8x (not 32x)\n");
    printf("    - The serial portion dominates as pipeline depth increases\n");
    printf("    - This is why real processors focus on reducing serial portions\n");
}

// ============================================
// Main Function
// ============================================

int main() {
    int choice;
    PipelineConfig config = {5, 20, 0, 0};  // Default configuration
    
    print_header("PIPELINE SPEEDUP ANALYSIS LABORATORY");
    
    printf("\n  This lab demonstrates pipeline performance equations,\n");
    printf("  speedup calculations, and visualization techniques.\n");
    
    do {
        printf("\n  +------------------------------------------------------------+\n");
        printf("  |                      MAIN MENU                             |\n");
        printf("  +------------------------------------------------------------+\n");
        printf("  |  1. Speedup Equation Derivation                            |\n");
        printf("  |  2. Compare Multiple Speedup Formulas                      |\n");
        printf("  |  3. Analyze Speedup vs. Number of Instructions             |\n");
        printf("  |  4. Analyze Speedup vs. Pipeline Depth                     |\n");
        printf("  |  5. Analyze Impact of Stalls                               |\n");
        printf("  |  6. Pipeline Gantt Chart Visualization                     |\n");
        printf("  |  7. Space Time Diagram                                     |\n");
        printf("  |  8. Throughput and CPI Analysis                            |\n");
        printf("  |  9. Interactive Speedup Calculator                         |\n");
        printf("  | 10. Compare Multiple Configurations                        |\n");
        printf("  | 11. Amdahl's Law Demonstration                             |\n");
        printf("  | 12. Generate Complete Performance Report                   |\n");
        printf("  |  0. Exit                                                   |\n");
        printf("  +------------------------------------------------------------+\n");
        printf("  Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                demonstrate_speedup_derivation(config.stages, config.instructions);
                break;
                
            case 2:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                printf("  Enter stall cycles: ");
                scanf("%d", &config.stall_cycles);
                printf("  Enter clock overhead (%%): ");
                scanf("%f", &config.clock_overhead);
                demonstrate_speedup_formulas(config.stages, config.instructions, 
                                             config.stall_cycles, config.clock_overhead);
                break;
                
            case 3:
                printf("\n  Enter pipeline depth (k): ");
                scanf("%d", &config.stages);
                analyze_speedup_vs_instructions(config.stages);
                break;
                
            case 4:
                printf("\n  Enter number of instructions (n): ");
                scanf("%d", &config.instructions);
                analyze_speedup_vs_stages(config.instructions);
                break;
                
            case 5:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                analyze_speedup_vs_stalls(config.stages, config.instructions);
                break;
                
            case 6:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                visualize_pipeline_gantt(config.stages, config.instructions);
                break;
                
            case 7:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                visualize_space_time_diagram(config.stages, config.instructions);
                break;
                
            case 8:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                printf("  Enter stall cycles: ");
                scanf("%d", &config.stall_cycles);
                analyze_throughput(config.stages, config.instructions, config.stall_cycles);
                break;
                
            case 9:
                interactive_speedup_calculator();
                break;
                
            case 10:
                compare_configurations();
                break;
                
            case 11:
                demonstrate_amdahls_law();
                break;
                
            case 12:
                printf("\n  Enter stages (k): ");
                scanf("%d", &config.stages);
                printf("  Enter instructions (n): ");
                scanf("%d", &config.instructions);
                printf("  Enter stall cycles: ");
                scanf("%d", &config.stall_cycles);
                printf("  Enter clock overhead (%%): ");
                scanf("%f", &config.clock_overhead);
                generate_performance_report(config);
                break;
                
            case 0:
                printf("\n  Exiting Pipeline Speedup Lab.!\n\n");
                break;
                
            default:
                printf("\n  Invalid choice. Please try again.\n");
        }
        
    } while (choice != 0);
    
    return 0;
}
