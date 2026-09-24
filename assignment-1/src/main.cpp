#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <random>
#include <functional>
#include <string>

#ifdef _WIN32
    #include <chrono>
#else
    #include <ctime>
#endif

using namespace std;

// this is like the worst possible way to do this, but it's 3 AM
// and i really don't want to deal with header files right now.
// Sorry...

// reused helper stuffs
template <typename Setup, typename Kernel> unsigned long long time(Setup&& setup, Kernel&& kernel) {
    setup();

    #ifdef _WIN32
        auto start = chrono::steady_clock::now();
    #else
        //required to use clock_gettime
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
    #endif
    
    kernel();

    #ifdef _WIN32
        auto end = chrono::steady_clock::now();
        auto elapsed = end - start;
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    #else
        clock_gettime(CLOCK_MONOTONIC, &end);
        unsigned long long seconds = end.tv_sec - start.tv_sec;
        unsigned long long nanoseconds = end.tv_nsec - start.tv_nsec;
        unsigned long long ns = (seconds * 1000000000LL) + nanoseconds;
    #endif

    return ns;
}
double secs(unsigned long long ns) {
    unsigned long long seconds = ns / 1'000'000'000ULL;
    unsigned long long remainder = ns % 1'000'000'000ULL;
    return static_cast<double>(seconds) + (static_cast<double>(remainder) / 1'000'000'000.0);
}

struct ProfilingResult {
    unsigned long long min;
    unsigned long long max;
    unsigned long long med;
    unsigned long long avg;
    unsigned long long total;

    friend std::ostream& operator<<(std::ostream& os, const ProfilingResult& r) {
        return os << "MIN: " << r.min << " ns | " << secs(r.min) << " s" << endl 
                  << "MAX: " << r.max << " ns | " << secs(r.max) << " s" << endl 
                  << "AVG: " << r.avg << " ns | " << secs(r.avg) << " s" << endl 
                  << "MED: " << r.med << " ns | " << secs(r.med) << " s" << endl 
                  << "TOTAL: " << r.total << " ns | " << secs(r.total) << " s";
    }
};
template <typename Setup, typename Kernel> ProfilingResult profile(Setup&& setup, Kernel&& kernel, int iterations) {
    // run once as warm-up
    time(setup, kernel);
    
    unsigned long long total = 0, 
        min = numeric_limits<unsigned long long>::max(), 
        max = numeric_limits<unsigned long long>::min();
    vector<unsigned long long> runTimes(iterations);
    for (int i = 0; i < iterations; i++) {
        auto ns = time(setup, kernel);
        total += ns;
        min = (ns < min) ? ns : min;
        max = (ns > max) ? ns : max;
        runTimes[i] = ns;
    }

    // find the median
    unsigned long long med;
    size_t n = runTimes.size();
    auto middle = runTimes.begin() + n / 2;    
    std::nth_element(runTimes.begin(), middle, runTimes.end());
    if (n % 2 != 0) {
        med = static_cast<unsigned long long>(*middle);
    } else {
        auto leftMiddle = runTimes.begin() + n / 2;
        std::nth_element(runTimes.begin(), leftMiddle, runTimes.end());
        unsigned long long val2 = *leftMiddle;

        auto rightMiddle = leftMiddle - 1;
        std::nth_element(runTimes.begin(), rightMiddle, leftMiddle);
        unsigned long long val1 = *rightMiddle;
        
        med = val1 + (val2 - val1) / 2;
    }

    // create the results
    ProfilingResult results;
    results.min = min;
    results.max = max;
    results.avg = total / iterations;
    results.med = med;
    results.total = total;
    return results;
}

enum class ArrayType { Same, Sorted, Reverse, Random };
string arrayTypeName(ArrayType type) {
    switch (type) {
        case ArrayType::Same: return "Same";
        case ArrayType::Sorted: return "Sorted";
        case ArrayType::Reverse: return "Reverse";
        case ArrayType::Random: return "Random";
        default: return "Unknown";
    }
}
template <typename T> void fillArray(vector<T>& a, ArrayType type) {
    size_t n = a.size();
    switch (type) {
        case ArrayType::Same: 
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(1);
            break;
        case ArrayType::Sorted:
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(i);
            break;
        case ArrayType::Reverse:
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(n - i);
            break;
        case ArrayType::Random: 
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(rand() % 1000);
            break;
    }
}

// hack to make sure the compiler doesnt optimize away everything.
static double compilerHack = 0.0;

// part 1
template <typename T> T arrayMax(const vector<T>& a) {
    T m = a[0];
    for (size_t i = 1; i < a.size(); i++) if (a[i] > m) m = a[i];
    return m;
}
template <typename T> T arrayMaxBranchless(const vector<T>& a) {
    T m = a[0];
    for (size_t i = 1; i < a.size(); i++) m = (a[i] > m) ? a[i] : m;
    return m;
}
ProfilingResult profileArrayMax(int n, int i, bool integer, bool branchless, ArrayType type) {
    if (integer) {
        vector<int> a(n);
        if (branchless) {
            return profile(
                [&](){ fillArray(a, type); },
                [&](){ compilerHack += arrayMaxBranchless(a); }, i
            );
        } else {
            return profile(
                [&](){ fillArray(a, type); },
                [&](){ compilerHack += arrayMax(a); }, i
            );
        }
    } else {
        vector<double> a(n);
        if (branchless) {
            return profile(
                [&](){ fillArray(a, type); },
                [&](){ compilerHack += arrayMaxBranchless(a); }, i
            );
        } else {
            return profile(
                [&](){ fillArray(a, type); },
                [&](){ compilerHack += arrayMax(a); }, i
            );
        }
    }
}
void profileArrayMaxSet(int n, int i) {
    // ints
    cout << "--- Integer ---" << endl;
    unsigned long long totalBytes = n * sizeof(int) * i;

    ProfilingResult result = profileArrayMax(n, i, true, false, ArrayType::Sorted);
    cout << "Sorted: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, true, true, ArrayType::Sorted);
    cout << "Sorted, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, true, false, ArrayType::Reverse);
    cout << "Reverse-sorted: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, true, true, ArrayType::Reverse);
    cout << "Reverse-sorted, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, true, false, ArrayType::Random);
    cout << "Randomized: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, true, true, ArrayType::Random);
    cout << "Randomized, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;

    // doubles
    totalBytes = n * sizeof(double) * i;
    cout << "--- Double ---" << endl;
    result = profileArrayMax(n, i, false, true, ArrayType::Sorted);
    cout << "Sorted, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, false, false, ArrayType::Reverse);
    cout << "Reverse-sorted: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, false, true, ArrayType::Reverse);
    cout << "Reverse-sorted, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, false, false, ArrayType::Random);
    cout << "Randomized: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
    result = profileArrayMax(n, i, false, true, ArrayType::Random);
    cout << "Randomized, branchless: " << endl << result << endl << "BYTES/S: " << (totalBytes / secs(result.total)) << endl << endl;
}

void profileArrayMaxBandwidth(int i) {
    for (long long bytes = 2LL * 1024; bytes <= 256LL * 1024 * 1024; bytes *= 2) {
        unsigned long long n = bytes / static_cast<long long>(sizeof(double));
        unsigned long long totalBytes = n * sizeof(double) * i;

        ProfilingResult result = profileArrayMax(n, i, false, false, ArrayType::Random);
        double bandwidth = totalBytes / secs(result.total);
        cout << n << " doubles:" << endl << "BANDWIDTH: " << bandwidth << endl << result << endl << endl;
    }
}

// part 2
template <typename T> void prefixSum(vector<T>& a) {
    T sum = static_cast<T>(0);
    for (size_t i = 0; i < a.size(); i++) {
        T value = a[i];
        a[i] = sum;
        sum += value;
    }
}
ProfilingResult profilePrefixSum(int n, int i, bool integer, ArrayType type) {
    if (integer) {
        vector<int> a(n);
        return profile(
            [&](){ fillArray(a, type); },
            [&](){ prefixSum(a); compilerHack += a.back(); }, i
        );
    } else {
        vector<double> a(n);
        return profile(
            [&](){ fillArray(a, type); },
            [&](){ prefixSum(a); compilerHack += a.back(); }, i
        );
    }
}
void profilePrefixSumSet(int n, int i) {
    ProfilingResult result = profilePrefixSum(n, i, true, ArrayType::Same);
    long double rate = (n * i) / secs(result.total);
    cout << "Integer: " << endl << result << endl << "ELEMENTS/S: " << rate << endl << endl;

    result = profilePrefixSum(n, i, false, ArrayType::Same);
    rate = (n * i) / secs(result.total);
    cout << "Doubles: " << endl << profilePrefixSum(n, i, false, ArrayType::Same) << endl << "ELEMENTS/S: " << rate << endl << endl;
}

// part 3
void matrixMultIJK(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
        double acc = 0.0;
        for (int k = 0; k < K; k++) acc += A[i * K + k] * B[k * N + j];
        C[i * N + j] = acc;
    }
}
void matrixMultIKJ(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int i = 0; i < M; i++)
    for (int k = 0; k < K; k++) {
        double a = A[i * K + k];
        for (int j = 0; j < N; j++) C[i * N + j] += a * B[k * N + j];
    }
}
void matrixMultJIK(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int j = 0; j < N; j++)
    for (int i = 0; i < M; i++) {
        double acc = 0.0;
        for (int k = 0; k < K; k++) acc += A[i * K + k] * B[k * N + j];
        C[i * N + j] = acc;
    }
}
void matrixMultJKI(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int j = 0; j < N; j++)
    for (int k = 0; k < K; k++) {
        double b = B[k * N + j];
        for (int i = 0; i < M; i++) C[i * N + j] += A[i * K + k] * b;
    }
}
void matrixMultKIJ(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int k = 0; k < K; k++)
    for (int i = 0; i < M; i++) {
        double a = A[i * K + k];
        for (int j = 0; j < N; j++) C[i * N + j] += a * B[k * N + j];
    }
}
void matrixMultKJI(const vector<double>& A, const vector<double>& B, vector<double>& C, int M, int K, int N) {
    for (int k = 0; k < K; k++)
    for (int j = 0; j < N; j++) {
        double b = B[k * N + j];
        for (int i = 0; i < M; i++) C[i * N + j] += A[i * K + k] * b;
    }
}
template <typename MultiplyFunction> void profileMatrixMult(const string& name, MultiplyFunction func, const vector<double>& a, const vector<double>& b, vector<double>& c, int m, int k, int n, int i) {
    fill(c.begin(), c.end(), 0.0); // reset c
    ProfilingResult results = profile([](){}, [&](){ func(a, b, c, m, k, n); compilerHack += c.back(); }, i);
    
    long double rate = (2.0 * m * k * n * i) / secs(results.total);
    cout << name << ": " << results << endl << "FLOPS: " << rate << endl << endl;
}
void profileMatrixMultSet(int m, int k, int n, int i) {
    vector<double> a(m*k);
    vector<double> b(k*n);
    vector<double> c(m*n);
    // fill with random garbage
    fillArray(a, ArrayType::Random);
    fillArray(b, ArrayType::Random);

    profileMatrixMult("IJK", matrixMultIJK, a, b, c, m, k, n, i);
    profileMatrixMult("IKJ", matrixMultIKJ, a, b, c, m, k, n, i);
    profileMatrixMult("JIK", matrixMultJIK, a, b, c, m, k, n, i);
    profileMatrixMult("JKI", matrixMultJKI, a, b, c, m, k, n, i);
    profileMatrixMult("KIJ", matrixMultKIJ, a, b, c, m, k, n, i);
    profileMatrixMult("KJI", matrixMultKJI, a, b, c, m, k, n, i);
}

