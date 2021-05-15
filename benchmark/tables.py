from prettytable import PrettyTable
import os

directory = 'results'

groups = {}
  
for filename in os.listdir(directory):
    parts = filename.split('-')
    key = '_'.join([parts[3], parts[4]])

    if int(parts[2]) == 16:
        member = {
            'threads': int(parts[2]),
            'begin': int(parts[3]),
            'end': int(parts[4]),
            'filename': os.path.join(directory, filename),
        }

        if key in groups:
            groups[key].append(member)
        else:
            groups[key] = [member]

data = {}

for key in groups:
    members = []

    for group in groups[key]:
        file = open(group['filename'], 'r')
        member = {
            'threads': group['threads']
        }
        allocators = []

        for i, line in enumerate(file.readlines()):
            line = line.strip()
            values = line.split(',')
            allocators.append({
                'mops': int(values[0]),
                'peak': int(values[1]),
                'sample': int(values[2]),
                'usage': int(values[3])
            })

        member['allocators'] = allocators
        members.append(member)
    data[key] = members

for key in data:
    for i in data[key]:
        malloc = i['allocators'][0]
        tcmalloc = i['allocators'][1]
        jemalloc = i['allocators'][2]
        mimalloc = i['allocators'][3]
        rpmalloc = i['allocators'][4]

        print('threads: {}\trange: {}'.format(i['threads'], ' -> '.join(key.split('_'))))
        result = ''
        result += '\tallocator: malloc, \t' + str(malloc['mops']) + '\t' + str(round(malloc['sample'] / 1024 / 1024, 2)) + '\t' + str(round(malloc['usage'] / 1024 / 1024, 2)) + '\t{}%\n'.format(str(round(100 * (malloc['usage'] - malloc['sample']) / malloc['usage'], 2)))
        result += '\tallocator: tcmalloc, \t' + str(tcmalloc['mops']) + '\t' + str(round(tcmalloc['sample'] / 1024 / 1024, 2)) + '\t' + str(round(tcmalloc['usage'] / 1024 / 1024, 2)) + '\t{}%\n'.format(str(round(100 * (tcmalloc['usage'] - tcmalloc['sample']) / tcmalloc['usage'], 2)))
        result += '\tallocator: jemalloc, \t' + str(jemalloc['mops']) + '\t' + str(round(jemalloc['sample'] / 1024 / 1024, 2)) + '\t' + str(round(jemalloc['usage'] / 1024 / 1024, 2)) + '\t{}%\n'.format(str(round(100 * (jemalloc['usage'] - jemalloc['sample']) / jemalloc['usage'], 2)))
        result += '\tallocator: mimalloc, \t' + str(mimalloc['mops']) + '\t' + str(round(mimalloc['sample'] / 1024 / 1024, 2)) + '\t' + str(round(mimalloc['usage'] / 1024 / 1024, 2)) + '\t{}%\n'.format(str(round(100 * (mimalloc['usage'] - mimalloc['sample']) / mimalloc['usage'], 2)))
        result += '\tallocator: rpmalloc, \t' + str(rpmalloc['mops']) + '\t' + str(round(rpmalloc['sample'] / 1024 / 1024, 2)) + '\t' + str(round(rpmalloc['usage'] / 1024 / 1024, 2)) + '\t{}%\n'.format(str(round(100 * (rpmalloc['usage'] - rpmalloc['sample']) / rpmalloc['usage'], 2)))

        print(result)
