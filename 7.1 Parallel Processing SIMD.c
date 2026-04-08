#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>
    #define USE_SSE 1
    #define USE_NEON 0
#elif defined(__aarch64__) || defined(_M_ARM64)
    #include <arm_neon.h>
    #define USE_NEON 1
    #define USE_SSE 0
#else
    #define USE_SSE 0
    #define USE_NEON 0
#endif

#define VEC 4
#define MAT 2
#define N_TASKS 4

// ==================== FLYNN'S CLASSIFICATION ====================

void sisd_demo(float a, float b) {
    printf("\n[SISD] Scalar ADD: %.2f + %.2f = %.2f\n", a, b, a + b);
    printf("  Description: Single instruction operates on single data element.\n");
    printf("  This is the baseline sequential processing model.\n");
}

void simd_demo(float *A, float *B) {
    float R[VEC];
    printf("\n[SIMD] Vector Addition:\n");

#if USE_SSE
    __m128 va = _mm_loadu_ps(A), vb = _mm_loadu_ps(B);
    _mm_storeu_ps(R, _mm_add_ps(va, vb));
    printf("  Hardware: x86 SSE _mm_add_ps (1 instruction, 4 additions)\n");
#elif USE_NEON
    vst1q_f32(R, vaddq_f32(vld1q_f32(A), vld1q_f32(B)));
    printf("  Hardware: ARM NEON vaddq_f32 (1 instruction, 4 additions)\n");
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] + B[i];
    printf("  WARNING: No SIMD ISA detected, using scalar fallback.\n");
#endif

    printf("  Description: Single instruction processes multiple data elements simultaneously.\n");
    for (int i = 0; i < VEC; i++)
        printf("  R[%d] = %.2f + %.2f = %.2f\n", i, A[i], B[i], R[i]);
}

void misd_demo(float d) {
    printf("\n[MISD] 3 instructions on same data (%.2f):\n", d);
    printf("  ADD 10 = %.2f | MUL 2 = %.2f | SUB 5 = %.2f\n", d+10, d*2, d-5);
    printf("  Description: Multiple instructions operate on the same data stream.\n");
    printf("  This is a rare architecture, often used in fault-tolerant systems.\n");
}

typedef struct { int id; float data, result; pthread_mutex_t *mtx; } mimd_t;

void *mimd_worker(void *arg) {
    mimd_t *t = (mimd_t *)arg;
    float ops[] = {t->data * t->data, t->data + 100, t->data / 3, t->data * 2.5f};
    const char *names[] = {"SQUARE", "ADD_100", "DIV_3", "MUL_2.5"};
    t->result = ops[t->id % 4];
    pthread_mutex_lock(t->mtx);
    printf("  Thread %d: %s(%.2f) = %.2f\n", t->id, names[t->id % 4], t->data, t->result);
    pthread_mutex_unlock(t->mtx);
    return NULL;
}

void mimd_demo(float *data) {
    pthread_t th[N_TASKS]; mimd_t tasks[N_TASKS];
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    printf("\n[MIMD] %d threads, different instructions, different data:\n", N_TASKS);
    printf("  Description: Multiple threads execute different instructions on different data.\n");
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (mimd_t){i, data[i], 0, &mtx};
        pthread_create(&th[i], NULL, mimd_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    pthread_mutex_destroy(&mtx);
}

// ==================== SIMD VECTOR OPERATIONS ====================

void simd_vector_add(float *A, float *B) {
    float R[VEC];
    printf("\n--- SIMD Vector Addition ---\n");

#if USE_SSE
    __m128 va = _mm_loadu_ps(A), vb = _mm_loadu_ps(B);
    _mm_storeu_ps(R, _mm_add_ps(va, vb));
    printf("  Hardware: x86 SSE _mm_add_ps\n");
    printf("  Operation: 4 parallel additions in 1 instruction\n");
#elif USE_NEON
    vst1q_f32(R, vaddq_f32(vld1q_f32(A), vld1q_f32(B)));
    printf("  Hardware: ARM NEON vaddq_f32\n");
    printf("  Operation: 4 parallel additions in 1 instruction\n");
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] + B[i];
    printf("  WARNING: No SIMD ISA detected, using scalar fallback.\n");
#endif

    printf("\n  Input A:  [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", A[i]);
    printf(" ]\n  Input B:  [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", B[i]);
    printf(" ]\n  Result R: [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", R[i]);
    printf(" ]\n");
}

