import re
import csv
import sys
from pathlib import Path

N_VALUES_BY_FILE = {
    "array_max.txt": [1_000_000, 10_000_000, 100_000_000],
    "prefix_sum_O0.txt": [1_000_000, 10_000_000, 100_000_000],
    "prefix_sum_O2.txt": [1_000_000, 10_000_000, 100_000_000],
    "prefix_sum_O3.txt": [1_000_000, 10_000_000, 100_000_000],
}

PREFIX_SUM_OPT_RE = re.compile(r"prefix_sum_(O\d)\.txt$")

RUN_SPLIT_RE = re.compile(r"Starting profiling\.\.\.\n")

STAT_BLOCK = (
    r"MIN: (?P<min_ns>\d+) ns.*\n"
    r"MAX: (?P<max_ns>\d+) ns.*\n"
    r"AVG: (?P<avg_ns>\d+) ns.*\n"
    r"MED: (?P<med_ns>\d+) ns.*\n"
    r"TOTAL: (?P<total_ns>\d+) ns.*"
)

ARRAY_MAX_SUBSECTION_RE = re.compile(
    r"^(?P<label>[A-Za-z][\w, -]*):\s*\n" + STAT_BLOCK,
    re.MULTILINE,
)

BANDWIDTH_RE = re.compile(
    r"^(?P<count>\d+) doubles:\n"
    r"BANDWIDTH: (?P<bandwidth>[\d.eE+-]+)\n" + STAT_BLOCK,
    re.MULTILINE,
)

PREFIX_SUM_SECTION_RE = re.compile(
    r"^(?P<label>Integer|Doubles):\s*\n" + STAT_BLOCK + r"\n\s*RATE: (?P<rate>[\d.eE+-]+)",
    re.MULTILINE,
)

PREFIX_SUM_HEADER_RE = re.compile(r"Profiling operation prefix_sum,\s*n\s*=\s*(\d+)")

MATMUL_HEADER_RE = re.compile(
    r"Profiling operation matmul,\s*m\s*=\s*(\d+),\s*n\s*=\s*(\d+),\s*k\s*=\s*(\d+)"
)

MATMUL_ORDER_RE = re.compile(
    r"^(?P<order>IJK|IKJ|JIK|JKI|KIJ|KJI): MIN: (?P<min_ns>\d+) ns.*\n"
    r"MAX: (?P<max_ns>\d+) ns.*\n"
    r"AVG: (?P<avg_ns>\d+) ns.*\n"
    r"MED: (?P<med_ns>\d+) ns.*\n"
    r"TOTAL: (?P<total_ns>\d+) ns.*\n"
    r"\n?FLOPS: (?P<flops>[\d.eE+-]+)",
    re.MULTILINE,
)

BFS_TYPE_HEADER_RE = re.compile(r"Profiling operation bfs(?:,\s*type\s*=\s*(\S+))?")

BFS_NODE_RE = re.compile(
    r"Results for starting from node (?P<node>\d+)\n"
    r"LEVELS: (?P<levels>\d+)\n"
    r"% REACHED: (?P<frac>[\d.eE+-]+)\n"
    r"EDGES INSPECTED: (?P<edges>\d+)\n"
    r"TEPS: (?P<teps>[\d.eE+-]+)"
)

BFS_TOTALS_RE = re.compile(
    r"Totals:\nMIN: (?P<min_teps>[\d.eE+-]+)\nMEDIAN: (?P<med_teps>[\d.eE+-]+)\nMAX: (?P<max_teps>[\d.eE+-]+)"
)

MERGE_SORT_HEADER_RE = re.compile(r"Profiling operation merge_sort,\s*n\s*=\s*(\d+)")

MERGE_SORT_ARRAY_TYPE_RE = re.compile(r"^(?P<label>Same|Sorted|Reverse|Random):\s*$", re.MULTILINE)

MERGE_SORT_METHOD_RE = re.compile(
    r"^(?P<method>std::sort|Reuse|Allocate):\s*\n"
    r"MIN: (?P<min_ns>\d+) ns.*\n"
    r"MAX: (?P<max_ns>\d+) ns.*\n"
    r"AVG: (?P<avg_ns>\d+) ns.*\n"
    r"MED: (?P<med_ns>\d+) ns.*\n"
    r"TOTAL: (?P<total_ns>\d+) ns.*\n"
    r"RATE: (?P<rate>[\d.eE+-]+)",
    re.MULTILINE,
)

EXPECTED_NS = {1_000_000, 10_000_000, 100_000_000}

def split_runs(text):
    parts = RUN_SPLIT_RE.split(text)
    return [p for p in parts if p.strip()]


