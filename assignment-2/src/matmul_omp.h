#pragma once
#include <algorithm>
#include <omp.h>

#include "matmul_tiled.h"

void mat_mul_omp_block(const double *A, int r1, int c1,
                       const double *B, int r2, int c2,
                       double *C,
                       int pr, int pc, int b)
{
#pragma omp parallel for collapse(2) num_threads(pr *pc) schedule(static)
    for (int bi = 0; bi < pr; bi++)
    {
        for (int bj = 0; bj < pc; bj++)
        {
            int rs, re, cs, ce;
            block_range(r1, pr, bi, rs, re);
            block_range(c2, pc, bj, cs, ce);
            tiled_kernel(A, B, C, c1, c2, rs, re, cs, ce, b);
        }
    }
}

void mat_mul_omp_row(const double *A, int r1, int c1,
                     const double *B, int r2, int c2,
                     double *C, int p, int b)
{
    mat_mul_omp_block(A, r1, c1, B, r2, c2, C, p, 1, b);
}