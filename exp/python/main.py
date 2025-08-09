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
        self.merge_all = merge_all
        if name is None:
            self.name = ("Mole" if (self.method == "fold3") else method)
            if merge_all is not None:
                self.name = self.name + ("-merge_all" if merge_all == "true" else "")
        else:
            self.name = name

val_time = CaredValue(lambda x: x["time"], "time")
root_path = Path.cwd()
runner_path = os.path.join(root_path, "exp")

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dataset', type=str, default="string-blaze", choices=["circuit", "clia", "string", "string-blaze"])
    parser.add_argument('-t', '--time_out', type=int, default=600)
    parser.add_argument('-m', '--memory_limit', type=int, default=4)
    parser.add_argument('-merge', '--merge_all', type=str, default="false", choices=["true", "false"])
    parser.add_argument('-s', '--solver', type=str, default="fold3", choices=(["forward", "backward"] + ["fold" + str(k) for k in range(2, 6)] + ["bfold" + str(k) for k in range(2, 6)]))
    parser.add_argument('-e', '--exp', type=str, choices=["rq1", "rq2", "rq3", "all"])
    return parser.parse_args()

def run_cegis_blaze(solver_list=None):
    dataset = "string-blaze"
    if solver_list is None:
        solver_list = [Solver("fold3", "Mole")]
    benchmark_path = os.path.join(root_path, "benchmark", dataset)
    benchmark_list = get_all_benchmark(benchmark_path)
    runner = os.path.join(exe_path, "run_cegis")
    cache_file = os.path.join(runner_path, "result_cache", dataset + ".json")
    for solver in solver_list:
        config = RunnerConfig(runner, time_limit, memory_limit, flags=lambda _: [solver.method, solver.merge_all], name=solver.name, repeat_num=1)
        execute(config, benchmark_list, cache_file, thread_num=4)
    result = load_cache(cache_file)
    draw_trend(result, val_time, dataset + "-time.png")

def run_cegis(solver_list=None, dataset=None):
    if solver_list is None:
        solver_list = [Solver("fold3", "Mole-Blaze", "false")]
    if dataset is None:
        dataset = args.dataset
    benchmark_path = os.path.join(root_path, "benchmark", dataset)
    benchmark_list = get_all_benchmark(benchmark_path)
    runner = os.path.join(exe_path, "run_cegis")
    cache_file = os.path.join(runner_path, "result_cache", dataset + ".json")
    for solver in solver_list:
        config = RunnerConfig(runner, time_limit, memory_limit, flags=lambda _: [solver.method], name=solver.name, repeat_num=1)
        execute(config, benchmark_list, cache_file, thread_num=4)
    result = load_cache(cache_file)
    draw_trend(result, val_time, dataset + "-time.png")

def run_merge(example_num):
    if solver_list is None:
        solver_list = [Solver("fold3", "Mole")]

    raw_benchmarks = get_all_benchmark(benchmark_path)
    raw_size_map = load_cache(os.path.join(runner_path, "resource", "size.json"))
    size_map, benchmarks = {}, []
    for task in raw_benchmarks:
        task_name = os.path.basename(task)
        task_name = os.path.splitext(task_name)[0]
        if task_name in raw_size_map:
            benchmarks.append(task)
            size_map[task] = raw_size_map[task_name]

    runner = os.path.join(exe_path, "run_merge")
    cache_file = os.path.join(runner_path, "result_cache", "merge.json")
    for solver in solver_list:
        flag = lambda task: [solver.method, size_map[task], example_num]
        config = RunnerConfig(runner, time_limit, memory_limit, flags=flag, name=solver.name, repeat_num=1)
        execute(config, benchmarks, cache_file, thread_num=4)
    result = load_cache(cache_file)
    draw_trend(result, val_time, "merge-time.png")

def run_rq1():
    solver_list = [Solver("fold3", "Mole"), Solver("backward", "RawVSA")]
    run_cegis(solver_list, "clia")
    run_cegis(solver_list, "circuit")
    run_cegis(solver_list, "string")

def run_rq2():
    solver_list = [Solver("fold3", "Mole-Blaze", "false"), Solver("fold3", "Blaze", "true")]
    run_cegis_blaze(solver_list)

def run_rq3():
    solver_list = [Solver("fold3", "Mole"), Solver("bfold3", "Mole-TopDown")]
    run_cegis(solver_list, "clia")
    run_cegis(solver_list, "circuit")
    run_cegis(solver_list, "string")

if __name__ == "__main__":
    args = parse_args()
    time_limit = args.time_out
    merge_all = args.merge_all
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
    elif args.dataset == "string-blaze":
        src_path = os.path.join(root_path, "src-blaze")
        exe_path = os.path.join(src_path, "main")
        run_cegis_blaze([Solver(method=args.solver, merge_all=args.merge_all)])
    elif args.dataset is not None:
        src_path = os.path.join(root_path, "src")
        exe_path = os.path.join(src_path, "main")
        run_cegis([Solver(method=args.solver)], dataset=args.dataset)
    else:
        print("Usage: python main.py [-e {rq1,rq2,rq3}] [-t TIMEOUT] [-m MEMORY_LIMIT] [-merge MERGE_ALL] [-s SOLVER] [-d {clia,circuit,string,string-blaze}]")
        sys.exit(1)
