import networkx as nx
import numpy as np
from pathlib import Path

def write_mtx(num_nodes, edges, filename):
    with open(filename, "w") as f:
        f.write("%%MatrixMarket matrix coordinate pattern symmetric\n")
        f.write(f"{num_nodes} {num_nodes} {len(edges)}\n")
        np.savetxt(f, edges + 1, fmt="%d %d")

def write_mtx_from_graph(G, filename):
    edges = np.array(list(G.edges()), dtype=np.int64)
    write_mtx(G.number_of_nodes(), edges, filename)

# translated basically 1 : 1 from https://github.com/microsoft/GraphEngine/wiki/GraphGenerator
def generate_rmat_edges(n, num_edges, a=0.57, b=0.19, c=0.19, d=0.05, rng=None):
    if rng is None:
        rng = np.random.default_rng()

    scale = int(np.log2(n))
    if 2 ** scale != n:
        raise ValueError(f"rmat requires n to be a power of 2")

    src = np.zeros(num_edges, dtype=np.int64)
    dst = np.zeros(num_edges, dtype=np.int64)

    ab = a + b
    c_norm = c / (1.0 - ab) if ab < 1.0 else 0.0

    for _ in range(scale):
        noise_a = a * (0.1 * rng.random(num_edges) - 0.05)
        noise_b = b * (0.1 * rng.random(num_edges) - 0.05)
        a_lvl = a + noise_a
        b_lvl = b + noise_b
        ab_lvl = a_lvl + b_lvl

        top = rng.random(num_edges) > ab_lvl
        left = rng.random(num_edges) > np.where(top, c_norm, a_lvl / np.where(ab_lvl == 0, 1, ab_lvl))

        src_bit = (~left).astype(np.int64)
        dst_bit = top.astype(np.int64)
        src = (src << 1) | src_bit
        dst = (dst << 1) | dst_bit

    perm = rng.permutation(n)
    src = perm[src]
    dst = perm[dst]

    return np.stack([src, dst], axis=1)

def generate_graphs():
    n = 2 ** 20
    avg_degree = 16
    num_edges = n * avg_degree // 2

    output = Path(__file__).resolve().parent.parent
    rng = np.random.default_rng(42)

    print("generating ER graph...")
    p = avg_degree / (n - 1)
    G_er = nx.fast_gnp_random_graph(n, p, directed=False, seed=int(rng.integers(2**31)))
    print("writing ER graph...")
    write_mtx_from_graph(G_er, output / "graph_er.mtx")

    print("generating RMAT graph...")
    rmat_edges = generate_rmat_edges(n, num_edges, rng=rng)
    print("writing RMAT graph...")
    write_mtx(n, rmat_edges, output / "graph_rmat.mtx")


if __name__ == "__main__":
    generate_graphs()