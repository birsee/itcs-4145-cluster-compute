import networkx as nx
import sys

def generate_graphs():
    n = 2**20  
    avg_degree = 16
    
    print(f"Generating Erdos-Renyi graph")
    p = avg_degree / (n - 1)
    G_er = nx.fast_gnp_random_graph(n, p, directed=False)
    print("Writing erdos_renyi.mtx...")
    nx.write_edgelist(G_er, "erdos_renyi.mtx", data=False)
    
    print(f"Generating RMAT graph")
    m = avg_degree // 2
    G_rmat = nx.barabasi_albert_graph(n, m)
    print("Writing rmat.mtx...")
    nx.write_edgelist(G_rmat, "rmat.mtx", data=False)
    
    for filename in ["erdos_renyi.mtx", "rmat.mtx"]:
        with open(filename, 'r') as f:
            content = f.read()
        with open(filename, 'w') as f:
            f.write(f"%%MatrixMarket matrix coordinate pattern symmetric\n")
            num_nodes = n
            num_edges = len(content.splitlines())
            f.write(f"{num_nodes} {num_nodes} {num_edges}\n")
            f.write(content)
            
    print("Graph generation complete.")

if __name__ == "__main__":
    generate_graphs()