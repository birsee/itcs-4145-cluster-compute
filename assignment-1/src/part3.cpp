#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string>

#include "csv.h"
#include "fill.h"
#include "profile.h"

void matrixMultIJK(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
        double acc = 0.0;
        for (int k = 0; k < K; k++) acc += A[i * K + k] * B[k * N + j];
        C[i * N + j] = acc;
    }
}
void matrixMultIKJ(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int i = 0; i < M; i++)
    for (int k = 0; k < K; k++) {
        double a = A[i * K + k];
        for (int j = 0; j < N; j++) C[i * N + j] += a * B[k * N + j];
    }
}
void matrixMultJIK(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int j = 0; j < N; j++)
    for (int i = 0; i < M; i++) {
        double acc = 0.0;
        for (int k = 0; k < K; k++) acc += A[i * K + k] * B[k * N + j];
        C[i * N + j] = acc;
    }
}
void matrixMultJKI(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int j = 0; j < N; j++)
    for (int k = 0; k < K; k++) {
        double b = B[k * N + j];
        for (int i = 0; i < M; i++) C[i * N + j] += A[i * K + k] * b;
    }
}
void matrixMultKIJ(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int k = 0; k < K; k++)
    for (int i = 0; i < M; i++) {
        double a = A[i * K + k];
        for (int j = 0; j < N; j++) C[i * N + j] += a * B[k * N + j];
    }
}
void matrixMultKJI(const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C, int M, int K, int N) {
    for (int k = 0; k < K; k++)
    for (int j = 0; j < N; j++) {
        double b = B[k * N + j];
        for (int i = 0; i < M; i++) C[i * N + j] += A[i * K + k] * b;
    }
}
template <typename MultiplyFunction> std::vector<double> profileMatrixMultFunc(std::ofstream& csv, const std::string& type, MultiplyFunction func, 
                                                            int m, int k, int n, int i) {
    double compilerHack = 0.0;
    std::vector<double> a(m * k), b(k * n), c(m * n);
    ProfilingResult r = profile(
        [&](){ 
            fill(a, FillType::Random);
            fill(b, FillType::Random);
            std::fill(c.begin(), c.end(), 0.0);
        }, 
        [&](){ 
            func(a, b, c, m, k, n);
            compilerHack += c.back();
        }, 
        i
    );
    long double flops = (2 * m * k * n * i) / nsToSec(r.totalNS);
    csvRow(csv, "" + std::to_string(m) + "x" + std::to_string(k) + " " + std::to_string(k) + "x" + std::to_string(n), type, flops, r.minNS, r.maxNS, r.medNS, r.avgNS, r.totalNS);

    std::cout << compilerHack << std::endl;

    return c;
}

void profileMatrixMult(const std::string& path, int m, int k, int n, int i) {
    std::ofstream csv = csvStart(path, "size", "type", "flops", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");
    auto r0 = profileMatrixMultFunc(csv, "IJK", matrixMultIJK, m, k, n, i);

    auto r1 = profileMatrixMultFunc(csv, "IKJ", matrixMultIKJ, m, k, n, i);
    // verify they are the same, always
    if (r0 != r1) std::cerr << "VERIFICATION FAILED: MATRICES HAVE DIFFERENT RESULTS!" << std::endl;
    r0 = r1;

    r1 = profileMatrixMultFunc(csv, "JIK", matrixMultJIK, m, k, n, i);
    if (r0 != r1) std::cerr << "VERIFICATION FAILED: MATRICES HAVE DIFFERENT RESULTS!" << std::endl;
    r0 = r1;

    r1 = profileMatrixMultFunc(csv, "JKI", matrixMultJKI, m, k, n, i);
    if (r0 != r1) std::cerr << "VERIFICATION FAILED: MATRICES HAVE DIFFERENT RESULTS!" << std::endl;
    r0 = r1;

    r1 = profileMatrixMultFunc(csv, "KIJ", matrixMultKIJ, m, k, n, i);
    if (r0 != r1) std::cerr << "VERIFICATION FAILED: MATRICES HAVE DIFFERENT RESULTS!" << std::endl;
    r0 = r1;

    r1 = profileMatrixMultFunc(csv, "KJI", matrixMultKJI, m, k, n, i);
    if (r0 != r1) std::cerr << "VERIFICATION FAILED: MATRICES HAVE DIFFERENT RESULTS!" << std::endl;
    
    csvEnd(csv);
}