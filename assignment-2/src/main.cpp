#include <iostream>

#include "assignment2.h"
#include "matmul_seq.h"
#include "matmul_pthreads.h"
#include "matmul_openmp.h"

// makes sure that two arrays are actually the same array
bool verify_mat(const double *A, const double *B, int r, int c)
{
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            int index = i * c + j;
            if (A[index] != B[index]) return false;
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    std::cout << "hello world!" << std::endl;

    std::string method = argv[1];
    int sizeA = std::stoi(argv[2]), sizeB = std::stoi(argv[3]), sizeC = std::stoi(argv[4]);
    int seed = std::stoi(argv[5]);

    int r1 = sizeA, c1 = sizeB;
    double *A = generate_matrix(r1, c1, (uint64_t) (seed + 0));

    int r2 = sizeB, c2 = sizeC;
    double *B = generate_matrix(r2, c2, (uint64_t) (seed + 1));

    // used for verification
    double *C_seq = mat_mul_seq_row(A, r1, c1, B, r2, c2);
    double *C;

    double time = now_seconds();
    if (method == "tiled") {
        int b = std::stoi(argv[6]);
        C = mat_mul_seq_tiled(A, r1, c1, B, r2, c2, b);
    } else if (method == "pthread_row") {
        int threads = std::stoi(argv[6]);
        C = mat_mul_pthread_row(A, r1, c1, B, r2, c2, threads);
    } else if (method == "pthread_block") {
        int threadsX = std::stoi(argv[6]),
            threadsY = std::stoi(argv[6]);
        C = mat_mul_pthread_block(A, r1, c1, B, r2, c2, threadsX, threadsY);
    } else if (method == "omp_row") {
        int threads = std::stoi(argv[6]);
        C = mat_mul_omp_row(A, r1, c1, B, r2, c2, threads);
    } else if (method == "omp_block") {
        int threadsX = std::stoi(argv[6]),
            threadsY = std::stoi(argv[6]);
        C = mat_mul_omp_block(A, r1, c1, B, r2, c2, threadsX, threadsY);
    }
    double elapsed = now_seconds() - time;

    if (!verify_mat(C_seq, C, r1, c2)) {
        std::cout << "ERROR! MATRICIES UNEQUAL!! RESULTS INVALID!" << std::endl;
    }

    return 0;
}