#
# usage:
#   python3 tools/combine.py [-rkey=value]... "a1 a2 a3 a4 a5 a6"... "b1 b2 b3 b4"... ....
# return all combinations e.g. a1 b1... a1 b2...
#

import sys


def recursive(i):
    if i == len(sys.argv):
        return ['']
    x = recursive(i + 1)
    return [x1 + x2 for x1 in sys.argv[i].split('|') for x2 in x]

start = 1

while sys.argv[start][0:2] == '-r':
    key, val = sys.argv[start][2:].split('=')
    start += 1
    for i in range(start, len(sys.argv)):
        # print(start, '\nr:', sys.argv[i], '\nk:', key, file=sys.stderr)
        if sys.argv[i] == key:
            sys.argv[i] = val
            # print(sys.argv, file=sys.stderr)

print(*recursive(start), sep='', end='')
