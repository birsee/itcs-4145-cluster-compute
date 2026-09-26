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

#include "csv.h"
#include "profile.h"

struct CSRGraph {
    long vertexCount;
    long edgeCount;
    std::vector<long> vertexToAdjacenceniesStart;
    std::vector<long> adjacencies;
};
// lots of google-fu here. i barely understand this at all
CSRGraph loadFromMTX(const std::string& path) {
    std::ifstream file(path);

    if (!file) {
        std::cerr << "couldn't open " + path << std::endl;
        CSRGraph graph;
        graph.vertexCount = 0;
        graph.edgeCount = 0;
        return graph;
    }
 
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;
        break;
    }
    
    long numRows, numCols, numLines;
    {
        std::istringstream iss(line);
        iss >> numRows >> numCols >> numLines;
    }
 
    CSRGraph graph;
    graph.vertexCount = numRows;
 
    struct Edge { long src, dst; };
    std::vector<Edge> tempEdges;
    tempEdges.reserve(numLines * 2);
 
    for (long i = 0; i < numLines; i++) {
        long u, v;
        if (!(file >> u >> v)) break;
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    for (std::size_t i = 0; i < tempEdges.size(); i++) graph.adjacencies[i] = tempEdges[i].dst;
 
    return graph;
}

struct BFSResult {
    int totalLevels;
    int totalVerticesReached;
    int totalEdgesEvaluated;
    double fractionReached;

    std::vector<int> frontierSizes;
    std::vector<int> verticesReached;
    std::vector<int> edgesEvaluated;
};
BFSResult bfs(int startNode, CSRGraph& graph) {
    std::queue<int> frontier;
    std::vector<bool> visited(graph.vertexCount, false);

    frontier.push(startNode);
    visited[startNode] = true;

    BFSResult result;
    result.totalLevels = 0;
    result.totalVerticesReached = 0;
    result.totalEdgesEvaluated = 0;

    while (!frontier.empty()) {
        int currentFrontierSize = frontier.size();

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

        result.totalVerticesReached += frontier.size();
        result.frontierSizes.push_back(frontier.size());
        result.verticesReached.push_back(result.totalVerticesReached);
        result.edgesEvaluated.push_back(result.totalEdgesEvaluated);

        result.totalLevels++;
    }

    result.fractionReached = static_cast<double>(result.totalVerticesReached) / graph.vertexCount;

    return result;
}

void profileBFS(const std::string& csvPath, const std::string& csvPathSearch, const std::string& graphPath, int n, int i) {
    int compilerHack = 0;

    CSRGraph graph = loadFromMTX(graphPath);
    if (graph.edgeCount == 0) return;

    std::vector<int> startNodes;
    while(startNodes.size() < static_cast<size_t>(n)) {
        int v = rand() % graph.vertexCount;
        if (graph.vertexToAdjacenceniesStart[v + 1] > graph.vertexToAdjacenceniesStart[v])
            startNodes.push_back(v);
    }

    std::ofstream csvNode = csvStart(csvPath, "graph_type", "start_node", "edges_per_second", "edges_inspected", "percent_reached", "levels", "min_ns", "max_ns", "med_ns", "avg_ns", "total_ns");

    for (int startNode : startNodes) {
        BFSResult result;
        // even though this is run multiple times, the bfs is deterministic so the BFSResult should always be the same
        ProfilingResult r = profile(
            [](){}, 
            [&](){ 
                result = bfs(startNode, graph); 
                compilerHack += result.frontierSizes[0];
            }, i
        );

        std::ofstream csvSearch = csvStart(csvPathSearch + "/search_" + std::to_string(startNode) + ".csv", 
            "start_node", "level", "frontier_size", "edges_evaluated", "percent_edges_evaluated", "vertices_reached", "percent_vertices_reached");
        for (std::size_t i = 0; i < result.frontierSizes.size(); i++) {
            csvRow(csvSearch, startNode, i, result.frontierSizes[i], 
                result.edgesEvaluated[i], static_cast<long double>(result.edgesEvaluated[i]) / static_cast<long double>(graph.edgeCount),
                result.verticesReached[i], static_cast<long double>(result.verticesReached[i]) / static_cast<long double>(graph.vertexCount));
        }
        csvEnd(csvSearch);

        double edgesPerSec = (result.totalEdgesEvaluated * i) / nsToSec(r.totalNS);
        csvRow(csvNode, graphPath, startNode, edgesPerSec, result.totalEdgesEvaluated, result.fractionReached, result.totalLevels, 
            r.minNS, r.maxNS, r.medNS, r.avgNS, r.totalNS);
    }
    csvEnd(csvNode);
    // the per-graph results can be easily derived from the per-node results

    std::cout << compilerHack << std::endl;
}