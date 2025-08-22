from cache import *
from show import *

from pathlib import Path
import argparse
import numpy as np

val_time = CaredValue(lambda x: x["time"], "time")
root_path = Path.cwd()
runner_path = os.path.join(root_path, "exp")

class TaskStat:
    def __init__(self, name, status, time=None, node=None, edge=None):
        self.status = status
        self.name = name
        self.time = time
        self.node = node
        self.edge = edge

datasets = ["clia", "circuit", "string", "string-blaze"]
all_exp_results = {}

def load_results(dataset):
    cache_file = os.path.join(runner_path, "result_cache", dataset + ".json")
    result_map = load_cache(cache_file)
    cur_dataset_results = {}
    for cur_solver, results in result_map.items():
        cur_solver_results = {}
        for name, result in results.items():
            status = result[0]['status']
            if status is True:
                cur_solver_results[name] = TaskStat(name, status, float(result[0]['time']), int(result[0]['node_cnt']), int(result[0]['edge_cnt']))
            else:
                cur_solver_results[name] = TaskStat(name, status)
        cur_dataset_results[cur_solver] = cur_solver_results
    return cur_dataset_results

def geometric_mean(values):
    numeric_values = []
    for v in values:
        try:
            num = float(v)
            if num > 0:
                numeric_values.append(num)
        except (ValueError, TypeError):
            continue
    if not numeric_values:
        return np.nan
    log_sum = sum(math.log(x) for x in numeric_values)
    return math.exp(log_sum / len(numeric_values))

def gen_solver_stat(baseline_result, our_result):
    time_baseline_all = []
    time_baseline_hard = []
    time_our_all = []
    time_our_hard = []
    node_baseline_all = []
    node_our_all = []
    num_solved = 0

    for name, stat_baseline in baseline_result.items():
        if not stat_baseline.status:
            continue
        num_solved += 1
        stat_our = our_result.get(name)
        if stat_our is not None and stat_our.status:
            time_baseline_all.append(stat_baseline.time)
            time_our_all.append(stat_our.time)
            node_baseline_all.append(stat_baseline.node)
            node_our_all.append(stat_our.node)
            if (stat_baseline.time >= 1 or stat_our.time >= 1):
                time_baseline_hard.append(stat_baseline.time)
                time_our_hard.append(stat_our.time)
    
    time_base = geometric_mean(time_baseline_all)
    time_our = geometric_mean(time_our_all)
    time_ratio_all = time_base / time_our
    time_base_hard = geometric_mean(time_baseline_hard)
    time_our_hard = geometric_mean(time_our_hard)
    time_ratio_hard = time_base_hard / time_our_hard
    node_base = geometric_mean(node_baseline_all)
    node_our = geometric_mean(node_our_all)
    node_ratio_all = node_base / node_our
    return num_solved, time_base, time_our, time_ratio_all, time_ratio_hard, node_base, node_our, node_ratio_all


def stat(filename, datasets, solvers):
    full_path = os.path.join(runner_path, "result_table", filename)
    with open(full_path, 'w', newline='') as f:
        f.write("dataset,solver,num_solved,time_base,time_our,time_ratio_all,time_ratio_hard,node_base,node_our,node_ratio_all\n")
        first_solver = True
        for dataset in datasets:
            first_solver = True
            first_solver_result = {}
            for solver in solvers:
                if first_solver:
                    first_solver = False
                    first_solver_results = all_exp_results[dataset][solver]
                cur_solver_results = all_exp_results[dataset][solver]
                num_solved, time_base, time_our, time_ratio_all, time_ratio_hard, node_base, node_our, node_ratio_all = gen_solver_stat(cur_solver_results, first_solver_results)
                f.write(f"{dataset},{solver},{num_solved},{time_base:.2f},{time_our:.2f},{time_ratio_all:.2f},{time_ratio_hard:.2f},{node_base:.0f},{node_our:.0f},{node_ratio_all:.2f}\n")

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-e', '--exp', type=str, default="all", choices=["all", "rq1", "rq2", "rq3"])
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    exp = args.exp
    for dataset in datasets:
        all_exp_results[dataset] = load_results(dataset)
    if (exp == "all" or exp == "rq1"):
        stat("rq1.csv", ["clia", "circuit", "string"], ["Mole-FTA", "RawVSA", "OE"])
    if (exp == "all" or exp == "rq2"):
        stat("rq2.csv", ["string-blaze"], ["Mole-Blaze", "Blaze"])
    if (exp == "all" or exp == "rq3"):
        stat("rq3.csv", ["circuit"], ["Mole-FTA", "Mole-TopDown", "Mole-Iter", "Mole-Cart", "Mole-AllOnce", "Mole-AllGroup", "Mole-2", "Mole-4"])