// part 4
void merge(vector<int>& arr, vector<int>& temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (i = left; i <= right; ++i) arr[i] = temp[i];
}
void mergeSortReuse(vector<int>& arr, vector<int>& temp, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortReuse(arr, temp, left, mid);
    mergeSortReuse(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}
void mergeSortAllocate(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;
    mergeSortAllocate(arr, left, mid);
    mergeSortAllocate(arr, mid + 1, right);
    vector<int> temp(arr.size()); // allocate a new temp array every time :-(
    merge(arr, temp, left, mid, right);
}
void profileMergeSort(int n, int i, ArrayType type) {
    vector<int> stdSortVec(n);
    fillArray(stdSortVec, type);

    unsigned long long totalItems = n * i;

    ProfilingResult stdSortResult = profile([](){}, [&](){
        sort(stdSortVec.begin(), stdSortVec.end());
        compilerHack += stdSortVec.back();
    }, i);
    cout << "std::sort: " << endl << stdSortResult 
         << "RATE: " << totalItems / secs(stdSortResult.total) << " elements/s" << endl << endl;

    vector<int> reuseVec(stdSortVec);
    ProfilingResult reuseResult = profile([](){}, [&](){
        vector<int> temp(reuseVec.size());
        mergeSortReuse(reuseVec, temp, 0, n - 1);
        compilerHack += reuseVec.back();
    }, i);
    cout << "Reuse: " << endl << reuseResult 
         << "RATE: " << totalItems / secs(reuseResult.total) << " elements/s" << endl << endl;

    vector<int> allocateVec(stdSortVec);
    ProfilingResult allocateResult = profile([](){}, [&](){
        mergeSortAllocate(allocateVec, 0, n - 1);
        compilerHack += allocateVec.back();
    }, i);
    cout << "Allocate: " << endl << allocateResult 
         << "RATE: " << n / secs(allocateResult.avg) << " elements/s" << endl << endl;

    // verify it is sorted and contains all original elements...
    // std::sort sort is known to work, so we can compare against that.
    for (size_t i = 0; i < n; i++) {
        int aVal = reuseVec[i];
        int bVal = allocateVec[i];
        int correctVal = stdSortVec[i];

        if (aVal != correctVal || bVal != correctVal) {
            cout << "VERIFICATION FAILED: SORTED ARRAY ISN'T SORTED PROPERLY!!" << endl;
            break;
        }
    }
}

void profileMergeSortSet(int n, int i) {
    vector<ArrayType> types = {ArrayType::Same, ArrayType::Sorted, ArrayType::Reverse, ArrayType::Random};
    for (size_t t = 0; t < types.size(); t++) {
        ArrayType type = types[t];
        cout << arrayTypeName(type) << ": " << endl;
        profileMergeSort(n, i, type);
    }
}

// part 5
struct CSRGraph {
    long vertexCount;
    long edgeCount;
    vector<long> vertexToAdjacenceniesStart;
    vector<long> adjacencies;
};
// lots of google-fu here. i barely understand this at all
CSRGraph loadFromMTX(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cout << "couldn't open " + filename << endl;
        CSRGraph graph;
        graph.vertexCount = 0;
        graph.edgeCount = 0;
        return graph;
    }
 
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;
        break;
    }
    
    long numRows, numCols, numLines;
    {
        istringstream iss(line);
        iss >> numRows >> numCols >> numLines;
    }
 
    CSRGraph graph;
    graph.vertexCount = numRows;
 
    struct Edge { long src, dst; };
    vector<Edge> tempEdges;
    tempEdges.reserve(numLines * 2);
 
    for (long i = 0; i < numLines; i++) {
        long u, v;
        if (!(file >> u >> v)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        int srcNode = static_cast<long>(u - 1);
        long destNode = static_cast<long>(v - 1);
        tempEdges.push_back({srcNode, destNode});
        if (srcNode != destNode) tempEdges.push_back({destNode, srcNode});
    }
    file.close();
 
    sort(tempEdges.begin(), tempEdges.end(), [](const Edge& a, const Edge& b) {
        if (a.src != b.src) return a.src < b.src;
        return a.dst < b.dst;
    });
 
    graph.edgeCount = static_cast<long>(tempEdges.size());
    graph.vertexToAdjacenceniesStart.assign(graph.vertexCount + 1, 0);
    graph.adjacencies.resize(tempEdges.size());
 
    for (const Edge& e : tempEdges) graph.vertexToAdjacenceniesStart[e.src + 1]++;
    for (long i = 1; i <= graph.vertexCount; i++) graph.vertexToAdjacenceniesStart[i] += graph.vertexToAdjacenceniesStart[i - 1];
    for (size_t i = 0; i < tempEdges.size(); i++) graph.adjacencies[i] = tempEdges[i].dst;
 
    return graph;
}
struct BFSResult {
    int totalLevels;
    int totalVerticesReached;
    int totalEdgesEvaluated;
    double fractionReached;
    vector<int> frontierSizes;
};
BFSResult bfs(int startNode, CSRGraph& graph) {
    queue<int> frontier;
    vector<bool> visited(graph.vertexCount, false);

    frontier.push(startNode);
    visited[startNode] = true;

    BFSResult result;
    result.totalLevels = 0;
    result.totalVerticesReached = 0;
    result.totalEdgesEvaluated = 0;

    while (!frontier.empty()) {
        int currentFrontierSize = frontier.size();
        result.totalVerticesReached += currentFrontierSize;
        result.frontierSizes.push_back(currentFrontierSize);

        for (int k = 0; k < currentFrontierSize; k++) {
            int nextNode = frontier.front();
            frontier.pop();

            int adjacenciesStart = graph.vertexToAdjacenceniesStart[nextNode];
            int adjacenciesEnd = graph.vertexToAdjacenceniesStart[nextNode + 1];
            
            for (int i = adjacenciesStart; i < adjacenciesEnd; i++) {
                int nodeToVisit = graph.adjacencies[i];
                result.totalEdgesEvaluated++;
                if (visited[nodeToVisit]) continue;

                visited[nodeToVisit] = true;
                frontier.push(nodeToVisit);
            }
        }

        result.totalLevels++;
    }

    result.fractionReached = static_cast<double>(result.totalVerticesReached) / graph.vertexCount;

    return result;
}
void profileBFS(const string& filename, int i) {
    CSRGraph graph = loadFromMTX(filename);
    if (graph.edgeCount == 0) return;

    vector<int> startNodes;
    srand(42);
    while(startNodes.size() < static_cast<size_t>(i)) {
        int v = rand() % graph.vertexCount;
        if (graph.vertexToAdjacenceniesStart[v+1] > graph.vertexToAdjacenceniesStart[v])
            startNodes.push_back(v);
    }

    vector<double> edgesPerSecondResults;
    
    for (int startNode : startNodes) {
        BFSResult result;
        unsigned long long nsTaken = time(
            [](){}, 
            [&](){ result = bfs(startNode, graph); }
        );
        
        double edgesPerSec = result.totalEdgesEvaluated / secs(nsTaken);
        edgesPerSecondResults.push_back(edgesPerSec);
        
        cout << "Results for starting from node " << startNode << endl
             << "LEVELS: " << result.totalLevels << endl
             << "% REACHED: " << result.fractionReached << endl
             << "EDGES INSPECTED: " << result.totalEdgesEvaluated << endl
             << "TEPS: " << edgesPerSec << endl << endl;
    }
    
    sort(edgesPerSecondResults.begin(), edgesPerSecondResults.end());
    double min_teps = edgesPerSecondResults.front();
    double max_teps = edgesPerSecondResults.back();
    double med_teps = edgesPerSecondResults[edgesPerSecondResults.size() / 2];
    
    cout << "TEPS Totals:" << endl
         << "MIN: " << min_teps << endl 
         << "MEDIAN: " << med_teps << endl
         << "MAX: " << max_teps << endl << endl;
}

