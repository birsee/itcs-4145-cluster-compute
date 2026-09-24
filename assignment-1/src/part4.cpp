#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string>

#include "csv.h"
#include "fill.h"
#include "profile.h"

void merge(std::vector<int>& arr, std::vector<int>& temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int t = 0; t < k; ++t) arr[left + t] = temp[t];
}

void mergeSortReuse(std::vector<int>& arr, std::vector<int>& temp, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortReuse(arr, temp, left, mid);
    mergeSortReuse(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}

void mergeSortAllocate(std::vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortAllocate(arr, left, mid);
    mergeSortAllocate(arr, mid + 1, right);
    std::vector<int> temp(right - left + 1); // allocate a new temp array every time :-(
    merge(arr, temp, left, mid, right);
}

void profileMergeSort(const std::string& path, int n, int i) {
    int compilerHack = 0;
    std::ofstream csv = csvStart(path, "fill_type", "sort_type", "elements_per_sec", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");
    if (!csv) return;

    for (int j = 0; j < FillType::Count; ++j) {
        FillType fillType = static_cast<FillType>(j);
        std::string ftName = fillTypeName(fillType);
        long totalItems = static_cast<long>(n) * i;

        std::vector<int> original(n);
        fill(original, fillType);
    
        std::vector<int> stdSortVec(n);
        ProfilingResult stdSortResult = profile(
            [&](){ stdSortVec = original; },
            [&](){
                sort(stdSortVec.begin(), stdSortVec.end());
                compilerHack += stdSortVec.back();
            }, i);
        csvRow(csv, ftName, "std", totalItems / nsToSec(stdSortResult.totalNS), stdSortResult.minNS, stdSortResult.maxNS, stdSortResult.medNS, stdSortResult.avgNS, stdSortResult.totalNS);
    
        std::vector<int> reuseVec(n);
        ProfilingResult reuseResult = profile(
            [&](){ reuseVec = original; },
            [&](){
                std::vector<int> temp(n);
                mergeSortReuse(reuseVec, temp, 0, n - 1);
                compilerHack += reuseVec.back();
            }, i);
        csvRow(csv, ftName, "reuse", totalItems / nsToSec(reuseResult.totalNS), reuseResult.minNS, reuseResult.maxNS, reuseResult.medNS, reuseResult.avgNS, reuseResult.totalNS);

        std::vector<int> allocateVec(n);
        ProfilingResult allocateResult = profile(
            [&](){ allocateVec = original; },
            [&](){
                mergeSortAllocate(allocateVec, 0, n - 1);
                compilerHack += allocateVec.back();
            }, i);
        csvRow(csv, ftName, "allocate", totalItems / nsToSec(allocateResult.totalNS), allocateResult.minNS, allocateResult.maxNS, allocateResult.medNS, allocateResult.avgNS, allocateResult.totalNS);

        // verify it is sorted and contains all original elements...
        // std::sort sort is known to work, so we can compare against that.
        if ((reuseVec != stdSortVec) || (allocateVec != stdSortVec)) std::cerr << "VERIFICATION FAILED: SORTED ARRAY ISN'T SORTED PROPERLY!!" << std::endl;
    }
   
    csvEnd(csv);
    std::cout << compilerHack<< std::endl;
}