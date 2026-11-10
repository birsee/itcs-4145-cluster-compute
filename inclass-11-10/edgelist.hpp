// edgelist.hpp: generates the edge list for the degree-sum exercise (C++).
// ITCS 4145 Parallel Programming, Fall 2026.
//
//   std::vector<edge_t> generate_edges(int n, long m, uint64_t seed);
//
// Returns m edges (u, v) of an undirected graph on the vertices 0, ..., n-1.
// Each edge is a pair of vertices drawn uniformly at random with u != v (no
// self-loops). A pair may occur more than once; each occurrence counts as a
// separate edge. The generator is xorshift64, the same as in edgelist.h, so
// the C and C++ programs see the same graph for the same (n, m, seed).
// Throws std::invalid_argument if n < 2 or m < 1.
//
// Do not modify this file: every program must see the same graph.
#ifndef EDGELIST_HPP
#define EDGELIST_HPP

#include <cstdint>
#include <stdexcept>
#include <vector>

struct edge_t { int u, v; };

constexpr uint64_t EDGELIST_DEFAULT_SEED = 88172645463325252ULL;

// xorshift64; the state must never be zero
inline uint64_t edgelist_next(uint64_t &state) {
  state ^= state << 13;
  state ^= state >> 7;
  state ^= state << 17;
  return state;
}

inline std::vector<edge_t> generate_edges(int n, long m,
                                          uint64_t seed = EDGELIST_DEFAULT_SEED) {
  if (n < 2 || m < 1) throw std::invalid_argument("generate_edges: need n >= 2, m >= 1");
  std::vector<edge_t> edge(m);
  uint64_t state = seed ? seed : EDGELIST_DEFAULT_SEED;
  for (long e = 0; e < m; e++) {
    int u, v;
    do {
      u = static_cast<int>(edgelist_next(state) % static_cast<uint64_t>(n));
      v = static_cast<int>(edgelist_next(state) % static_cast<uint64_t>(n));
    } while (u == v);
    edge[e] = {u, v};
  }
  return edge;
}

#endif // EDGELIST_HPP
