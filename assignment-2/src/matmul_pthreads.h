#pragma once
#include <pthread.h>

struct PThreadArgs
{
    const double *A;
    const double *B;
    double *C;
    int c1, c2; // column sizes
    int rs, re; // row start, row end
    int cs, ce; // col start, col end
    int b;
};

void *compute_pthread(void *arg)
{
    PThreadArgs *a = static_cast<PThreadArgs *>(arg);
    tiled_kernel(a->A, a->B, a->C, a->c1, a->c2, a->rs, a->re, a->cs, a->ce, a->b);
    return nullptr;
}

void mat_mul_pthread_block(const double *A, int r1, int c1,
                           const double *B, int r2, int c2,
                           double *C,
                           int pr, int pc, int b)
{
    int blocks = pr * pc;
    pthread_t *threads = new pthread_t[blocks];
    PThreadArgs *args = new PThreadArgs[blocks];

    for (int rank = 0; rank < blocks; rank++)
    {
        // these variable names kind of suck
        // but its the c++ style im sure
        // it seems sucking, in general, is the c++ style
        int bi = rank / pc, bj = rank % pc;
        int rs, re, cs, ce;
        block_range(r1, pr, bi, rs, re);
        block_range(c2, pc, bj, cs, ce);

        args[rank] = {A, B, C, c1, c2, rs, re, cs, ce, b};
        pthread_create(&threads[rank], nullptr, compute_pthread, &args[rank]);
    }

    for (int i = 0; i < blocks; i++)
        pthread_join(threads[i], nullptr);

    delete[] threads;
    delete[] args;
}

void mat_mul_pthread_row(const double *A, int r1, int c1,
                         const double *B, int r2, int c2,
                         double *C, int p, int b)
{
    mat_mul_pthread_block(A, r1, c1, B, r2, c2, C, p, 1, b);
}