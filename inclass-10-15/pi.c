/* pi_start.c: estimate pi by random points, sequential and with pthreads.
 * ITCS 4145 in-class exercise, starter file. Complete the two TODO parts.
 *
 * Usage: ./pi <n> <p>      n random points, p threads (p = 0: sequential)
 *
 * A point (x, y) with 0 <= x, y < 1 lies inside the quarter circle of
 * radius 1 when x*x + y*y < 1. The quarter circle has area pi/4 and the
 * square has area 1, so pi is about 4 * hits / n.
 *
 * Build: gcc -O2 -Wall -pthread pi.c -o pi
 */
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---- Timer ---- */
static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

/* ---- Random numbers: xorshift64*, one state per generator ---- */
typedef struct { uint64_t state; } rng_t;

static void rng_seed(rng_t *r, uint64_t seed) {
    r->state = seed * 0x9E3779B97F4A7C15ULL + 1;
}
static double rng_unit(rng_t *r) {              /* uniform in [0, 1) */
    r->state ^= r->state >> 12;
    r->state ^= r->state << 25;
    r->state ^= r->state >> 27;
    return (double)((r->state * 2685821657736338717ULL) >> 11)
           * (1.0 / 9007199254740992.0);
}

/* ---- Count the points of one generator that fall inside ---- */
static long count_hits(rng_t *r, long points) {
    long hits = 0;
    for (long i = 0; i < points; i++) {
        double x = rng_unit(r), y = rng_unit(r);
        if (x * x + y * y < 1.0) hits++;
    }
    return hits;
}

/* ---- Sequential ---- */
static long pi_seq(long n, uint64_t seed) {
    rng_t r;
    rng_seed(&r, seed);
    return count_hits(&r, n);
}

/* ---- Parallel: one argument structure per thread ---- */
typedef struct {
    int      rank;    /* which thread am I */
    int      size;    /* how many threads */
    long     n;       /* total number of points */
    uint64_t seed;    /* base seed; each thread adds its rank */
    long    *hits;    /* shared array of p slots; thread rank writes slot rank */
} args_t;

static void *work(void *arg) {
    args_t *a = (args_t *)arg;
    /* TODO (step 3):
     *   1. compute my block of points: n / size, the last rank takes the
     *      remainder;
     *   2. seed my own generator with seed + rank;
     *   3. count my hits with count_hits and write the count to my own
     *      slot hits[rank]. */
    
    int rank = a->rank;
    int size = a->size;
    long n = a->n;
    uint64_t seed = a->seed;
    long *hits = a->hits;

    long points = n / size;
    // add remainder to last thread
    if (rank >= (size - 1)) {
       points += (points * size) - n;
    }

    // seed generator
    rng_t r;
    rng_seed(&r, seed + rank);

    long hitCount = count_hits(&r, points);
    *hits += hitCount;

    (void) a;
    return NULL;
}

static long pi_par(long n, int p, uint64_t seed, long *hits) {

    // create threads
    pthread_t threadIds[p];
    args_t threadArgs[p];
    long threadHits[p];
    for (int i = 0; i < p; i++) {
        threadArgs[i].rank = i;
        threadArgs[i].size = p;
        threadArgs[i].n = n;
        threadArgs[i].seed = seed;
        threadArgs[i].hits = &threadHits[i];

        pthread_create(&threadIds[i], NULL, work, &threadArgs[i]);
    }

    // join
    for (int i = 0; i < p; i++) {
        pthread_join(threadIds[i], NULL);
        *hits += threadHits[i];
    }
    

    (void)n; (void)p; (void)seed; (void)hits; (void)work;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s n p   (p = 0: sequential)\n", argv[0]);
        return 1;
    }
    long n = atol(argv[1]);
    int  p = atoi(argv[2]);
    uint64_t seed = 4145;
    long hits[p > 0 ? p : 1];

    double t0 = now_seconds();
    long total = (p == 0) ? pi_seq(n, seed) : pi_par(n, p, seed, hits);
    double t1 = now_seconds();

    double estimate = 4.0 * (double)total / (double)n;
    printf("n = %ld, p = %d, hits = %ld, pi = %.6f, error = %.2e, time = %.4f s\n",
           n, p, total, estimate, estimate - 3.14159265358979, t1 - t0);
    return 0;
}
