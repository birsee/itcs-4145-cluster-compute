#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#include "csv.h"
#include "fill.h"
#include "profile.h"

template <typename Num> Num arrayMax(const std::vector<Num>& a) {
    Num max = a[0];
    for (std::size_t i = 1; i < a.size(); i++) if (a[i] > max) max = a[i];
    return max;
}

template <typename Num> Num arrayMaxBranchless(const std::vector<Num>& a) {
    Num max = a[0];
    for (std::size_t i = 1; i < a.size(); i++) max = (a[i] > max) ? a[i] : max;
    return max;
}

template <typename Num> void profileArrayTemplated(std::ofstream& csv, const std::string& type, int n, int i) {
    Num compilerHack = static_cast<Num>(0);

    unsigned long long totalBytes = n * sizeof(Num) * i;
    ProfilingResult r;
    for (int j = 0; j < FillType::Count; ++j) {
        FillType fillType = static_cast<FillType>(j);
        for (bool branchless : {false, true}) {
            std::vector<Num> a(n);
            r = branchless ? 
                profile( [&](){ fill(a, fillType); }, [&](){ compilerHack += arrayMaxBranchless(a); }, i) : 
                profile( [&](){ fill(a, fillType); }, [&](){ compilerHack += arrayMax(a); }, i);
            csvRow(csv, n, type, branchless, totalBytes / nsToSec(r.totalNS), r.minNS, r.maxNS, r.medNS, r.avgNS, r.totalNS);
        }
    }

    std::cout << compilerHack << std::endl;
}

void profileArrayMax(const std::string& path, int n, int i) {
    ProfilingResult r;
    std::ofstream csv = csvStart(path, "n", "type", "branchless", "bytes_per_sec", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");
    if (!csv) return;
    profileArrayTemplated<int>(csv, "integer", n, i);
    profileArrayTemplated<double>(csv, "double", n, i);
    csvEnd(csv);
}

void profileArrayMaxBandwidth(const std::string& path, int i) {
    double compilerHack = 0.0;

    ProfilingResult r;
    std::ofstream csv = csvStart(path, "n", "bytes_per_sec", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");
    if (!csv) return;

    for (long long bytes = 2LL * 1024; bytes <= 256LL * 1024 * 1024; bytes *= 2) {
        unsigned long long n = bytes / static_cast<long long>(sizeof(double));
        unsigned long long totalBytes = n * sizeof(double) * i;

        std::vector<double> a(n);
        r = profile( [&](){ fill(a, FillType::Sorted); }, [&](){ compilerHack += arrayMax(a); }, i);

        csvRow(csv, n, totalBytes / nsToSec(r.totalNS), r.minNS, r.maxNS, r.medNS, r.avgNS, r.totalNS);
    }
    csvEnd(csv);

    std::cout << compilerHack << std::endl;
}