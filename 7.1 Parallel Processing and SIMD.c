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

void sisd_demo(float a, float b) {
    printf("[SISD] %.2f + %.2f = %.2f\n", a, b, a + b);
}

void simd_demo(float *A, float *B) {
    float R[VEC];
#if USE_SSE
    _mm_storeu_ps(R, _mm_add_ps(_mm_loadu_ps(A), _mm_loadu_ps(B)));
#elif USE_NEON
    vst1q_f32(R, vaddq_f32(vld1q_f32(A), vld1q_f32(B)));
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] + B[i];
#endif
    printf("[SIMD] Vector Add: ");
    for (int i = 0; i < VEC; i++) printf("%.2f+%.2f=%.2f ", A[i], B[i], R[i]);
    printf("\n");
}

void misd_demo(float d) {
    printf("[MISD] Data %.2f: ADD10=%.2f MUL2=%.2f SUB5=%.2f\n", d, d+10, d*2, d-5);
}

typedef struct { int id; float data, result; pthread_mutex_t *mtx; } mimd_t;

void *mimd_worker(void *arg) {
    mimd_t *t = (mimd_t *)arg;
    float ops[] = {t->data * t->data, t->data + 100, t->data / 3, t->data * 2.5f};
    t->result = ops[t->id % 4];
    pthread_mutex_lock(t->mtx);
    printf("  Thread %d: %.2f -> %.2f\n", t->id, t->data, t->result);
    pthread_mutex_unlock(t->mtx);
    return NULL;
}

void mimd_demo(float *data) {
    pthread_t th[N_TASKS]; mimd_t tasks[N_TASKS];
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
    printf("[MIMD] %d threads:\n", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (mimd_t){i, data[i], 0, &mtx};
        pthread_create(&th[i], NULL, mimd_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    pthread_mutex_destroy(&mtx);
}

void print_vec(const char *name, float *v) {
    printf("  %s: [", name);
    for (int i = 0; i < VEC; i++) printf(" %.2f", v[i]);
    printf(" ]\n");
}

void simd_vector_add(float *A, float *B) {
    float R[VEC];
#if USE_SSE
    _mm_storeu_ps(R, _mm_add_ps(_mm_loadu_ps(A), _mm_loadu_ps(B)));
#elif USE_NEON
    vst1q_f32(R, vaddq_f32(vld1q_f32(A), vld1q_f32(B)));
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] + B[i];
#endif
    printf("Vector Addition:\n");
    print_vec("A", A); print_vec("B", B); print_vec("R", R);
}

void simd_vector_mul(float *A, float *B) {
    float R[VEC];
#if USE_SSE
    _mm_storeu_ps(R, _mm_mul_ps(_mm_loadu_ps(A), _mm_loadu_ps(B)));
#elif USE_NEON
    vst1q_f32(R, vmulq_f32(vld1q_f32(A), vld1q_f32(B)));
#else
    for (int i = 0; i < VEC; i++) R[i] = A[i] * B[i];
#endif
    printf("Vector Multiplication:\n");
    print_vec("A", A); print_vec("B", B); print_vec("R", R);
}

void print_mat(const char *name, float m[MAT][MAT]) {
    printf("  %s: [[%.2f %.2f] [%.2f %.2f]]\n", name, m[0][0], m[0][1], m[1][0], m[1][1]);
}

void simd_matmul(float A[MAT][MAT], float B[MAT][MAT]) {
    float R[MAT][MAT] = {{0}};
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
    printf("Matrix Multiplication:\n");
    print_mat("A", A); print_mat("B", B); print_mat("R", R);
}

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

    printf("Sequential:\n");
    clock_gettime(CLOCK_MONOTONIC, &s);
    for (int i = 0; i < N_TASKS; i++) {
        float v = inputs[i];
        for (int j = 0; j < 1000000; j++) v = v * 1.000001f + 0.000001f;
        printf("  Task %d: %.2f -> %.6f\n", i, inputs[i], v);
    }
    clock_gettime(CLOCK_MONOTONIC, &e);
    long seq = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    printf("Parallel (%d threads):\n", N_TASKS);
    clock_gettime(CLOCK_MONOTONIC, &s);
    for (int i = 0; i < N_TASKS; i++) {
        tasks[i] = (par_t){i, inputs[i], 0, &mtx};
        pthread_create(&th[i], NULL, par_worker, &tasks[i]);
    }
    for (int i = 0; i < N_TASKS; i++) pthread_join(th[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &e);
    long par = (e.tv_sec-s.tv_sec)*1000000L + (e.tv_nsec-s.tv_nsec)/1000;

    printf("Seq: %ldus, Par: %ldus, Speedup: %.2fx, Eff: %.1f%%\n", 
           seq, par, par > 0 ? (float)seq/par : 0, par > 0 ? ((float)seq/par)/N_TASKS*100 : 0);
    pthread_mutex_destroy(&mtx);
}

int main() {
    float a, b, vecA[VEC], vecB[VEC], mimd_data[N_TASKS], par_in[N_TASKS];
    float matA[MAT][MAT], matB[MAT][MAT];

    printf("=== FLYNN'S CLASSIFICATION ===\n");
    printf("SISD operands: "); scanf("%f %f", &a, &b);
    sisd_demo(a, b);

    printf("SIMD Vector A (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("SIMD Vector B (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_demo(vecA, vecB);

    printf("MISD value: "); scanf("%f", &a);
    misd_demo(a);

    printf("MIMD data (%d): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &mimd_data[i]);
    mimd_demo(mimd_data);

    printf("\n=== SIMD VECTOR OPS ===\n");
    printf("Vector A (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecA[i]);
    printf("Vector B (%d): ", VEC);
    for (int i = 0; i < VEC; i++) scanf("%f", &vecB[i]);
    simd_vector_add(vecA, vecB);
    simd_vector_mul(vecA, vecB);

    printf("\n=== SIMD MATRIX MUL ===\n");
    printf("Matrix A (%dx%d): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matA[i][j]);
    printf("Matrix B (%dx%d): ", MAT, MAT);
    for (int i = 0; i < MAT; i++) for (int j = 0; j < MAT; j++) scanf("%f", &matB[i][j]);
    simd_matmul(matA, matB);

    printf("\n=== PARALLEL PROCESSING ===\n");
    printf("Inputs (%d): ", N_TASKS);
    for (int i = 0; i < N_TASKS; i++) scanf("%f", &par_in[i]);
    parallel_demo(par_in);

    return 0;
}
