import sys

delimiter = sys.argv[1]
fields = [int(x) - 1 for x in sys.argv[2].split(',')]

for line in sys.stdin:
    parts = line.split(',')
    size = len(parts)
    selected = []
    for field in fields:
        if field < size:
            selected.append(parts[field])
    print(','.join(selected))
