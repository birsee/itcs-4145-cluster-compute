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

// just defers to the different parts
int main(int argc, char* argv[]) {
    std::iostream::sync_with_stdio(false);

    std::string operation = argv[1];
    std::cout << "profiling operation " << operation << std::endl;

    std::string output = argv[2];
    std::cout << "outputting results to " << output;

    if (operation == "array_max") {
        int n = std::stoi(argv[3]), i = std::stoi(argv[4]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        profileArrayMax(output, n, i);
    } else if (operation == "array_bandwidth") {
        int i = std::stoi(argv[3]);
        std::cout << ", i = " << i << std::endl;

        profileArrayMaxBandwidth(output, i);
    } else if (operation == "prefix_sum") {
        int n = std::stoi(argv[3]), i = std::stoi(argv[4]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        std::string optimizationLevel = argv[5];

        profilePrefixSum(output, optimizationLevel, n, i);
    } else if (operation == "matmul") {
        int m = std::stoi(argv[3]), k = std::stoi(argv[4]), n = std::stoi(argv[5]), i = std::stoi(argv[6]);
        std::cout << ", m = " << m << ", k = " << k << ", n = " << n << ", i = " << i << std::endl;
    
        profileMatrixMult(output, m, k, n, i);
    } else if (operation == "merge_sort") {
        int n = std::stoi(argv[3]), i = std::stoi(argv[4]);
        std::cout << ", n = " << n << ", i = " << i << std::endl;

        profileMergeSort(output, n, i);
    } else if (operation == "bfs") {
        std::string searchOutput = argv[3];
        std::string mtx = argv[4];
        int n = std::stoi(argv[5]), i = std::stoi(argv[6]);
        std::cout << ", mtx file: " << mtx << ", n = " << n << ", i = " << i << std::endl;

        profileBFS(output, searchOutput, mtx, n, i);
    }

    return 0;
}