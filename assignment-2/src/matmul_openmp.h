#pragma once
#include <algorithm>
#include <omp.h>

// pretty straightforward port of matmul_pthreads.h to omp!

double *mat_mul_omp_row(const double *A, int r1, int c1,
                        const double *B, int r2, int c2,
                        int block_rows)
{
    double *C = new double[r1 * c2]();

    #pragma omp parallel for num_threads(block_rows) schedule(static)
    for (int i = 0; i < r1; i++)
    {
        for (int j = 0; j < c1; j++)
        {
            double aij = A[i * c1 + j];
            for (int k = 0; k < c2; k++)
            {
                C[i * c2 + k] += aij * B[j * c2 + k];
            }
        }
    }

    return C;
}

double *mat_mul_omp_block(const double *A, int r1, int c1,
                          const double *B, int r2, int c2,
                          int block_rows, int block_cols)
{
    double *C = new double[r1 * c2]();

    int rows_per_block = (r1 + block_rows - 1) / block_rows;
    int cols_per_block = (c2 + block_cols - 1) / block_cols;

    #pragma omp parallel for collapse(2) num_threads(block_rows * block_cols) schedule(static)
    for (int bi = 0; bi < block_rows; bi++)
    {
        for (int bj = 0; bj < block_cols; bj++)
        {
            int rs = (bi + 0) * rows_per_block;
            int re = (bi + 1) * rows_per_block;
            if (re > r1) re = r1;
    
            int cs = (bj + 0) * cols_per_block;
            int ce = (bj + 1) * cols_per_block;
            if (ce > c2) ce = c2;

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
        }
    }

    return C;
}