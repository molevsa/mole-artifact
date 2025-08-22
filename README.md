# Mole

Artifact for OOPSLA'25: Tunneling Through the Hill: Multi-Way Intersection for Version-Space Algebras in Program Synthesis.

The updates of this project can be found on [GitHub](https://github.com/StudyingFather/mole).

## Introduction

We implement two VSA-based synthesizers in our artifact. The one is a general synthesizer called *Mole-FTA*, and the other is a specialized synthesizer based on *Blaze* called *Mole-Blaze*. Additionally, some baseline solvers are implemented in our artifact for comparison.

This artifact supports the following claims in our paper:

- *Mole-FTA* outperforms standard VSA-based general synthesizer on the SyGuS-Comp dataset.
- The intersection way of *Mole-FTA* can improve the specialized VSA-based solver like *Blaze*.
- Bottom-up intersection plays an effective role in *Mole-FTA*.

## Hardware Dependencies

The experiments in our paper are conducted on Intel(R) Xeon(R) Platinum 8369HC 8-Core CPU. At least 16GB of RAM is required to run our artifact efficiently.

## Getting Started Guide

### Install

#### Build from Source (Tested on Ubuntu 22.04)

1. Install dependencies. This project requires gcc $\geq 9$, CMake $\geq 3.13$, [z3](https://github.com/Z3Prover/z3), [glog](https://github.com/google/glog), [jsoncpp](https://github.com/open-source-parsers/jsoncpp) and so on, most of which (except z3) can be installed as follows.

```bash
$ sudo apt install gcc cmake libjsoncpp-dev libgoogle-glog-dev python3 python3-pip
$ pip install pyparsing matplotlib tqdm
```

You need to build z3 from source, which can be done in the build script `install`.

2. Clone *Mole*.

```bash
$ git clone https://github.com/StudyingFather/mole
```

3. Build *Mole* under the root directory of the project.

```bash
$ cd mole-artifact; ./install
```

#### Download Docker Image

We also released a docker image of *Mole*. You can download and run it using the following commands.

```bash
$ docker pull studyingfather/mole
$ docker run -i -t studyingfather/mole
```

In this image, *Mole* is built under directory `/root/mole`. Its file structure is the same as this repository (see the last section of this document).

### Run Tests

**Note**: In the remainder of this document, all commands are assumed to be executed in the root directory of *Mole*, which is `/root/mole` in our docker image and is the directory created by git clone if *Mole* is built from the source.

1. Test whether *Mole* is successfully built:

```bash
# you need to fill in the absolute path of the task file
$ src/main/solver /root/mole/benchmark/string/2171308.sl result.txt fold3 10
$ src-blaze/main/solver /root/mole/benchmark/string-blaze/bikes.sl result.txt fold3 10
```

The output (in `stdout` and `result.txt`) should contain the target program, the number of VSA nodes and VSA edges constructed in total, the number of examples used and the time cost in the synthesis process.

2. Test whether test script is work:

**Note**: The test script only runs tasks that do not have experiment results in `exp/result_cache`. So you need to backup and remove original experiment result first to run the test script.

```bash
# remove original experiment result
$ rm exp/result_cache/string-blaze.json
$ python3 exp/python/main.py -e rq2 -t 1
```

The test result will be stored in `exp/result_cache/string-blaze.json` (see the **Evaluation Results** section for the detailed information), and a figure (`string-blaze-time.png`) shows the number of tasks solved by different solvers over time will be drawn.

## Step by Step Instructions

### Run Mole on a Single Task

You can run *Mole-FTA* and *Mole-Blaze* directly by executing the executable in the following way.

```bash
# run mole-fta
$ src/main/solver <task_path> <result_path> <exe_type> <prog_size>
# run mole-blaze
$ src-blaze/main/solver <task_path> <result_path> <exe_type> <prog_size>

# Example: run a specific task
$ src/main/solver /root/mole/benchmark/string/2171308.sl result.txt fold3 10
$ src-blaze/main/solver /root/mole/benchmark/string-blaze/bikes.sl result.txt fold3 10
```

1. `task_path`: the path to task file (in absolute path) in SyGuS format to synthesis.
2. `result_path`: The path to the result file to write.
3. `exe_type`: the intersection method. The value can be `forward` (bottom-up intersection), `backward` (top-down intersection), `foldX` (k-way bottom-up intersection, where X is a positive number representing the argument k), `bfoldX` (k-way top-down intersection), `cfoldX` (k-way bottom-up intersection, building the entire Cartesian product in the intersection process), `foldall` (n-way bottom-up intersection, where n is the number of examples), `ffldX` (k-way bottom-up intersection, with no termination of grouping intersection process), `blaze` (the way *Blaze* constructs the multi-example VSA). Note that the `bfoldX` method is only available in *Mole-FTA*, and `blaze` method is only available in *Mole-Blaze*. *Mole-FTA* and *Mole-Blaze* use `fold3` merge method.
4. `prog_size`: the size of the target program *Mole-FTA* or *Mole-Blaze* needs to synthesis.

The relationship between the `exe_type` argument when invoking our program and the name of solvers in our paper is shown in the following table:

|         invoker         | `exe_type` |        Solvers in our paper        |
| :---------------------: | :--------: | :--------------------------------: |
|    `src/main/solver`    |  `fold3`   |    $\mathrm{Mole}_\mathrm{FTA}$    |
|    `src/main/solver`    | `backward` |         $\mathrm{RawVSA}$          |
|    `src/main/solver`    |   `obe`    |           $\mathrm{OE}$            |
|    `src/main/solver`    |  `bfold3`  | $\mathrm{Mole}_\mathrm{Top-Down}$  |
|    `src/main/solver`    | `forward`  |   $\mathrm{Mole}_\mathrm{iter}$    |
|    `src/main/solver`    |  `cfold3`  |   $\mathrm{Mole}_\mathrm{Cart}$    |
|    `src/main/solver`    | `foldall`  | $\mathrm{Mole}_\mathrm{All-Once}$  |
|    `src/main/solver`    |  `ffld3`   | $\mathrm{Mole}_\mathrm{All-Group}$ |
|    `src/main/solver`    |  `fold2`   |         $\mathrm{Mole}_2$          |
|    `src/main/solver`    |  `fold4`   |         $\mathrm{Mole}_4$          |
| `src-blaze/main/solver` |  `fold3`   |   $\mathrm{Mole}_\mathrm{Blaze}$   |
| `src-blaze/main/solver` |  `blaze`   |          $\mathrm{Blaze}$          |

### Reproduce Results in Our Paper

We provide a script `exp/python/main.py` to reproduce the experiment results in our paper. Its usage is as follows.

```bash
# You need to run the script in root directory of Mole
$ python3 exp/python/main.py [-e {rq1,rq2,rq3,all}] [-t TIMEOUT] [-m MEMORY_LIMIT]
                             [-s SOLVER] [-d {clia,circuit,string,string-blaze}]

# Example: run specific experiment in our paper
$ python3 exp/python/main.py -e rq1
# Example: run evaluation on a specific dataset
$ python3 exp/python/main.py -d clia -t 60
```

1. `-exp`: the name of the experiment. `rq1`, `rq2`, and `rq3` correspond to Tables 7, 8, and 9 in our paper, respectively, and `all` donates all of them. When this argument is set, other arguments (except `-t` and `-m`) will be ignored.
2. `-t`: the time out of each execution (in seconds), where the default value is 600.
3. `-m`: the memory limit of each execution (in GB), where the default value is 4.
4. `-s`: the name of solver used, where the default value is `fold3` (the same configuration as *Mole*).
5. `-d`: the dataset to be evaluated.

The experiment result will be stored in `exp/result_cache/*.json`. View the **Evaluation Results** section for the detailed information.

**Note**: The test script only runs tasks that do not have experiment results in `exp/result_cache`. So you need to backup and remove original experiment result first to run the test script.

```bash
# backup the original experiment result first
$ cp -r exp/result_cache exp/result_cache_bak
# remove the result files
$ rm exp/result_cache/*
# now you can run the test script to reproduce the result
$ python3 exp/python/main.py -e all

# or you just want to rerun a specific dataset
$ rm exp/result_cache/clia.json
$ python3 exp/python/main.py -d clia
```

After running the test script, you can export the summarized result tables by running the script `exp/python/result_table_gen.py`. The tables are organized in the same way as our papers, and they will be stored in `exp/result_table` directory.

```bash
# export all experiment result tables
$ python3 exp/python/result_table_gen.py
# or a specific one
$ python3 exp/python/result_table_gen.py -e rq1
```

**Note**: It takes about a day to run the entire experiment due to the excessive number of tasks.

## Reusability Guide

Our artifact can deal with SyGuS tasks in common domains like integer arithmetic, cryptographic circuits and so on. For new tasks in these domains, *Mole* can simply deal with them without any modification.

To support more SyGuS tasks in new domains, you need to implement corresponding values, types and semantics in `src/sygus/theory` folders.

## The Structure of This Repository

### Appendix

The proofs of lemmas and theorems in our paper are available in `appendix.pdf`.

### Evaluation Results

All evaluation results are available in directory `exp`.

The performance is summarized in `exp/result_cache/*.json`, where the synthesis status (fail or success), the program solvers synthesises, the number of VSA nodes and VSA edges constructed during the synthesis progress and the time cost are recorded.

Besides, all benchmarks in our dataset are available in directory `benchmark`.

### Source Code

Our source code of *Mole-FTA* is stored in `src`, while our re-implementation version of *Blaze* in our framework (*Mole-Blaze*) is stored in  `src-blaze`.

The structure of `src` and `src-blaze` are the same, organized as follows.

|    Directory     |               Description                |
| :--------------: | :--------------------------------------: |
| `include` | The head files of our project. |
| `basic`   | The basic data structure used in program synthesis process, including values, programs, specification, etc. |
| `ext` | The extension data structure for Z3 framework. |
| `fta` | The data structure and algorithm of VSA. |
| `sygus` | The data structure and interface of SyGuS framework, including the definitions of operators and the parser of program synthesis tasks in SyGuS format. |
| `main` | The entrance of invoking *Mole-FTA* and *Mole-Blaze*. |
