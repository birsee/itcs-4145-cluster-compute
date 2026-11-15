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
};

// i reuse this for both pthreads implementations since it's fairly robust...
void *compute_pthread(void *arg)
{
    PThreadArgs *args = static_cast<PThreadArgs *>(arg);

    const double *A = args->A;
    const double *B = args->B;
    double *C = args->C;

    int c1 = args->c1;
    int c2 = args->c2;

    int rs = args->rs;
    int re = args->re;

    int cs = args->cs;
    int ce = args->ce;

    for (int i = rs; i < re; i++)
    {
        for (int j = 0; j < c1; j++)
        {
            double aij = A[i * c1 + j];
            for (int k = cs; k < ce; k++)
            {
                C[i * c2 + k] += aij * B[j * c2 + k];
            }
        }
    }

    return nullptr;
}

double *mat_mul_pthread_row(const double *A, int r1, int c1,
                            const double *B, int r2, int c2,
                            int block_rows)
{
    double *C = new double[r1 * c2]();
    int rows_per_block = (r1 + block_rows - 1) / block_rows;

    pthread_t *threads = new pthread_t[block_rows];
    PThreadArgs *args = new PThreadArgs[block_rows];

    for (int i = 0; i < block_rows; i++)
    {
        int rs = (i + 0) * rows_per_block;
        int re = (i + 1) * rows_per_block;
        if (re > r1) re = r1;

        args[i] = {A, B, C, c1, c2, rs, re, 0, c2};
        pthread_create(&threads[i], nullptr, compute_pthread, &args[i]);
    }

    for (int i = 0; i < block_rows; i++)
        pthread_join(threads[i], nullptr);

    delete[] threads;
    delete[] args;

    return C;
}

double *mat_mul_pthread_block(const double *A, int r1, int c1,
                              const double *B, int r2, int c2,
                              int block_rows, int block_cols)
{
    double *C = new double[r1 * c2]();
    int blocks = block_rows * block_cols;

    int rows_per_block = (r1 + block_rows - 1) / block_rows;
    int cols_per_block = (c2 + block_cols - 1) / block_cols;
    
    pthread_t *threads = new pthread_t[blocks];
    PThreadArgs *args = new PThreadArgs[blocks];

    int i = 0;
    for (int bi = 0; bi < block_rows; bi++) 
    {
        // these variable names kind of suck
        // but its the c++ style im sure
        // it seems sucking, in general, is the c++ style
        int rs = (bi + 0) * rows_per_block;
        int re = (bi + 1) * rows_per_block;
        if (re > r1) re = r1;

        for (int bj = 0; bj < block_cols; bj++)
        {
            int cs = (bj + 0) * cols_per_block;
            int ce = (bj + 1) * cols_per_block;
            if (ce > c2) ce = c2;

            args[i] = {A, B, C, c1, c2, rs, re, cs, ce};
            pthread_create(&threads[i], nullptr, compute_pthread, &args[i]);
            i++;
        }
    }

    for (int j = 0; j < blocks; j++)
        pthread_join(threads[j], nullptr);

    delete[] threads;
    delete[] args;

    return C;
}