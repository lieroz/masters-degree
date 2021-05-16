import os
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import numpy as np

directory = 'results'

def get_stats(line):
    stats = line.strip().split(',')
    return {
        'ops': int(stats[0]),
        'peak': int(stats[1]),
        'sample': int(stats[2]),
        'usage': int(stats[3]),
    }
  
all_stats = {}

for filename in os.listdir(directory):
    parts = filename.split('-')
    key = ' - '.join([parts[3], parts[4]])
    lines = open(os.path.join(directory, filename), 'r').readlines()

    member = {
        'threads': int(parts[2]),
        'malloc': get_stats(lines[0]),
        'tcmalloc': get_stats(lines[1]),
        'jemalloc': get_stats(lines[2]),
        'mimalloc': get_stats(lines[3]),
        'rpmalloc': get_stats(lines[4]),
    }

    if key in all_stats:
        all_stats[key].append(member)
    else:
        all_stats[key] = [member]

for key in all_stats:
    all_stats[key].sort(key=lambda x: x['threads'])

for key in all_stats:
    threads = []

    malloc_ops = []
    tcmalloc_ops = []
    jemalloc_ops = []
    mimalloc_ops = []
    rpmalloc_ops = []
    # ops = []

    malloc_overhead = []
    tcmalloc_overhead = []
    jemalloc_overhead = []
    mimalloc_overhead = []
    rpmalloc_overhead = []
    # overhead = []

    for stats in all_stats[key]:
        threads.append(stats['threads'])

        malloc_ops.append(stats['malloc']['ops'])
        tcmalloc_ops.append(stats['tcmalloc']['ops'])
        jemalloc_ops.append(stats['jemalloc']['ops'])
        mimalloc_ops.append(stats['mimalloc']['ops'])
        rpmalloc_ops.append(stats['rpmalloc']['ops'])
        # ops.append((malloc_ops[-1] + tcmalloc_ops[-1] + jemalloc_ops[-1] +
        #     mimalloc_ops[-1] + rpmalloc_ops[-1]) / 5)

        malloc_overhead.append(100 * (stats['malloc']['usage'] - stats['malloc']['sample']) /
                stats['malloc']['usage'])
        tcmalloc_overhead.append(100 * (stats['tcmalloc']['usage'] - stats['tcmalloc']['sample']) /
                stats['tcmalloc']['usage'])
        jemalloc_overhead.append(100 * (stats['jemalloc']['usage'] - stats['jemalloc']['sample']) /
                stats['jemalloc']['usage'])
        mimalloc_overhead.append(100 * (stats['mimalloc']['usage'] - stats['mimalloc']['sample']) /
                stats['mimalloc']['usage'])
        rpmalloc_overhead.append(100 * (stats['rpmalloc']['usage'] - stats['rpmalloc']['sample']) /
                stats['rpmalloc']['usage'])
        # overhead.append((malloc_overhead[-1] + tcmalloc_overhead[-1] +
        #     jemalloc_overhead[-1] + mimalloc_overhead[-1] + rpmalloc_overhead[-1]) / 5)

    fig, ax = plt.subplots()

    ax.plot(threads, malloc_ops, 'o-', label='glibc malloc')
    ax.plot(threads, tcmalloc_ops, 'o-', label='tcmalloc')
    ax.plot(threads, jemalloc_ops, 'o-', label='jemalloc')
    ax.plot(threads, mimalloc_ops, 'o-', label='mimalloc')
    ax.plot(threads, rpmalloc_ops, 'o-', label='rpmalloc')
    # ax.plot(threads, ops, 'o-', label='данный метод')

    ax.set_xticks(np.arange(min(threads), max(threads) + 1, 1))
    ax.ticklabel_format(style='plain')

    ax.set_xlabel('Количество потоков')
    ax.set_ylabel('Oпeраций в секунду')
    ax.set_title(key)

    plt.legend()
    plt.show()

    fig, ax = plt.subplots()

    ax.plot(threads, malloc_overhead, 'o-', label='glibc malloc')
    ax.plot(threads, tcmalloc_overhead, 'o-', label='tcmalloc')
    ax.plot(threads, jemalloc_overhead, 'o-', label='jemalloc')
    ax.plot(threads, mimalloc_overhead, 'o-', label='mimalloc')
    ax.plot(threads, rpmalloc_overhead, 'o-', label='rpmalloc')
    # ax.plot(threads, overhead, 'o-', label='данный метод')

    ax.set_xticks(np.arange(min(threads), max(threads) + 1, 1))
    ax.ticklabel_format(style='plain')
    ax.yaxis.set_major_formatter(mtick.PercentFormatter())

    ax.set_xlabel('Количество потоков')
    ax.set_ylabel('Накладные расходы на выделение памяти')
    ax.set_title(key)

    plt.legend()
    plt.show()
