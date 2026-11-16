#include <iostream>
#include <omp.h>
#include <functional>

#include "assignment2.h"
#include "csv.h"
#include "matmul_tiled.h"
#include "matmul_pthreads.h"
#include "matmul_omp.h"

// regular old sequential implementation for testing / reference
double *mat_mul_seq_row(const double *A, int r1, int c1,
                        const double *B, int r2, int c2,
                        double *C)
{

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

bool warm_up_time_and_verify_mat_mul(const std::function<void(const double*,int,int,const double*,int,int, double*)>& func, 
                                    const double *A, int r1, int c1,
                                    const double *B, int r2, int c2,
                                    double* C, 
                                    const double* C_verified, 
                                    bool use_omp_time, std::ofstream& csv) {
    int n = r1 * c2;
    std::fill(C, C + n, 0.0);
    func(A, r1, c1, B, r2, c2, C); // warm up function

    for (int i = 0; i < 10; i++) {
        std::fill(C, C + n, 0.0);
        double time, elapsed;

        if (use_omp_time) time = omp_get_wtime();
        else time = now_seconds();
        func(A, r1, c1, B, r2, c2, C);
        if (use_omp_time) elapsed = omp_get_wtime() - time;
        else elapsed = now_seconds() - time;
    
        csvRow(csv, elapsed);

        if (!verify_mat(C_verified, C, r1, c2)) {
            std::cout << "ERROR! MATRICIES UNEQUAL!! RESULTS INVALID!" << std::endl;
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[])
{
    int arg_index = 1;
    std::string output_path = argv[arg_index++];

    std::string method = argv[arg_index++];
    int sizeA = std::stoi(argv[arg_index++]), sizeB = std::stoi(argv[arg_index++]), sizeC = std::stoi(argv[arg_index++]);
    int seed = std::stoi(argv[arg_index++]);

    int r1 = sizeA, c1 = sizeB;
    double *A = generate_matrix(r1, c1, (uint64_t) (seed + 0));

    int r2 = sizeB, c2 = sizeC;
    double *B = generate_matrix(r2, c2, (uint64_t) (seed + 1));

    int b = std::stoi(argv[arg_index++]);

    // used for verification
    double *C_ref = new double[r1 * c2]();
    mat_mul_seq_row(A, r1, c1, B, r2, c2, C_ref);
    
    double *C = new double[r1 * c2]();

    std::ofstream csv = csvStart(output_path, "time");
    if (!csv) return 1;

    bool failed = false;
    if (method == "tiled") {
        failed = warm_up_time_and_verify_mat_mul(
            [b](const double* p_a, int p_r1, int p_c1, const double* p_b, int p_r2, int p_c2, double* p_c) {
                mat_mul_seq_tiled(p_a, p_r1, p_c1, p_b, p_r2, p_c2, p_c, b);
            },
            A, r1, c1, B, r2, c2,
            C, C_ref, false, csv
        );
    } else if (method == "pthread-row") {
        int threads = std::stoi(argv[arg_index++]);
        failed = warm_up_time_and_verify_mat_mul(
            [threads, b](const double* p_a, int p_r1, int p_c1, const double* p_b, int p_r2, int p_c2, double* p_c) {
                mat_mul_pthread_row(p_a, p_r1, p_c1, p_b, p_r2, p_c2, p_c, threads, b);
            },
            A, r1, c1, B, r2, c2,
            C, C_ref, false, csv
        );
    } else if (method == "pthread-block") {
        int threadsX = std::stoi(argv[arg_index++]),
            threadsY = std::stoi(argv[arg_index++]);
        failed = warm_up_time_and_verify_mat_mul(
            [threadsX, threadsY, b](const double* p_a, int p_r1, int p_c1, const double* p_b, int p_r2, int p_c2, double* p_c) {
                mat_mul_pthread_block(p_a, p_r1, p_c1, p_b, p_r2, p_c2, p_c, threadsX, threadsY, b);
            },
            A, r1, c1, B, r2, c2,
            C, C_ref, false, csv
        );
    } else if (method == "omp-row") {
        int threads = std::stoi(argv[arg_index++]);
        failed = warm_up_time_and_verify_mat_mul(
            [threads, b](const double* p_a, int p_r1, int p_c1, const double* p_b, int p_r2, int p_c2, double* p_c) {
                mat_mul_omp_row(p_a, p_r1, p_c1, p_b, p_r2, p_c2, p_c, threads, b);
            },
            A, r1, c1, B, r2, c2,
            C, C_ref, false, csv
        );
    } else if (method == "omp-block") {
        int threadsX = std::stoi(argv[arg_index++]),
            threadsY = std::stoi(argv[arg_index++]);
        failed = warm_up_time_and_verify_mat_mul(
            [threadsX, threadsY, b](const double* p_a, int p_r1, int p_c1, const double* p_b, int p_r2, int p_c2, double* p_c) {
                mat_mul_omp_block(p_a, p_r1, p_c1, p_b, p_r2, p_c2, p_c, threadsX, threadsY, b);
            },
            A, r1, c1, B, r2, c2,
            C, C_ref, false, csv
        );
    }
    
    csvEnd(csv);

    if (failed) return 1;
    return 0;
}