// the actual program
int main(int argc, char* argv[]) {
    iostream::sync_with_stdio(false);
    cout << "Starting profiling..." << endl;
    cout << "Profiling operation " << argv[1];

    if (strcmp(argv[1], "array_max") == 0) {
        int n = stoi(argv[2]), i = stoi(argv[3]);
        cout << ", n = " << n << ", i = " << i << endl;

        cout << "Array Max:" << endl;
        profileArrayMaxSet(n, i);
        cout << "Bandwidth" << endl;
        profileArrayMaxBandwidth(i);
    } else if (strcmp(argv[1], "prefix_sum") == 0) {
        int n = stoi(argv[2]), i = stoi(argv[3]);
        cout << ", n = " << n << ", i = " << i << endl;

        profilePrefixSumSet(n, i);
    } else if (strcmp(argv[1], "matmul") == 0) {
        int m = stoi(argv[2]), k = stoi(argv[3]), n = stoi(argv[4]), i = stoi(argv[5]);
        cout << ", m = " << m << ", k = " << k << ", n = " << n << endl;

        profileMatrixMultSet(m, k, n, i);
    } else if (strcmp(argv[1], "merge_sort") == 0) {
        int n = stoi(argv[2]), i = stoi(argv[3]);
        cout << ", n = " << n << ", i = " << i << endl;

        profileMergeSortSet(n, i);
    } else if (strcmp(argv[1], "bfs") == 0) {
        string filename = argv[2];
        int i = stoi(argv[3]);
        cout << ", file: " << filename << ", i = " << i << endl;
        profileBFS(filename, i);
    }

    cout << "dirty compiler hack, don't look at this..." << compilerHack << endl;

    return 0;
}