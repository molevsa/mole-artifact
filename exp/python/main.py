from show import *
from config import RunnerConfig
from show import draw_trend
from executor import get_all_benchmark, execute
from cache import *
from pprint import pprint
from pathlib import Path
import argparse

class Solver:
    def __init__(self, method, name=None, merge_all=None):
        self.method = method
        if name is None:
            self.name = method
        else:
            self.name = name

val_time = CaredValue(lambda x: x["time"], "time")
root_path = Path.cwd()
runner_path = os.path.join(root_path, "exp")

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dataset', type=str, default="circuit", choices=["circuit", "clia", "string", "string-blaze"])
    parser.add_argument('-t', '--time_out', type=int, default=600)
    parser.add_argument('-m', '--memory_limit', type=int, default=4)
    parser.add_argument('-s', '--solver', type=str, default="fold3")
    parser.add_argument('-e', '--exp', type=str, choices=["rq1", "rq2", "rq3", "all"])
    return parser.parse_args()

def run_cegis(solver_list=None, dataset=None):
    if solver_list is None:
        solver_list = [Solver("fold3", "Mole")]
    if dataset is None:
        dataset = args.dataset
    size_cache_file = os.path.join(runner_path, "size_cache", dataset + ".json")
    size_cache = load_cache(size_cache_file)
    benchmark_path = os.path.join(root_path, "benchmark", dataset)
    raw_benchmark_list = get_all_benchmark(benchmark_path)
    benchmark_list = []
    for bench in raw_benchmark_list:
        if size_cache['size_cache'][bench.split('/')[-1][:-3]][0]['status']:
            benchmark_list.append(bench)
    runner = os.path.join(exe_path, "solver")
    cache_file = os.path.join(runner_path, "result_cache", dataset + ".json")
    for solver in solver_list:
        if solver.method == "obe":
            # run all tasks, prog_size ignored
            config = RunnerConfig(runner, time_limit, memory_limit, flags=lambda task_file: [solver.method, 1], name=solver.name, repeat_num=1)
            execute(config, raw_benchmark_list, cache_file, thread_num=4)
        else:
            config = RunnerConfig(runner, time_limit, memory_limit, flags=lambda task_file: [solver.method, size_cache['size_cache'][task_file.split('/')[-1][:-3]][0]['prog_size']], name=solver.name, repeat_num=1)
            execute(config, benchmark_list, cache_file, thread_num=4)
    result = load_cache(cache_file)
    draw_trend(result, val_time, dataset + "-time.png")

def run_rq1():
    solver_list = [Solver("fold3", "Mole-FTA"), Solver("backward", "RawVSA"), Solver("obe", "OE")]
    run_cegis(solver_list, "clia")
    run_cegis(solver_list, "circuit")
    run_cegis(solver_list, "string")

def run_rq2():
    solver_list = [Solver("fold3", "Mole-Blaze"), Solver("blaze", "Blaze")]
    run_cegis(solver_list, "string-blaze")

def run_rq3():
    solver_list = [Solver("fold3", "Mole-FTA"), Solver("bfold3", "Mole-TopDown"), Solver("forward", "Mole-Iter"), Solver("cfold3", "Mole-Cart"), Solver("foldall", "Mole-AllOnce"), Solver("ffld3", "Mole-AllGroup"), Solver("fold2", "Mole-2"), Solver("fold4", "Mole-4")]
    run_cegis(solver_list, "circuit")

if __name__ == "__main__":
    args = parse_args()
    time_limit = args.time_out
    memory_limit = args.memory_limit
    if args.exp is not None:
        if args.exp == 'rq1' or args.exp == 'all':
            src_path = os.path.join(root_path, "src")
            exe_path = os.path.join(src_path, "main")
            run_rq1()
        if args.exp == 'rq2' or args.exp == 'all':
            src_path = os.path.join(root_path, "src-blaze")
            exe_path = os.path.join(src_path, "main")
            run_rq2()
        if args.exp == 'rq3' or args.exp == 'all':
            src_path = os.path.join(root_path, "src")
            exe_path = os.path.join(src_path, "main")
            run_rq3()
    elif args.dataset is not None:
        src_path = os.path.join(root_path, "src")
        exe_path = os.path.join(src_path, "main")
        run_cegis([Solver(method=args.solver)], dataset=args.dataset)
    else:
        print("Usage: python main.py [-e {rq1,rq2,rq3}] [-t TIMEOUT] [-m MEMORY_LIMIT] [-s SOLVER] [-d {clia,circuit,string,string-blaze}]")
        sys.exit(1)
