# Mole-artifact

Artifact for OOPSLA'25: Tunneling Through the Hill: Multi-Way Intersection for Version-Space Algebras in Program Synthesis.

## Install

The following installation way is available on Ubuntu 24.04.

1. Install dependencies. This project requires gcc $\geq 9$, CMake $\geq 3.13$, [z3](https://github.com/Z3Prover/z3), [glog](https://github.com/google/glog) and [jsoncpp](https://github.com/open-source-parsers/jsoncpp), most of which (except z3) can be installed as follows.

```bash
$ sudo apt install gcc cmake libjsoncpp-dev libgoogle-glog-dev
```

You need to build z3 from source as follows.

```
$ git clone https://github.com/Z3Prover/z3
$ cd z3
$ python scripts/mk_make.py
$ cd build
$ make
```


2. Clone *Mole*.

```bash
$ git clone https://github.com/molevsa/mole-artifact
```

3. Build *Mole*.

You need to set the path of z3 in `src/CMakeLists.txt` and `src-blaze/CMakeLists.txt`.

```cmake
set(Z3_PATH /path/to/z3)
```

Then, build *Mole* and *Mole-Blaze* respectively.

```bash
# assume you're in the root directory of the project
# build Mole
$ cd src
$ cmake .
$ make
# build Mole-Blaze
$ cd src-blaze
$ cmake .
$ make
```

4. Modify the config of *Mole*. You need to set the variable `config::KSourcePath` in `src/basic/config.cpp` (and `src-blaze/basic/config.cpp`) to the root source code directory.

```cpp
// mole in $MOLE_ROOT/src
const std::string config::KSourcePath =
    "/mole-root/src";
// mole-blaze in $MOLE_ROOT/src-blaze
const std::string config::KSourcePath =
    "/mole-root/src-blaze";
```

## Run Mole on a Single Task

```bash
# run mole
$ src/main/run_cegis <task_path> <result_path> <exe_type>
# run mole-blaze
$ src-blaze/main/run_cegis <task_path> <result_path> <exe_type> <merge_all>

# run a specific task
$ src/main/run_cegis /path/to/mole/benchmark/string/2171308.sl result.txt fold3
$ src-blaze/main/run_cegis /path/to/mole/benchmark/string-blaze/bikes.sl result.txt fold3 true
```

1. `task_path`: the path to task file (in absolute path) in SyGuS format to synthesis.
2. `result_path`: The path to the result file to write.
3. `exe_type`: the merge method. The value can be `forward` (bottom-up merge), `backward` (top-down merge), `foldX` (k-way bottom-up merge, where X is a positive number representing the argument k), `bfoldX` (k-way top-down merge, where X is a positive number representing the argument k). Note that the `bfoldX` method is only available in *Mole*. *Mole* uses `fold3` merge method.
4. `merge_all`: whether build a VSA with all examples or not. The value can be `true` (the way *Blaze* uses to construct VSAs) or `false` (the way *Mole-Blaze* uses to construct VSAs). Only available in *Mole-Blaze*.

## Reproduce Results in Our Paper

We provide a script `exp/python/main.py` to reproduce the results in our paper. Its usage is as follows.

```bash
# You need to run the script in root directory of Mole
$ python exp/python/main.py [-e {rq1,rq2,rq3,all}] [-t TIMEOUT] [-m MEMORY_LIMIT]
                          [-merge MERGE_ALL] [-s SOLVER] [-d {clia,circuit,string,string-blaze}]

# Run specific experiment in our paper
$ python exp/python/main.py -e rq1
# Or run evaluation on a specific dataset
$ python exp/python/main.py -d clia -t 60
```


1. `-exp`: the name of the experiment. `rq1`, `rq2`, and `rq3` correspond to Tables 7, 8, and 9 in our paper, respectively, and `all` donates all of them. When this argument is set, other arguments (except `-t` and `-m`) will be ignored.
2. `-t`: the time out of each execution (in seconds), where the default value is 600.
3. `-m`: the memory limit of each execution (in GB), where the default value is 4.
4. `-merge`: whether build a VSA for all examples at once or not. Only valid when the dataset is `string-blaze`.
5. `-s`: the name of solver used, where the default value is `fold3` (the same configuration as *Mole*).
6. `-d`: the dataset to be evaluated.

## The Structure of This Repository

### Evaluation results

All evaluation results are available in directory `exp`.

The performance is summarized in `exp/result_cache/*.json`, where the synthesis status (fail or success), the program *Mole* synthesises, the number of nodes and edges constructed during the synthesis progress and the time cost are recorded.

Besides, all benchmarks in our dataset are available in directory `benchmark`.

### Source code

Our source code of *Mole* is stored in `src`, while our re-implementation version of *Blaze* in our framework (*Mole-Blaze*) is stored in  `src-blaze`.

The structure of `src` and `src-blaze` are the same, organized as follows.

|    Directory     |               Description                |
| :--------------: | :--------------------------------------: |
| `include` |                The head files of our project.                |
| `basic`   |       The basic data structure used in program synthesis process, including values, programs, specification, etc.       |
| `ext` | The extension data structure for Z3 framework. |
| `fta` | The data structure and algorithm of VSA. |
| `sygus` | The data structure and interface of SyGuS framework, including the definitions of operators and the parser of program synthesis tasks in SyGuS format. |
| `main` | The entrance of invoking *Mole* and *Mole-Blaze*. |
