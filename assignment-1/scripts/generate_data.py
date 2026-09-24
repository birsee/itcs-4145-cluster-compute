import networkx as nx
from pathlib import Path

def write_mtx(G, filename):
    edges = list(G.edges())

    with open(filename, "w") as f:
        f.write("%%MatrixMarket matrix coordinate pattern symmetric\n")

        f.write(
            f"{G.number_of_nodes()} "
            f"{G.number_of_nodes()} "
            f"{len(edges)}\n"
        )

        for u, v in edges:
            f.write(f"{u+1} {v+1}\n")


def generate_graphs():
    n = 2**20
    avg_degree = 16

    output = Path(__file__).resolve().parent.parent

    print("generating ER graph...")
    p = avg_degree / (n - 1)
    G_er = nx.fast_gnp_random_graph(
        n, p, directed=False
    )
    print("writing ER graph...")
    write_mtx(
        G_er,
        output / "graph_er.mtx"
    )

    print("generating RMAT graph...")
    G_rmat = nx.barabasi_albert_graph(
        n, avg_degree // 2
    )
    print("writing RMAT graph...")
    write_mtx(
        G_rmat,
        output / "graph_rmat.mtx"
    )

if __name__ == "__main__":
    generate_graphs()