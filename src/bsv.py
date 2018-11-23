import struct

def load(f):
    max = struct.unpack('H', f.read(2))[0]
    size = (max + 1) * 2
    data = f.read(size)
    sizes = [struct.unpack('H', data[i * 2:i * 2 + 2])[0] for i in range(size // 2)]
    return [f.read(size) for size in sizes]

def dump(f, xs):
    f.write(struct.pack('H', len(xs) - 1))
    for x in xs:
        f.write(struct.pack('H', len(x)))
    for x in xs:
        f.write(x)
