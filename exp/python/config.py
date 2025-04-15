import os

def _get_bin_name(bin_file):
    return os.path.basename(bin_file)

class RunnerConfig:
    def __init__(self, bin_file: str, time_limit: int, memory_limit: int, flags=None, name=None, repeat_num=1):
        if flags is None: flags = lambda path: []
        self.bin_file = bin_file
        self.time_limit = time_limit
        self.memory_limit = memory_limit
        self.flags = flags
        self.name = name
        self.repeat_num = repeat_num

    def build_command(self, task_file: str, result_file: str):
        command = ['ulimit -v ' + str(self.memory_limit * 1024 * 1024) + ';',
                   "timeout " + str(self.time_limit), self.bin_file,
                   task_file, result_file]
        flags = list(map(str, self.flags(task_file)))
        command = command + flags + [">/dev/null", "2>/dev/null"]
        print(" ".join(command))
        return " ".join(command)
    