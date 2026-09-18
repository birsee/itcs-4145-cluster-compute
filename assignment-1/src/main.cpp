#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <ctime>

using namespace std;

// this is like the worst possible way to do this, but it's 3 AM
// and i really don't want to deal with header files right now.
// Sorry...

template <typename Func>
void time_function(Func&& func) {
    // struct timespec start, end;
    // clock_gettime(CLOCK_MONOTONIC, &start);
    func();
    // clock_gettime(CLOCK_MONOTONIC, &end);

    // double ms = (end.tv_sec - start.tv_sec) + 
    //             (end.tv_nsec - start.tv_nsec) / 1000000.0;
    // cout << "Execution time: " << ms << " ms\n";
}

// part 1
int max(const vector<int>& a) { 
    int max = a[0];
    for (size_t i = 1; i < a.size(); i++) {
        if (a[i] > max) max = a[i];
    }
    return max;
}
double max(const vector<double>& a) { 
    double max = a[0];
    for (size_t i = 1; i < a.size(); i++) {
        if (a[i] > max) max = a[i];
    }
    return max;
}
int maxBranchless(const vector<int>& a) { 
    int max = a[0];
    for (size_t i = 1; i < a.size(); i++) {
        max = (a[i] > max) ? a[i] : max;
    }
    return max;
}
double maxBranchless(const vector<double>& a) { 
    double max = a[0];
    for (size_t i = 1; i < a.size(); i++) {
        max = (a[i] > max) ? a[i] : max;
    }
    return max;
}

// part 2
void prefixSum(vector<int>& a) { 
    int sum = 0;
    for (size_t i = 0; i < a.size(); i++) {
        a[i] = sum;
        sum += a[i];
    }
}
void prefixSum(vector<double>& a) { 
    double sum = 0;
    for (size_t i = 0; i < a.size(); i++) {
        a[i] = sum;
        sum += a[i];
    }
}

// part 3
void multiplyMatrixIJK(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int i = 0; i < size; ++i)
    for (int j = 0; j < size; ++j)
    for (int k = 0; k < size; ++k)
            result[i * size + j] += a[i * size + k] * b[k * j];
}
void multiplyMatrixIKJ(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int i = 0; i < size; ++i)
    for (int k = 0; k < size; ++k)
    for (int j = 0; j < size; ++j)
            result[i * size + j] += a[i * size + k] * b[k * j];
}
void multiplyMatrixJIK(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int j = 0; j < size; ++j)
    for (int i = 0; i < size; ++i)
    for (int k = 0; k < size; ++k)
            result[i * size + j] += a[i * size + k] * b[k * j];
}
void multiplyMatrixJKI(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int j = 0; j < size; ++j)
    for (int k = 0; k < size; ++k)
    for (int i = 0; i < size; ++i)
            result[i * size + j] += a[i * size + k] * b[k * j];
}
void multiplyMatrixKIJ(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int k = 0; k < size; ++k)
    for (int i = 0; i < size; ++i)
    for (int j = 0; j < size; ++j)
            result[i * size + j] += a[i * size + k] * b[k * j];
}
void multiplyMatrixKJI(const vector<double>& a, const vector<double>& b, vector<double>& result, int size) {
    for (int k = 0; k < size; ++k)
    for (int j = 0; j < size; ++j)
    for (int i = 0; i < size; ++i)
            result[i * size + j] += a[i * size + k] * b[k * j];
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
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    mergeSortReuse(arr, temp, left, mid);
    mergeSortReuse(arr, temp, mid + 1, right);
    merge(arr, temp, left, mid, right);
}
void mergeSortAllocate(vector<int>& arr, int left, int right) {
    if (left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    mergeSortAllocate(arr, left, mid);
    mergeSortAllocate(arr, mid + 1, right);
    vector<int> temp; // allocate a new temp array every time :-(
    merge(arr, temp, left, mid, right);
}

// part 5
struct CSRGraph {
    int vertexCount;
    int edgeCount;
    vector<int> vertexToAdjacenceniesStart;
    vector<int> adjacencies;
};
CSRGraph loadFromMTX(const string& filename) {
    CSRGraph graph = {0, 0, {}, {}};

    ifstream file(filename);

    while (file.peek() == '%') file.ignore(2048, '\n');

    int numRows = 0, numCols = 0, numLines = 0;
    file >> numRows >> numCols >> numLines;

    graph.vertexCount = numRows;

    struct Edge { int src; int dest; };
    vector<Edge> tempEdges;
    tempEdges.reserve(numLines * 2); 

    for (int i = 0; i < numLines; ++i) {
        int u = 0, v = 0;
        if (!(file >> u >> v)) break;

        int srcNode = u - 1;
        int destNode = v - 1;

        tempEdges.push_back({srcNode, destNode});
        if (srcNode != destNode) tempEdges.push_back({destNode, srcNode});
    }
    file.close();

    graph.edgeCount = tempEdges.size();

    sort(tempEdges.begin(), tempEdges.end(), [](const Edge& a, const Edge& b) {
        if (a.src != b.src) return a.src < b.src;
        return a.dest < b.dest;
    });

    graph.vertexToAdjacenceniesStart.assign(graph.vertexCount + 1, 0);
    graph.adjacencies.resize(graph.edgeCount);

    for (const Edge& edge : tempEdges) graph.vertexToAdjacenceniesStart[edge.src + 1]++;
    for (int i = 1; i <= graph.vertexCount; ++i) graph.vertexToAdjacenceniesStart[i] += graph.vertexToAdjacenceniesStart[i - 1];
    for (size_t i = 0; i < tempEdges.size(); ++i) graph.adjacencies[i] = tempEdges[i].dest;

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

// the actual program
int main() {
    iostream::sync_with_stdio(false);
    cout << "Hello, World!" << endl;
    return 0;
}