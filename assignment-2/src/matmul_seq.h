#pragma once
#include <algorithm>

// regular old sequential implementation for testing / reference
double *mat_mul_seq_row(const double *A, int r1, int c1,
                        const double *B, int r2, int c2)
{
    double *C = new double[r1 * c2]();

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

double* mat_mul_seq_tiled(const double *A, int r1, int c1,
                          const double *B, int r2, int c2,
                          int b)
{
    double *C = new double[r1 * c2]();

    int bi = (b > 0) ? b : r1;
    int bj = (b > 0) ? b : c2;
    int bk = (b > 0) ? b : c1;

    for (int ii = 0; ii < r1; ii += bi)
    {
        int ie = std::min(ii + bi, r1);
        for (int jj = 0; jj < c2; jj += bj)
        {
            int je = std::min(jj + bj, c2);

            for (int kk = 0; kk < c1; kk += bk)
            {
                int ke = std::min(kk + bk, c1);

                // funky inner block loop
                for (int i = ii; i < ie; i++)
                {
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

    return C;
}