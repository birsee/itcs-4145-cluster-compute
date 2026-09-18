#define WINDOWS

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

#ifdef WINDOWS
    #include <chrono>
#else
    #include <ctime>
#endif

using namespace std;

// this is like the worst possible way to do this, but it's 3 AM
// and i really don't want to deal with header files right now.
// Sorry...

// reused helper stuffs
template <typename Setup, typename Kernel> double time(Setup&& setup, Kernel&& kernel) {
    setup();

    #ifdef WINDOWS
        auto start = chrono::steady_clock::now();
    #else
        // we're required to use clock_gettime i guess???
        // doesnt work on my machine so i'm putting it here...
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
    #endif
    
    kernel();

    #ifdef WINDOWS
        auto end = chrono::steady_clock::now();
        auto elapsed = end - start;
        double ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    #else
        clock_gettime(CLOCK_MONOTONIC, &end);
        double ms = (end.tv_nsec - start.tv_nsec) / 1000000.0;
    #endif

    return ms;
}

struct ProfilingResult {
    double min;
    double max;
    double med;
    double avg;
    double total;

    friend std::ostream& operator<<(std::ostream& os, const ProfilingResult& r) {
        return os << "MIN: " << r.min << endl 
                  << "MIN: " << r.max << endl 
                  << "AVG: " << r.avg << endl 
                  << "MED: " << r.med << endl 
                  << "TOTAL: " << r.total << endl;
    }
};
template <typename Setup, typename Kernel> ProfilingResult profile(Setup&& setup, Kernel&& kernel, int iterations) {
    // run once as warm-up
    kernel();
    
    double avg = 0, total = 0, min = numeric_limits<double>::max(), max = numeric_limits<double>::min();
    vector<double> runTimes(iterations);
    for (int i = 0; i < iterations; i++) {
        double ms = time(setup, kernel);
        avg += ms / iterations;
        total += ms;
        min = (ms < min) ? ms : min;
        max = (ms > max) ? ms : max;
        runTimes[i] = ms;
    }

    // find the median
    double med;
    size_t n = runTimes.size();
    auto mid_it = runTimes.begin() + n / 2;    
    std::nth_element(runTimes.begin(), mid_it, runTimes.end());
    if (n % 2 != 0) {
        med = static_cast<double>(*mid_it);
    } else {
        auto left_mid_it = std::max_element(runTimes.begin(), mid_it);
        med = (*mid_it + *left_mid_it) / 2.0;
    }

    // create the results
    ProfilingResult results;
    results.min = min;
    results.max = max;
    results.avg = avg;
    results.med = med;
    results.total = total;
    return results;
}

