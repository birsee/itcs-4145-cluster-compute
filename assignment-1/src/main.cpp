#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <random>
#include <functional>
#include <string>

#include <part1.h>
#include <part2.h>
#include <part3.h>
#include <part4.h>
#include <part5.h>

int main(int argc, char* argv[]) {
    std::iostream::sync_with_stdio(false);

    std::string operation = argv[1];
    std::cout << "profiling operation " << operation << std::endl;

    std::string output = argv[2];
    std::cout << "outputting results to " << output;

    if (operation == "array_max") {
        std::string bandwidthOutput = argv[3];
        std::cout << ", and bandwidth results to " << argv[3];

        int n = std::stoi(argv[4]), i = std::stoi(argv[5]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        profileArrayMax(output, n, i);
        profileArrayMaxBandwidth(bandwidthOutput, i);
    } else if (operation == "prefix_sum") {
        int n = std::stoi(argv[2]), i = std::stoi(argv[3]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        int optimizationLevel = std::stoi(argv[4]);

        profilePrefixSum(output, optimizationLevel, n, i);
    } else if (operation == "matmul") {
        int m = std::stoi(argv[2]), k = std::stoi(argv[3]), n = std::stoi(argv[4]), i = std::stoi(argv[5]);
        std::cout << ", m = " << m << ", k = " << k << ", n = " << n << ", i = " << i << std::endl;
    
        profileMatrixMult(output, m, k, n, i);
    } else if (operation == "merge_sort") {
        int n = std::stoi(argv[2]), i = std::stoi(argv[3]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        profileMergeSort(output, n, i);
    } else if (operation == "bfs") {
        std::string mtx = argv[3];
        int n = std::stoi(argv[2]), i = std::stoi(argv[3]);
        std::cout << ", mtx file: " << mtx << ", n = " << n << ", i = " << i << std::endl;

        profileBFS(output, mtx, n, i);
    }

    return 0;
}