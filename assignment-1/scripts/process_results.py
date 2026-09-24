import os
import re
import csv
import shutil

def parse_array_max(filepath):
    out_csv = filepath.replace('.txt', '.csv')
    bw_csv = filepath.replace('.txt', '_bandwidth.csv')
    
    with open(filepath, 'r') as f, \
         open(out_csv, 'w', newline='') as out, \
         open(bw_csv, 'w', newline='') as bw_out:
        
        content = f.read()
        
        writer = csv.writer(out)
        writer.writerow(['N', 'Data_Type', 'Configuration', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns', 'Bytes_Per_Sec'])
        
        bw_writer = csv.writer(bw_out)
        bw_writer.writerow(['N', 'Doubles_Count', 'Bandwidth', 'Min_ns', 'Max_ns', 'Avg_ns', 'Med_ns'])
        
        runs = re.split(r'Profiling operation array_max, n = (\d+),', content)[1:]
        for i in range(0, len(runs), 2):
            n_val = runs[i]
            run_data = runs[i+1]
            
            parts = re.split(r'Bandwidth', run_data)
            array_max_part = parts[0]
            
            data_types = re.split(r'--- (Integer|Double) ---', array_max_part)[1:]
            for j in range(0, len(data_types), 2):
                dtype = data_types[j]
                dtype_data = data_types[j+1]
                
                configs = re.findall(r'([A-Za-z\-,\s]+):\s*\nMIN:\s*(\d+) ns.*?MAX:\s*(\d+) ns.*?AVG:\s*(\d+) ns.*?MED:\s*(\d+) ns.*?TOTAL.*?BYTES/S:\s*([-\d.e+]+)', dtype_data, re.DOTALL)
                for config in configs:
                    name, min_ns, max_ns, avg_ns, med_ns, bps = config
                    writer.writerow([n_val, dtype.strip(), name.strip(), min_ns, max_ns, avg_ns, med_ns, bps])
            
            if len(parts) > 1:
                bw_part = parts[1]
                bw_configs = re.findall(r'(\d+)\s+doubles:\nBANDWIDTH:\s*([-\d.e+]+)\nMIN:\s*(\d+)\s+ns.*?MAX:\s*(\d+)\s+ns.*?AVG:\s*(\d+)\s+ns.*?MED:\s*(\d+)\s+ns', bw_part, re.DOTALL)
                for bw_config in bw_configs:
                    d_count, bw, min_ns, max_ns, avg_ns, med_ns = bw_config
                    bw_writer.writerow([n_val, d_count, bw, min_ns, max_ns, avg_ns, med_ns])

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

    # generate all csvs
    for file in os.listdir('.'):
        if file == 'array_max.txt':
            parse_array_max(file)
        elif file.startswith('bfs_') and file.endswith('.txt'):
            parse_bfs(file)
        elif file.startswith('matmul') and file.endswith('.txt'):
            parse_matmul(file)
        elif file == 'merge_sort.txt':
            parse_merge_sort(file)
        elif file.startswith('prefix_sum_') and file.endswith('.txt'):
            parse_prefix_sum(file)

    # move them into data folder
    for file in os.listdir('.'):
        if file.endswith('.csv'):
            destination = os.path.join(data_dir, file)
            if os.path.exists(destination):
                os.remove(destination)
            shutil.move(file, destination)
            print(f"Processed and moved -> {destination}")