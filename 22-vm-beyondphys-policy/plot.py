#! /usr/bin/env python3
import os

# import subprocess
import matplotlib.pyplot as plt
import numpy as np

# cache_sizes = np.arange(0, 120, 20)
cache_sizes = np.arange(1, 5)
policies = ["FIFO", "LRU", "OPT", "UNOPT", "RAND", "CLOCK"]
# these were acheived after running `run.sh`
hit_rates = [
    # FIFO
    [45.03, 83.08, 93.53, 97.42],
    # LRU
    [45.03, 88.04, 95.20, 98.30],
    # OPT
    [45.03, 88.46, 96.35, 98.73],
    # UNOPT
    # NOTE: was unable to finish running this one, as it took too long.
    [45.03, None, None, None],
    # RAND
    [45.03, 82.06, 93.16, 97.36],
    # CLOCK
    [45.03, 83.59, 94.09, 97.73],
]

# for cacheSize in cache_sizes:
#     hitRate = []
#     for policy in policies:
#         result = subprocess.run(["./paging-policy.py", "-c", "-p", policy,
#             "-f", "./vpn.txt", "-C", str(cacheSize)], stdout=subprocess.PIPE)
#         result = result.stdout.decode('utf-8')
#         hitRate.append(result)
#     hit_rates.append(hitRate)

for i in range(len(policies)):
    plt.plot(cache_sizes, hit_rates[i])

plt.legend(policies)
plt.margins(0)
plt.xticks(cache_sizes, cache_sizes)
plt.xlabel("Cache Size (Blocks)")
plt.ylabel("Hit Rate")
plt.savefig("workload.png", dpi=227)
