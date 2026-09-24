#pragma once
#include <algorithm>
#include <vector>
#include <limits>
#ifdef _WIN32
    #include <chrono>
#else
    #include <ctime>
#endif

struct ProfilingResult {
    unsigned long long minNS;
    unsigned long long maxNS;
    unsigned long long medNS;
    unsigned long long avgNS;
    unsigned long long totalNS;
};

template <typename Setup, typename Kernel> unsigned long long time(Setup&& setup, Kernel&& kernel) {
    setup();

    #ifdef _WIN32
        auto start = std::chrono::steady_clock::now();
    #else
        //required to use clock_gettime
        struct timespec start, end;
        std::clock_gettime(CLOCK_MONOTONIC, &start);
    #endif
    
    kernel();

    #ifdef _WIN32
        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - start;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    #else
        std::clock_gettime(CLOCK_MONOTONIC, &end);
        unsigned long long seconds = end.tv_sec - start.tv_sec;
        unsigned long long nanoseconds = end.tv_nsec - start.tv_nsec;
        unsigned long long ns = (seconds * 1000000000LL) + nanoseconds;
    #endif

    return ns;
}

template <typename Setup, typename Kernel> ProfilingResult profile(Setup&& setup, Kernel&& kernel, int iterations) {
    // run once as warm-up
    time(setup, kernel);
    
    unsigned long long total = 0, 
        min = std::numeric_limits<unsigned long long>::max(), 
        max = std::numeric_limits<unsigned long long>::min();
        std::vector<unsigned long long> runTimes(iterations);
    for (int i = 0; i < iterations; i++) {
        auto ns = time(setup, kernel);
        total += ns;
        min = (ns < min) ? ns : min;
        max = (ns > max) ? ns : max;
        runTimes[i] = ns;
    }
 
    // find the median
    unsigned long long med;
    std::size_t n = runTimes.size();
    auto middle = runTimes.begin() + n / 2;    
    std::nth_element(runTimes.begin(), middle, runTimes.end());
    if (n % 2 != 0) {
        // if its odd, the median is the middle
        med = static_cast<unsigned long long>(*middle);
    } else {
        // otherwise we need to take the average...
        auto leftMiddle = runTimes.begin() + n / 2;
        std::nth_element(runTimes.begin(), leftMiddle, runTimes.end());
        unsigned long long val2 = *leftMiddle;
 
        auto rightMiddle = leftMiddle - 1;
        std::nth_element(runTimes.begin(), rightMiddle, leftMiddle);
        unsigned long long val1 = *rightMiddle;
        
        // i do it in a weird way to avoid imprecision
        med = val1 + (val2 - val1) / 2;
    }
 
    // create the results
    ProfilingResult results;
    results.minNS = min;
    results.maxNS = max;
    results.avgNS = total / iterations;
    results.medNS = med;
    results.totalNS = total;
    return results;
}

inline double nsToSec(unsigned long long ns) {
    unsigned long long seconds = ns / 1'000'000'000ULL;
    unsigned long long remainder = ns % 1'000'000'000ULL;
    return static_cast<double>(seconds) + (static_cast<double>(remainder) / 1'000'000'000.0);
}
