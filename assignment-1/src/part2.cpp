#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "csv.h"
#include "fill.h"
#include "profile.h"

template <typename Num> void prefixSum(std::vector<Num>& a) {
    Num sum = static_cast<Num>(0);
    for (std::size_t i = 0; i < a.size(); i++) {
        Num value = a[i];
        a[i] = sum;
        sum += value;
    }
}

template <typename Num> void profilePrefixSumTemplated(std::ofstream& csv, const std::string& type, const std::string& optimizationLvl, int n, int i) {
    Num compilerHack = static_cast<Num>(0);

    std::vector<Num> a(n);
    ProfilingResult r = profile(
        [&](){ fill(a, FillType::Same); },
        [&](){ prefixSum(a); compilerHack += a.back(); }, i
    );
    csvRow(csv, n, optimizationLvl, type, (static_cast<double>(i) * n) / nsToSec(r.totalNS), r.minNS, r.maxNS, r.medNS, r.avgNS, r.totalNS);

    std::cout << compilerHack << std::endl;
}

void profilePrefixSum(const std::string& path, const std::string& optimizationLvl, int n, int i) {
    std::ofstream csv = csvStart(path, "n", "optimization_level", "type", "elements_per_sec", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");
    if (!csv) return;
    profilePrefixSumTemplated<int>(csv, "integer", optimizationLvl, n, i);
    profilePrefixSumTemplated<double>(csv, "double", optimizationLvl, n, i);
    csvEnd(csv);
}