// degree.cpp: the degree sum with locks and with atomics (C++).
// ITCS 4145 Parallel Programming, Fall 2026: in-class exercise.
//
// Each function receives the edge list of an undirected graph on n vertices
// and a vector deg of n entries that is ZERO on entry. It must
//   1. set deg[v] to the number of edges that have v as an end, and
//   2. return the degree sum deg[0] + ... + deg[n-1], which equals 2m.
//
// degree_seq is complete. Complete degree_lock and degree_atomic:
//   - parallelize the edge loop with omp parallel for, and protect each
//     update of deg with the synchronization named in the title;
//   - parallelize the sum loop with a reduction.
// Do not modify anything below the line "Driver and verifier".
//
// Build:  make degree_cpp
// Run:    ./degree_cpp 1    instance 1: n = 2^20 vertices, m = 2^23 edges
//         ./degree_cpp 2    instance 2: n = 2^10 vertices, m = 2^23 edges
// The driver times degree_seq and both parallel versions with 2, 4, 8 and
// 32 threads, checks every result against degree_seq, and prints the best
// of three runs. Copy those numbers onto the worksheet.
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <omp.h>
#include "edgelist.hpp"

using degrees = std::vector<long>;

// ---------------------------------------------------------------------------
// Sequential: the reference. Complete; do not modify.
// ---------------------------------------------------------------------------
long degree_seq(int n, const std::vector<edge_t> &edge, degrees &deg) {
  long m = static_cast<long>(edge.size());
  for (long e = 0; e < m; e++) {   // each edge once
    deg[edge[e].u]++;              // one end
    deg[edge[e].v]++;              // the other end
  }
  long sum = 0;
  for (int v = 0; v < n; v++)      // each vertex once
    sum += deg[v];
  return sum;
}

// ---------------------------------------------------------------------------
// Version 1: one lock per vertex (omp_lock_t).
// Create and initialize the n locks here; destroy them before returning.
// ---------------------------------------------------------------------------
long degree_lock(int n, const std::vector<edge_t> &edge, degrees &deg) {
  long m = static_cast<long>(edge.size());
  long sum = 0;

  std::vector<omp_lock_t> locks(n);

  // create the locks
  #pragma omp parallel for
  for (int i = 0; i < n; i++)
    omp_init_lock(&locks[i]);
  

  // run da function
  for (long e = 0; e < m; e++) {
    int i0 = edge[e].u;
    int i1 = edge[e].v;

    omp_set_lock(&locks[i0]);
    deg[i0]++;
    omp_unset_lock(&locks[i0]);

    omp_set_lock(&locks[i1]);
    deg[i1]++;
    omp_unset_lock(&locks[i1]);
  }

  // destroy the locks
  #pragma omp parallel for
  for (int i = 0; i < n; i++)
    omp_destroy_lock(&locks[i]);

  #pragma omp parallel for reduction(+:sum)
  for (int v = 0; v < n; v++)
    sum += deg[v];

  return sum;
}

// ---------------------------------------------------------------------------
// Version 2: atomic updates (#pragma omp atomic).
// ---------------------------------------------------------------------------
long degree_atomic(int n, const std::vector<edge_t> &edge, degrees &deg) {
  long m = static_cast<long>(edge.size());
  long sum = 0;

  #pragma omp parallel for
  for (long e = 0; e < m; e++) {
    #pragma omp atomic
    deg[edge[e].u]++;
    #pragma omp atomic
    deg[edge[e].v]++;
  }

  #pragma omp parallel for reduction(+:sum)
  for (int v = 0; v < n; v++)
    sum += deg[v];

  return sum;
}

// ===========================================================================
// Driver and verifier: do not modify below this line.
// ===========================================================================
constexpr int REPS = 3;
using degree_fn = long (*)(int, const std::vector<edge_t> &, degrees &);

static bool report_wrong = true;   // print the details of the first error only

// true if deg equals the reference ref and sum equals 2m
static bool verify(const std::string &name, int p, long m, const degrees &ref,
                   const degrees &deg, long sum) {
  long bad = 0;
  int first = -1;
  for (int v = 0; v < static_cast<int>(ref.size()); v++)
    if (deg[v] != ref[v]) { if (first < 0) first = v; bad++; }
  if (bad == 0 && sum == 2 * m) return true;
  if (report_wrong) {
    std::printf("  ! %s, %d threads:", name.c_str(), p);
    if (bad > 0)
      std::printf(" %ld of %zu degrees differ (first: deg[%d] = %ld, expected %ld);",
                  bad, ref.size(), first, deg[first], ref[first]);
    if (sum != 2 * m)
      std::printf(" returned sum %ld, expected 2m = %ld", sum, 2 * m);
    std::printf("\n");
    report_wrong = false;
  }
  return false;
}

// best of REPS runs of fn with p threads; -1 if any run is wrong
static double best_time(degree_fn fn, const std::string &name, int p, int n,
                        const std::vector<edge_t> &edge, const degrees &ref,
                        degrees &deg) {
  long m = static_cast<long>(edge.size());
  double best = 1e30;
  omp_set_num_threads(p);
  for (int r = 0; r < REPS; r++) {
    std::fill(deg.begin(), deg.end(), 0);      // zero on entry
    double t = omp_get_wtime();
    long sum = fn(n, edge, deg);
    t = omp_get_wtime() - t;
    if (!verify(name, p, m, ref, deg, sum)) return -1;
    best = std::min(best, t);
  }
  return best;
}

static void print_cell(double t) {
  if (t < 0) std::printf("  %10s", "WRONG");
  else       std::printf("  %10.4f", t);
}

int main(int argc, char **argv) {
  int instance = argc > 1 ? std::atoi(argv[1]) : 0;
  int n;
  long m = 1L << 23;
  if      (instance == 1) n = 1 << 20;
  else if (instance == 2) n = 1 << 10;
  else { std::fprintf(stderr, "usage: %s 1|2\n", argv[0]); return 1; }

  std::vector<edge_t> edge = generate_edges(n, m, EDGELIST_DEFAULT_SEED);
  degrees ref(n), deg(n);

  std::printf("Instance %d: n = %d vertices, m = %ld edges\n", instance, n, m);
  std::printf("Times in seconds, best of %d runs.\n\n", REPS);

  // sequential: reference degrees and time
  double tseq = 1e30;
  for (int r = 0; r < REPS; r++) {
    std::fill(ref.begin(), ref.end(), 0);
    double t = omp_get_wtime();
    long sum = degree_seq(n, edge, ref);
    t = omp_get_wtime() - t;
    if (sum != 2 * m) { std::printf("degree_seq: sum %ld != 2m\n", sum); return 1; }
    tseq = std::min(tseq, t);
  }
  std::printf("  sequential  %10.4f\n\n", tseq);

  const int threads[] = {2, 4, 8, 32};
  int failed = 0;
  std::printf("  threads     %10s  %10s\n", "lock", "atomic");
  for (int p : threads) {
    double tl = best_time(degree_lock,   "lock",   p, n, edge, ref, deg);
    double ta = best_time(degree_atomic, "atomic", p, n, edge, ref, deg);
    failed += (tl < 0) + (ta < 0);
    std::printf("  %7d   ", p);
    print_cell(tl);
    print_cell(ta);
    std::printf("\n");
  }
  std::printf("\n%s\n", failed ? "Some results are WRONG: fix them before reporting."
                               : "All results are correct.");
  return failed ? 1 : 0;
}