void simd_vector_mul(float *A, float *B) {
    float R[VEC];
    printf("\n--- SIMD Vector Multiplication ---\n");

#if USE_SSE
    __m128 va = _mm_loadu_ps(A), vb = _mm_loadu_ps(B);
    _mm_storeu_ps(R, _mm_mul_ps(va, vb));
    printf("  Hardware: x86 SSE _mm_mul_ps\n");
    printf("  Operation: 4 parallel multiplications in 1 instruction\n");
#elif USE_NEON
    vst1q_f32(R, vmulq_f32(vld1q_f32(A), vld1q_f32(B)));
    printf("  Hardware: ARM NEON vmulq_f32\n");
    printf("  Operation: 4 parallel multiplications in 1 instruction\n");
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] * B[i];
    printf("  WARNING: No SIMD ISA detected, using scalar fallback.\n");
#endif

    printf("\n  Input A:  [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", A[i]);
    printf(" ]\n  Input B:  [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", B[i]);
    printf(" ]\n  Result R: [");
    for (int i = 0; i < VEC; i++) printf(" %.2f", R[i]);
    printf(" ]\n");
}

// ==================== SIMD MATRIX MULTIPLY ====================

void simd_matmul(float A[MAT][MAT], float B[MAT][MAT]) {
    float R[MAT][MAT] = {{0}};
    printf("\n--- SIMD Matrix Multiplication (%dx%d) ---\n", MAT, MAT);

    for (int i = 0; i < MAT; i++)
        for (int k = 0; k < MAT; k++) {
#if USE_SSE
            float a_bc[4] = {A[i][k], A[i][k], 0, 0};
            float b_rw[4] = {B[k][0], B[k][1], 0, 0};
            float p[4];
            _mm_storeu_ps(p, _mm_mul_ps(_mm_loadu_ps(a_bc), _mm_loadu_ps(b_rw)));
            R[i][0] += p[0]; R[i][1] += p[1];
#elif USE_NEON
            float a_bc[4] = {A[i][k], A[i][k], 0, 0};
            float b_rw[4] = {B[k][0], B[k][1], 0, 0};
            float p[4];
            vst1q_f32(p, vmulq_f32(vld1q_f32(a_bc), vld1q_f32(b_rw)));
            R[i][0] += p[0]; R[i][1] += p[1];
#else
            R[i][0] += A[i][k] * B[k][0];
            R[i][1] += A[i][k] * B[k][1];
#endif
        }

#if USE_SSE
    printf("  Hardware: x86 SSE _mm_mul_ps\n");
#elif USE_NEON
    printf("  Hardware: ARM NEON vmulq_f32\n");
#else
    printf("  WARNING: No SIMD, scalar fallback.\n");
#endif

    printf("  Operation: Parallel element-wise multiplication for matrix computation\n");
    printf("\n  Matrix A:\n");
    for (int i = 0; i < MAT; i++)
        printf("    [ %.2f  %.2f ]\n", A[i][0], A[i][1]);
    printf("\n  Matrix B:\n");
    for (int i = 0; i < MAT; i++)
        printf("    [ %.2f  %.2f ]\n", B[i][0], B[i][1]);
    printf("\n  Result (A x B):\n");
    for (int i = 0; i < MAT; i++)
        printf("    [ %.2f  %.2f ]\n", R[i][0], R[i][1]);
}

// ==================== PARALLEL PROCESSING (TIMED) ====================

typedef struct { int id; float in, out; pthread_mutex_t *mtx; } par_t;

void *par_worker(void *arg) {
    par_t *t = (par_t *)arg;
    float v = t->in;
    for (int i = 0; i < 1000000; i++) v = v * 1.000001f + 0.000001f;
    t->out = v;
    pthread_mutex_lock(t->mtx);
    printf("  Task %d: %.2f -> %.6f\n", t->id, t->in, t->out);
    pthread_mutex_unlock(t->mtx);
    return NULL;
}

