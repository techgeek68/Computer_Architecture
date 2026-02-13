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
#define N_INST 4
#define N_TASKS 4

// ==================== FLYNN'S CLASSIFICATION ====================

void sisd_demo(float a, float b) {
    printf("\n[SISD] Scalar ADD: %.2f + %.2f = %.2f\n", a, b, a + b);
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

    for (int i = 0; i < VEC; i++)
        printf("  R[%d] = %.2f + %.2f = %.2f\n", i, A[i], B[i], R[i]);
}

void misd_demo(float d) {
    printf("\n[MISD] 3 instructions on same data (%.2f):\n", d);
    printf("  ADD 10 = %.2f | MUL 2 = %.2f | SUB 5 = %.2f\n", d+10, d*2, d-5);
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
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (mimd_t){i, data[i], 0, &mtx};
        pthread_create(&th[i], NULL, mimd_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    pthread_mutex_destroy(&mtx);
}

// ==================== PIPELINING ====================

void pipeline_demo(float ops[][2], char *signs, int n) {
    const char *stg[] = {"IF", "ID", "EX", "WB"};
    int total = n + 3;
    float results[N_INST];

    printf("\n--- Pipeline (%d instructions, 4 stages, %d cycles) ---\n", n, total);
    for (int c = 0; c < total; c++) {
        printf("C%d: ", c + 1);
        for (int i = 0; i < n; i++) {
            int s = c - i;
            if (s >= 0 && s < 4) {
                if (s == 2) results[i] = (signs[i] == '+') ? ops[i][0] + ops[i][1] : ops[i][0] - ops[i][1];
                if (s == 2) printf("[I%d:%s=%.1f] ", i+1, stg[s], results[i]);
                else        printf("[I%d:%s] ", i+1, stg[s]);
            }
        }
        printf("\n");
    }
    printf("Pipelined: %d cycles | Non-pipelined: %d cycles\n", total, n * 4);

    for (int i = 0; i < n; i++)
        printf("  I%d: %.2f %c %.2f = %.2f\n", i+1, ops[i][0], signs[i], ops[i][1], results[i]);
}

// ==================== SPEEDUP EQUATION ====================

void speedup_demo(int k, int n) {
    printf("\nSpeedup: S = (k×n)/(k+n-1) = (%d×%d)/(%d) = %.4f\n", k, n, k+n-1, (float)(k*n)/(k+n-1));
    printf("Efficiency: %.2f%% | Max (n→∞): S→%d\n", ((float)(k*n)/(k+n-1))/k*100, k);

    int tv[] = {1, 5, 10, 50, 100, 1000};
    printf("\n  %-6s %-8s %-8s\n", "n", "Speedup", "Eff%");
    for (int i = 0; i < 6; i++) {
        float s = (float)(k * tv[i]) / (k + tv[i] - 1);
        printf("  %-6d %-8.3f %-7.1f%%\n", tv[i], s, s/k*100);
    }
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

    for (int i = 0; i < MAT; i++)
        printf("  [ %.2f  %.2f ]\n", R[i][0], R[i][1]);
}

// ==================== PARALLEL PROCESSING (TIMED) ====================

typedef struct { int id; float in, out; pthread_mutex_t *mtx; } par_t;

void *par_worker(void *arg) {
    par_t *t = (par_t *)arg;
    float v = t->in;
    for (int i = 0; i < 1000000; i++) v = v * 1.000001f + 0.000001f;
    t->out = v;
    pthread_mutex_lock(t->mtx);
    printf("  Task %d: %.2f → %.6f\n", t->id, t->in, t->out);
    pthread_mutex_unlock(t->mtx);
    return NULL;
}

void parallel_demo(float *inputs) {
    pthread_t th[N_TASKS]; par_t tasks[N_TASKS];
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    struct timespec s, e;

    // Sequential
    clock_gettime(CLOCK_MONOTONIC, &s);
    for (int i = 0; i < N_TASKS; i++) {
        float v = inputs[i];
        for (int j = 0; j < 1000000; j++) v = v * 1.000001f + 0.000001f;
    }
    clock_gettime(CLOCK_MONOTONIC, &e);
    long seq = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    // Parallel
    clock_gettime(CLOCK_MONOTONIC, &s);
    printf("\nParallel execution (%d threads):\n", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (par_t){i, inputs[i], 0, &mtx};
        pthread_create(&th[i], NULL, par_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &e);
    long par = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    printf("Sequential: %ldµs | Parallel: %ldµs | Speedup: %.2fx\n", seq, par, par>0?(float)seq/par:0);
    pthread_mutex_destroy(&mtx);
}

// ==================== MAIN ====================

int main() {
    float a, b, vecA[VEC], vecB[VEC], mimd_data[N_TASKS], par_in[N_TASKS];
    float pip_ops[N_INST][2]; char pip_signs[N_INST];
    float matA[MAT][MAT], matB[MAT][MAT];
    int k, n;

    printf("=== PIPELINING, PARALLEL PROCESSING & SIMD — HARDWARE LAB ===\n");

    // Flynn's Classification
    printf("\n*** FLYNN'S CLASSIFICATION ***\n");
    printf("SISD operands (A B): "); scanf("%f %f", &a, &b);
    sisd_demo(a, b);

    printf("SIMD Vector A (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("SIMD Vector B (%d floats): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_demo(vecA, vecB);

    printf("MISD data value: "); scanf("%f", &a);
    misd_demo(a);

    printf("MIMD data (%d values): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &mimd_data[i]);
    mimd_demo(mimd_data);

    // Pipelining
    printf("\n*** PIPELINING ***\n");
    for (int i = 0; i < N_INST; i++) {
        printf("I%d (A B op): ", i+1);
        scanf("%f %f %c", &pip_ops[i][0], &pip_ops[i][1], &pip_signs[i]);
    }
    pipeline_demo(pip_ops, pip_signs, N_INST);

    // Speedup
    printf("\n*** SPEEDUP EQUATION ***\n");
    printf("Stages(k) Instructions(n): "); scanf("%d %d", &k, &n);
    speedup_demo(k, n);

    // SIMD Vector Addition
    printf("\n*** SIMD VECTOR ADDITION ***\n");
    printf("Vector A (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("Vector B (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_demo(vecA, vecB);

    // SIMD Matrix Multiplication
    printf("\n*** SIMD MATRIX MULTIPLICATION ***\n");
    printf("Matrix A (%dx%d, row-major): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matA[i][j]);
    printf("Matrix B (%dx%d, row-major): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matB[i][j]);
    simd_matmul(matA, matB);

    // Parallel Processing
    printf("\n*** PARALLEL PROCESSING ***\n");
    printf("Inputs (%d values): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &par_in[i]);
    parallel_demo(par_in);

    printf("\n=== LAB COMPLETE ===\n");
    return 0;
}
