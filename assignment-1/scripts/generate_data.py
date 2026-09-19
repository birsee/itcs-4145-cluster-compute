import os
import re
import csv
import shutil

def parse_array_max(filepath):
    # Extracts profiling stats for array_max integer and double operations
    out_csv = filepath.replace('.txt', '.csv')
    with open(filepath, 'r') as f, open(out_csv, 'w', newline='') as out:
        content = f.read()
        writer = csv.writer(out)
        writer.writerow(['N', 'Data_Type', 'Configuration', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns', 'Bytes_Per_Sec'])
        
        # Split by profiling runs
        runs = re.split(r'Profiling operation array_max, n = (\d+),', content)[1:]
        for i in range(0, len(runs), 2):
            n_val = runs[i]
            run_data = runs[i+1]
            
            data_types = re.split(r'--- (Integer|Double) ---', run_data)[1:]
            for j in range(0, len(data_types), 2):
                dtype = data_types[j]
                dtype_data = data_types[j+1]
                
                # Match configuration blocks before the Bandwidth section
                configs = re.findall(r'([A-Za-z\-,\s]+):\s*\nMIN:\s*(\d+) ns.*?MAX:\s*(\d+) ns.*?AVG:\s*(\d+) ns.*?MED:\s*(\d+) ns.*?TOTAL.*?BYTES/S:\s*([-\d.e+]+)', dtype_data, re.DOTALL)
                
                for config in configs:
                    name, min_ns, max_ns, avg_ns, med_ns, bps = config
                    writer.writerow([n_val, dtype.strip(), name.strip(), min_ns, max_ns, avg_ns, med_ns, bps])

def parse_bfs(filepath):
    out_csv = filepath.replace('.txt', '.csv')
    with open(filepath, 'r') as f, open(out_csv, 'w', newline='') as out:
        content = f.read()
        writer = csv.writer(out)
        writer.writerow(['File', 'Start_Node', 'Levels', 'Percent_Reached', 'Edges_Inspected', 'TEPS'])
        
        graph_file = re.search(r'file: (.*?),', content).group(1).split('/')[-1]
        
        results = re.findall(r'Results for starting from node (\d+)\nLEVELS: (\d+)\n% REACHED: (\d+)\nEDGES INSPECTED: (\d+)\nTEPS: ([\d.e+]+)', content)
        for res in results:
            writer.writerow([graph_file, res[0], res[1], res[2], res[3], res[4]])

def parse_matmul(filepath):
    out_csv = filepath.replace('.txt', '.csv')
    with open(filepath, 'r') as f, open(out_csv, 'w', newline='') as out:
        content = f.read()
        writer = csv.writer(out)
        writer.writerow(['M', 'K', 'N', 'Loop_Order', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns', 'FLOPS'])
        
        runs = re.split(r'Profiling operation matmul, m = (\d+), k = (\d+), n = (\d+)', content)[1:]
        for i in range(0, len(runs), 4):
            m, k, n = runs[i], runs[i+1], runs[i+2]
            run_data = runs[i+3]
            
            loops = re.findall(r'(IJK|IKJ|JIK|JKI|KIJ|KJI):\s*MIN:\s*(\d+) ns.*?MAX:\s*(\d+) ns.*?AVG:\s*(\d+) ns.*?MED:\s*(\d+) ns.*?FLOPS:\s*([\d.e+]+)', run_data, re.DOTALL)
            for loop in loops:
                order, min_ns, max_ns, avg_ns, med_ns, flops = loop
                writer.writerow([m, k, n, order, min_ns, max_ns, avg_ns, med_ns, flops])

def parse_merge_sort(filepath):
    out_csv = filepath.replace('.txt', '.csv')
    with open(filepath, 'r') as f, open(out_csv, 'w', newline='') as out:
        content = f.read()
        writer = csv.writer(out)
        writer.writerow(['N', 'Data_Pattern', 'Algorithm', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns', 'Elements_Per_Sec'])
        
        runs = re.split(r'Profiling operation merge_sort, n = (\d+),', content)[1:]
        for i in range(0, len(runs), 2):
            n_val = runs[i]
            run_data = runs[i+1]
            
            patterns = re.split(r'(Same:|Sorted:|Reverse:|Random:)', run_data)[1:]
            for j in range(0, len(patterns), 2):
                pattern = patterns[j].replace(':', '').strip()
                pattern_data = patterns[j+1]
                
                algos = re.findall(r'(std::sort:|Reuse:)\s*MIN:\s*(\d+) ns.*?MAX:\s*(\d+) ns.*?AVG:\s*(\d+) ns.*?MED:\s*(\d+) ns.*?RATE:\s*([\d.e+]+)', pattern_data, re.DOTALL)
                for algo in algos:
                    algo_name, min_ns, max_ns, avg_ns, med_ns, rate = algo
                    writer.writerow([n_val, pattern, algo_name.replace(':', '').strip(), min_ns, max_ns, avg_ns, med_ns, rate])

def parse_prefix_sum(filepath):
    out_csv = filepath.replace('.txt', '.csv')
    with open(filepath, 'r') as f, open(out_csv, 'w', newline='') as out:
        content = f.read()
        writer = csv.writer(out)
        writer.writerow(['N', 'Optimization_Level', 'Data_Type', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns', 'Elements_Per_Sec'])
        
        opt_level = re.search(r'prefix_sum_(O[0-3])\.txt', filepath)
        opt_level = opt_level.group(1) if opt_level else "Unknown"
        
        runs = re.split(r'Profiling operation prefix_sum, n = (\d+),', content)[1:]
        for i in range(0, len(runs), 2):
            n_val = runs[i]
            run_data = runs[i+1]
            
            dtypes = re.findall(r'(Integer|Doubles):\s*MIN:\s*(\d+) ns.*?MAX:\s*(\d+) ns.*?AVG:\s*(\d+) ns.*?MED:\s*(\d+) ns.*?ELEMENTS/S:\s*([\d.e+]+)', run_data, re.DOTALL)
            for dtype in dtypes:
                dtype_name, min_ns, max_ns, avg_ns, med_ns, rate = dtype
                writer.writerow([n_val, opt_level, dtype_name, min_ns, max_ns, avg_ns, med_ns, rate])

if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    results_dir = os.path.join(script_dir, '..', 'results')
    data_dir = os.path.join(script_dir, '..', 'data')

    os.makedirs(data_dir, exist_ok=True)
    os.chdir(results_dir)

    for file in os.listdir('.'):
        processed = False
        
        if file == 'array_max.txt':
            parse_array_max(file)
            processed = True
        elif file.startswith('bfs_') and file.endswith('.txt'):
            parse_bfs(file)
            processed = True
        elif file.startswith('matmul') and file.endswith('.txt'):
            parse_matmul(file)
            processed = True
        elif file == 'merge_sort.txt':
            parse_merge_sort(file)
            processed = True
        elif file.startswith('prefix_sum_') and file.endswith('.txt'):
            parse_prefix_sum(file)
            processed = True
            
        if processed:
            csv_file = file.replace('.txt', '.csv')
            destination = os.path.join(data_dir, csv_file)
            shutil.move(csv_file, destination)
            print(f"processed {file} -> {destination}")