enum class ArrayType { Same, Sorted, Reverse, Random };
constexpr string_view arrayTypeName(ArrayType type) {
    switch (type) {
        case ArrayType::Same: return "Same";
        case ArrayType::Sorted:  return "Sorted";
        case ArrayType::Reverse: return "Reverse";
        case ArrayType::Random: return "Random";
        default: return "Unknown";
    }
}
template <typename T> void fillArray(const vector<T>& a, ArrayType type) {
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
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(rand());
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
    cout << "Sorted: " << endl << profileArrayMax(n, i, true, false, ArrayType::Sorted) << endl;
    cout << "Sorted, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Sorted) << endl;

    cout << "Reverse-sorted: " << endl << profileArrayMax(n, i, true, false, ArrayType::Reverse) << endl;
    cout << "Reverse-sorted, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Reverse) << endl;

    cout << "Randomized: " << endl << profileArrayMax(n, i, true, false, ArrayType::Random) << endl;
    cout << "Randomized, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Random) << endl;

    // doubles
    cout << "--- Double ---" << endl;
    cout << "Sorted: " << endl << profileArrayMax(n, i, false, false, ArrayType::Sorted) << endl;
    cout << "Sorted, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Sorted) << endl;

    cout << "Reverse-sorted: " << endl << profileArrayMax(n, i, false, false, ArrayType::Reverse) << endl;
    cout << "Reverse-sorted, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Reverse) << endl;

    cout << "Randomized: " << endl << profileArrayMax(n, i, false, false, ArrayType::Random) << endl;
    cout << "Randomized, branchless: " << endl << profileArrayMax(n, i, true, true, ArrayType::Random) << endl;
}
void profileArrayMaxBandwidth(int i) {
    for (long long bytes = 2LL * 1024; bytes <= 256LL * 1024 * 1024; bytes *= 2) {
        long long n = bytes / static_cast<long long>(sizeof(double));
        
        ProfilingResult result = profileArrayMax(n, i, false, false, ArrayType::Random);
        double bandwidth = (static_cast<double>(n) * sizeof(double)) / result.avg;
        cout << n << " doubles:" << endl << "BANDWIDTH: " << bandwidth << endl << result << endl;
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
            [&](){ fillArray(a, ArrayType::Sorted); },
            [&](){ prefixSum(a); }, i
        );
        compilerHack += a.back();
    } else {
        vector<double> a(n);
        return profile(
            [&](){ fillArray(a, type); },
            [&](){ prefixSum(a); }, i
        );
        compilerHack += a.back();
    }
}
void profilePrefixSumSet(int n, int i) {
    cout << "Integer: " << endl << profilePrefixSum(n, i, true, ArrayType::Same) << endl;
    cout << "Doubles: " << endl << profilePrefixSum(n, i, false, ArrayType::Same) << endl;
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
void profileMatrixMult(int n, int i) {
    vector<double> a(n * n), b(n * n), c(n * n);
    fillArray(a, ArrayType::Random);
    fillArray(b, ArrayType::Random);

    cout << "IJK: " << endl << profile([](){}, [&](){ matrixMultIJK(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
    cout << "IKJ: " << endl << profile([](){}, [&](){ matrixMultIKJ(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
    cout << "JIK: " << endl << profile([](){}, [&](){ matrixMultJIK(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
    cout << "JKI: " << endl << profile([](){}, [&](){ matrixMultJKI(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
    cout << "KIJ: " << endl << profile([](){}, [&](){ matrixMultKIJ(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
    cout << "KJI: " << endl << profile([](){}, [&](){ matrixMultKJI(a, b, c, n, n, n); compilerHack += c.back(); }, i) << endl;
}

// part 4
void merge(vector<int>& arr, vector<int>& temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
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
// [0] = mergeSortAllocate results, [1] = mergeSortReuse results, [2] = std::sort
vector<ProfilingResult> profileMergeSort(int n, int i, ArrayType type) {
    vector<int> a(n);
    fillArray(a, type);
    vector<int> b(a);
    vector<int> c(a);

    ProfilingResult resultA = profile([](){}, [&](){
        mergeSortAllocate(a, 0, n - 1); compilerHack += c.back();
    }, i);
    ProfilingResult resultB = profile([](){}, [&](){
        vector<int> temp(b.size());
        mergeSortReuse(b, temp, 0, n - 1);
        compilerHack += c.back();
    }, i);
    ProfilingResult resultC = profile([](){}, [&](){
        sort(c.begin(), c.end());
        compilerHack += c.back();
    }, i);

    // verify it is sorted and contains all original elements...
    // std::sort sort is known to work, so we can compare against that.
    for (size_t i = 0; i < n; i++) {
        int aVal = a[i];
        int bVal = b[i];
        int correctVal = c[i];

        if (aVal != correctVal || bVal != correctVal) {
            cout << "VERIFICATION FAILED: SORTED ARRAY ISN'T SORTED PROPERLY!!" << endl;
            break;
        }
    }

    vector<ProfilingResult> results = {resultA, resultB, resultC};
    return results;
}

void profileMergeSortSet(int n, int i) {
    // todo
}


// part 5
struct CSRGraph {
    int vertexCount;
    int edgeCount;
    vector<int> vertexToAdjacenceniesStart;
    vector<int> adjacencies;
};
// lots of google-fu here. i barely understand this at all
CSRGraph loadFromMTX(const string& filename) {
    ifstream file(filename);
    if (!file) throw runtime_error("cannot open " + filename);
 
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;
        break;
    }
    
    long long numRows, numCols, numLines;
    {
        istringstream iss(line);
        iss >> numRows >> numCols >> numLines;
    }
 
    CSRGraph graph;
    graph.vertexCount = numRows;
 
    struct Edge { int src, dst; };
    vector<Edge> tempEdges;
    tempEdges.reserve(numLines * 2);
 
    for (long long i = 0; i < numLines; i++) {
        long long u, v;
        if (!(file >> u >> v)) break;
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        int srcNode = static_cast<int>(u - 1);
        int destNode = static_cast<int>(v - 1);
        tempEdges.push_back({srcNode, destNode});
        if (srcNode != destNode) tempEdges.push_back({destNode, srcNode});
    }
    file.close();
 
    sort(tempEdges.begin(), tempEdges.end(), [](const Edge& a, const Edge& b) {
        if (a.src != b.src) return a.src < b.src;
        return a.dst < b.dst;
    });
 
    graph.edgeCount = static_cast<long long>(tempEdges.size());
    graph.vertexToAdjacenceniesStart.assign(graph.vertexCount + 1, 0);
    graph.adjacencies.resize(tempEdges.size());
 
    for (const Edge& e : tempEdges) graph.vertexToAdjacenceniesStart[e.src + 1]++;
    for (long long i = 1; i <= graph.vertexCount; i++) graph.vertexToAdjacenceniesStart[i] += graph.vertexToAdjacenceniesStart[i - 1];
    for (size_t i = 0; i < tempEdges.size(); i++) graph.adjacencies[i] = tempEdges[i].dst;
 
    return graph;
}

struct BFSResult {
    int totalLevels;
    int totalVerticesReached;
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
void profileBFS(int n, int i) {
    // todo
}

// the actual program
int main(int argc, char* argv[]) {
    iostream::sync_with_stdio(false);
    cout << "Starting profiling..." << endl;
    cout << "Profiling operation " << argv[1] << " with n = " << argv[2] << " and i = " << argv[3] << endl;

    int n = stoi(argv[2]);
    int i = stoi(argv[3]);
    if (strcmp(argv[1], "array_max") == 0) {
        cout << "Array Max:" << endl;
        profileArrayMaxSet(n, i);

        cout << "Bandwidth" << endl;
        profileArrayMaxBandwidth(i);
    }

    cout << "dirty compiler hack, don't look at this..." << compilerHack << endl;

    return 0;
}