def parse_array_max(filename, text, n_values):
    runs = split_runs(text)

    timing_rows = []
    bandwidth_rows = []

    for n, run_text in zip(n_values, runs):
        bw_idx = run_text.find("\nBandwidth\n")
        main_part = run_text if bw_idx == -1 else run_text[:bw_idx]
        bw_part = "" if bw_idx == -1 else run_text[bw_idx:]

        int_idx = main_part.find("--- Integer ---")
        dbl_idx = main_part.find("--- Double ---")
        int_block = main_part[int_idx:dbl_idx] if int_idx != -1 else ""
        dbl_block = main_part[dbl_idx:] if dbl_idx != -1 else ""

        for dtype, block in (("int", int_block), ("double", dbl_block)):
            for m in ARRAY_MAX_SUBSECTION_RE.finditer(block):
                label = m.group("label").strip()
                branchless = "branchless" in label.lower()
                access_pattern = label.replace(", branchless", "").strip()
                timing_rows.append({
                    "n": n,
                    "dtype": dtype,
                    "access_pattern": access_pattern,
                    "branchless": branchless,
                    "min_ns": m.group("min_ns"),
                    "max_ns": m.group("max_ns"),
                    "avg_ns": m.group("avg_ns"),
                    "med_ns": m.group("med_ns"),
                    "total_ns": m.group("total_ns"),
                })

        for m in BANDWIDTH_RE.finditer(bw_part):
            count = int(m.group("count"))
            bandwidth_rows.append({
                "n_outer_loop": n,  # note: bandwidth sweep ignores this; see README note
                "doubles_count": count,
                "bytes": count * 8,
                "bandwidth_bytes_per_sec": m.group("bandwidth"),
                "min_ns": m.group("min_ns"),
                "max_ns": m.group("max_ns"),
                "avg_ns": m.group("avg_ns"),
                "med_ns": m.group("med_ns"),
                "total_ns": m.group("total_ns"),
            })

    return timing_rows, bandwidth_rows


def parse_prefix_sum(filename, text, n_values, opt_level):
    runs = split_runs(text)

    rows = []
    for idx, run_text in enumerate(runs):
        header_match = PREFIX_SUM_HEADER_RE.search(run_text)
        if header_match:
            n = int(header_match.group(1))
        elif idx < len(n_values):
            n = n_values[idx]
        else:
            continue

        for m in PREFIX_SUM_SECTION_RE.finditer(run_text):
            label = m.group("label")
            dtype = "int" if label == "Integer" else "double"
            rows.append({
                "n": n,
                "opt_level": opt_level,
                "dtype": dtype,
                "min_ns": m.group("min_ns"),
                "max_ns": m.group("max_ns"),
                "avg_ns": m.group("avg_ns"),
                "med_ns": m.group("med_ns"),
                "total_ns": m.group("total_ns"),
                "rate_elements_per_sec": m.group("rate"),
            })
    return rows


def parse_merge_sort(filename, text):
    runs = split_runs(text)
    rows = []
    for idx, run_text in enumerate(runs):
        header_match = MERGE_SORT_HEADER_RE.search(run_text)
        if not header_match:
            continue
        n = int(header_match.group(1))

        type_positions = [(m.start(), m.group("label")) for m in MERGE_SORT_ARRAY_TYPE_RE.finditer(run_text)]
        if not type_positions:
            continue

        for method_match in MERGE_SORT_METHOD_RE.finditer(run_text):
            pos = method_match.start()
            array_type = next((label for start, label in reversed(type_positions) if start <= pos), None)
            rows.append({
                "n": n,
                "array_type": array_type,
                "method": method_match.group("method"),
                "min_ns": method_match.group("min_ns"),
                "max_ns": method_match.group("max_ns"),
                "avg_ns": method_match.group("avg_ns"),
                "med_ns": method_match.group("med_ns"),
                "total_ns": method_match.group("total_ns"),
                "rate_elements_per_sec": method_match.group("rate"),
            })
    return rows


def parse_matmul(filename, text):
    runs = split_runs(text)
    rows = []
    for idx, run_text in enumerate(runs):
        header_match = MATMUL_HEADER_RE.search(run_text)
        if not header_match:
            continue
        m_dim, n_dim, k_dim = (int(x) for x in header_match.groups())

        for order_match in MATMUL_ORDER_RE.finditer(run_text):
            rows.append({
                "m": m_dim,
                "k": k_dim,
                "n": n_dim,
                "loop_order": order_match.group("order"),
                "min_ns": order_match.group("min_ns"),
                "max_ns": order_match.group("max_ns"),
                "avg_ns": order_match.group("avg_ns"),
                "med_ns": order_match.group("med_ns"),
                "total_ns": order_match.group("total_ns"),
                "flops_per_sec": order_match.group("flops"),
            })
    return rows