void parallel_demo(float *inputs) {
    pthread_t th[N_TASKS]; par_t tasks[N_TASKS];
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    struct timespec s, e;
    float seq_results[N_TASKS];

    printf("\n--- Parallel Processing Performance Comparison ---\n");
    printf("  Workload: 1,000,000 floating-point operations per task\n");
    printf("  Tasks: %d independent computational tasks\n\n", N_TASKS);

    // Sequential execution
    printf("Sequential execution:\n");
    clock_gettime(CLOCK_MONOTONIC, &s);
    for (int i = 0; i < N_TASKS; i++) {
        float v = inputs[i];
        for (int j = 0; j < 1000000; j++) v = v * 1.000001f + 0.000001f;
        seq_results[i] = v;
        printf("  Task %d: %.2f -> %.6f\n", i, inputs[i], seq_results[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &e);
    long seq = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    // Parallel execution
    clock_gettime(CLOCK_MONOTONIC, &s);
    printf("\nParallel execution (%d threads):\n", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (par_t){i, inputs[i], 0, &mtx};
        pthread_create(&th[i], NULL, par_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &e);
    long par = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    printf("\n--- Performance Results ---\n");
    printf("  Sequential time: %ld microseconds\n", seq);
    printf("  Parallel time:   %ld microseconds\n", par);
    printf("  Speedup:         %.2fx\n", par > 0 ? (float)seq/par : 0);
    printf("  Efficiency:      %.1f%%\n", par > 0 ? ((float)seq/par)/N_TASKS*100 : 0);
    pthread_mutex_destroy(&mtx);
}

// ==================== MAIN ====================

int main() {
    float a, b, vecA[VEC], vecB[VEC], mimd_data[N_TASKS], par_in[N_TASKS];
    float matA[MAT][MAT], matB[MAT][MAT];

    printf("=== PARALLEL PROCESSING & SIMD OPERATIONS ===\n");
    printf("This lab demonstrates parallel computing concepts:\n");
    printf("  - Flynn's Classification (SISD, SIMD, MISD, MIMD)\n");
    printf("  - SIMD Vector Operations\n");
    printf("  - SIMD Matrix Multiplication\n");
    printf("  - Multithreaded Parallel Processing\n");

    // Flynn's Classification
    printf("\n**************************************************\n");
    printf("*** SECTION 1: FLYNN'S CLASSIFICATION ***\n");
    printf("**************************************************\n");

    printf("\nSISD operands (A B): "); scanf("%f %f", &a, &b);
    sisd_demo(a, b);

    printf("\nSIMD Vector A (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("SIMD Vector B (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_demo(vecA, vecB);

    printf("\nMISD data value: "); scanf("%f", &a);
    misd_demo(a);

    printf("\nMIMD data (%d values): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &mimd_data[i]);
    mimd_demo(mimd_data);

    // SIMD Vector Operations
    printf("\n**************************************************\n");
    printf("*** SECTION 2: SIMD VECTOR OPERATIONS ***\n");
    printf("**************************************************\n");

    printf("\nVector A (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("Vector B (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_vector_add(vecA, vecB);
    simd_vector_mul(vecA, vecB);

    // SIMD Matrix Multiplication
    printf("\n**************************************************\n");
    printf("*** SECTION 3: SIMD MATRIX MULTIPLICATION ***\n");
    printf("**************************************************\n");

    printf("\nMatrix A (%dx%d, row-major): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matA[i][j]);
    printf("Matrix B (%dx%d, row-major): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matB[i][j]);
    simd_matmul(matA, matB);

    // Parallel Processing
    printf("\n**************************************************\n");
    printf("*** SECTION 4: PARALLEL PROCESSING WITH PTHREADS ***\n");
    printf("**************************************************\n");

    printf("\nInputs (%d values): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &par_in[i]);
    parallel_demo(par_in);

    printf("\n=== LAB COMPLETE ===\n");
    printf("Summary:\n");
    printf("  - Demonstrated all four Flynn's classification categories\n");
    printf("  - Showed SIMD vector addition and multiplication\n");
    printf("  - Performed SIMD-accelerated matrix multiplication\n");
    printf("  - Compared sequential vs parallel execution performance\n");
    return 0;
}
