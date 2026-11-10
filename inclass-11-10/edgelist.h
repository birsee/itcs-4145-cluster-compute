/* edgelist.h: generates the edge list for the degree-sum exercise.
 * ITCS 4145 Parallel Programming, Fall 2026.
 *
 *   edge_t *generate_edges(int n, long m, uint64_t seed);
 *
 * Returns an array of m edges (u, v) of an undirected graph on the vertices
 * 0, ..., n-1. Each edge is a pair of vertices drawn uniformly at random with
 * u != v (no self-loops). A pair may occur more than once; each occurrence
 * counts as a separate edge. The generator is xorshift64, so the same
 * (n, m, seed) gives the same edge list on every machine and in every run.
 * The caller frees the array with free(). Returns NULL if n < 2, m < 1, or
 * memory is exhausted.
 *
 * Do not modify this file: every program must see the same graph.
 */
#ifndef EDGELIST_H
#define EDGELIST_H

#include <stdint.h>
#include <stdlib.h>

typedef struct { int u, v; } edge_t;

#define EDGELIST_DEFAULT_SEED 88172645463325252ULL

/* xorshift64; the state must never be zero */
static inline uint64_t edgelist_next(uint64_t *state) {
  uint64_t x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  return *state = x;
}

static inline edge_t *generate_edges(int n, long m, uint64_t seed) {
  if (n < 2 || m < 1) return NULL;
  edge_t *edge = malloc((size_t)m * sizeof *edge);
  if (edge == NULL) return NULL;
  uint64_t state = seed ? seed : EDGELIST_DEFAULT_SEED;
  for (long e = 0; e < m; e++) {
    int u, v;
    do {
      u = (int)(edgelist_next(&state) % (uint64_t)n);
      v = (int)(edgelist_next(&state) % (uint64_t)n);
    } while (u == v);
    edge[e].u = u;
    edge[e].v = v;
  }
  return edge;
}

#endif /* EDGELIST_H */
