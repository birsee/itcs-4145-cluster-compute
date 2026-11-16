#pragma once
#include <algorithm>

void block_range(int n, int parts, int idx, int &s, int &e)
{
    int base = n / parts;
    s = base * idx;
    e = (idx == parts - 1) ? n : s + base;
}

void tiled_kernel(const double *A, const double *B, double *C,
                  int c1, int c2,
                  int rs, int re, int cs, int ce,
                  int b)
{
    if (rs >= re || cs >= ce)
        return;

    int bi = (b > 0) ? b : re - rs;
    int bj = (b > 0) ? b : ce - cs;
    int bk = (b > 0) ? b : c1;

    for (int ii = rs; ii < re; ii += bi)
    {
        int ie = std::min(ii + bi, re);
        for (int jj = cs; jj < ce; jj += bj)
        {
            int je = std::min(jj + bj, ce);
            for (int kk = 0; kk < c1; kk += bk)
            {
                int ke = std::min(kk + bk, c1);
                for (int i = ii; i < ie; i++)
                    for (int k = kk; k < ke; k++)
                    {
                        double aik = A[i * c1 + k];
                        for (int j = jj; j < je; j++)
                            C[i * c2 + j] += aik * B[k * c2 + j];
                    }
            }
        }
    }
}

void mat_mul_seq_tiled(const double *A, int r1, int c1,
                       const double *B, int r2, int c2,
                       double *C, int b)
{
    tiled_kernel(A, B, C, c1, c2, 0, r1, 0, c2, b);
}