def parse_bfs(filename, text, graph_type_from_filename):
    runs = split_runs(text)
    per_node_rows = []
    summary_rows = []


    for idx, run_text in enumerate(runs):
        header_match = BFS_TYPE_HEADER_RE.search(run_text)
        header_type = header_match.group(1) if header_match and header_match.group(1) else None
        graph_type = header_type or graph_type_from_filename

        for node_match in BFS_NODE_RE.finditer(run_text):
            per_node_rows.append({
                "graph_type": graph_type,
                "run_index": idx,
                "start_node": node_match.group("node"),
                "levels": node_match.group("levels"),
                "fraction_reached": node_match.group("frac"),
                "edges_inspected": node_match.group("edges"),
                "teps": node_match.group("teps"),
            })

        totals_match = BFS_TOTALS_RE.search(run_text)
        if totals_match:
            summary_rows.append({
                "graph_type": graph_type,
                "run_index": idx,
                "min_teps": totals_match.group("min_teps"),
                "median_teps": totals_match.group("med_teps"),
                "max_teps": totals_match.group("max_teps"),
            })

    return per_node_rows, summary_rows


def write_csv(rows, path, fieldnames):
    if not rows:
        print(f"  (no rows to write for {path.name}, skipping)")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    print(f"  wrote {len(rows)} rows -> {path}")


def main():
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} <results_dir> <csv_output_dir>")
        sys.exit(1)

    results_dir = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])

    all_array_max_timing = []
    all_array_max_bandwidth = []
    all_prefix_sum = []
    all_matmul = []
    all_merge_sort = []
    all_bfs_per_node = []
    all_bfs_summary = []

    BFS_FILENAME_TYPES = {
        "bfs_er.txt": "ER",
        "bfs_rmat.txt": "rmat",
    }

    for path in sorted(results_dir.glob("*.txt")):
        name = path.name
        text = path.read_text()

        if name == "array_max.txt":
            if name not in N_VALUES_BY_FILE:
                continue
            print(f"parsing {name} ...")
            timing, bandwidth = parse_array_max(name, text, N_VALUES_BY_FILE[name])
            all_array_max_timing.extend(timing)
            all_array_max_bandwidth.extend(bandwidth)

        elif PREFIX_SUM_OPT_RE.search(name):
            opt_level = PREFIX_SUM_OPT_RE.search(name).group(1)
            print(f"parsing {name} (opt={opt_level}) ...")
            rows = parse_prefix_sum(name, text, N_VALUES_BY_FILE.get(name, []), opt_level)
            all_prefix_sum.extend(rows)

        elif name.startswith("matmul"):
            print(f"parsing {name} ...")
            rows = parse_matmul(name, text)
            all_matmul.extend(rows)

        elif name.startswith("merge_sort"):
            print(f"parsing {name} ...")
            rows = parse_merge_sort(name, text)
            all_merge_sort.extend(rows)

        elif name.startswith("bfs_"):
            print(f"parsing {name} ...")
            graph_type = BFS_FILENAME_TYPES.get(name, "unknown")
            per_node, summary = parse_bfs(name, text, graph_type)
            all_bfs_per_node.extend(per_node)
            all_bfs_summary.extend(summary)
        else:
            print(f"skipping {name}: no parser for this file yet")

    write_csv(
        all_array_max_timing,
        output_dir / "array_max_timing.csv",
        ["n", "dtype", "access_pattern", "branchless", "min_ns", "max_ns", "avg_ns", "med_ns", "total_ns"],
    )
    write_csv(
        all_array_max_bandwidth,
        output_dir / "array_max_bandwidth.csv",
        ["n_outer_loop", "doubles_count", "bytes", "bandwidth_bytes_per_sec", "min_ns", "max_ns", "avg_ns", "med_ns", "total_ns"],
    )
    write_csv(
        all_prefix_sum,
        output_dir / "prefix_sum.csv",
        ["n", "opt_level", "dtype", "min_ns", "max_ns", "avg_ns", "med_ns", "total_ns", "rate_elements_per_sec"],
    )
    write_csv(
        all_matmul,
        output_dir / "matmul.csv",
        ["m", "k", "n", "loop_order", "min_ns", "max_ns", "avg_ns", "med_ns", "total_ns", "flops_per_sec"],
    )
    write_csv(
        all_merge_sort,
        output_dir / "merge_sort.csv",
        ["n", "array_type", "method", "min_ns", "max_ns", "avg_ns", "med_ns", "total_ns", "rate_elements_per_sec"],
    )
    write_csv(
        all_bfs_per_node,
        output_dir / "bfs_per_node.csv",
        ["graph_type", "run_index", "start_node", "levels", "fraction_reached", "edges_inspected", "teps"],
    )
    write_csv(
        all_bfs_summary,
        output_dir / "bfs_summary.csv",
        ["graph_type", "run_index", "min_teps", "median_teps", "max_teps"],
    )


if __name__ == "__main__":
    main()
