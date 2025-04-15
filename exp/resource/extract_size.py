import os
import json

with open("../cache/cegis.json", "r") as inp:
    cache = json.load(inp)
cache = cache["forward"]

size_map = {}

for task, result in cache.items():
    result = result[0]
    if not result["status"]: continue
    output = result["result"]
    if len(output) < 2: continue
    size = int(output[-1])
    size_map[task] = size
    print(task, size)

with open("size.json", "w") as oup:
    json.dump(size_map, oup, indent=4)