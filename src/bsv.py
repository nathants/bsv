import schema # pip install git+https://github.com/nathants/py-util git+https://github.com/nathants/py-schema
import struct
import io

BSV_CHAR = 0
BSV_INT = 1
BSV_FLOAT = 2
uint16 = 'H'
uint8 = 'B'
int32 = 'i'
float32 = 'f'
bsv_int = int32
bsv_float = float32
sizeof = {int32: 4,
          float32: 4,
          uint8: 1,
          uint16: 2}

@schema.check(yields=[(':or', bytes, int, float)])
def load(f: io.IOBase) -> None:
    # read chunk header to get size of chunk
    data = f.read(sizeof[int32])
    if len(data) == sizeof[int32]:
        # read chunk
        chunk_size = struct.unpack(int32, data)[0]
        buffer = f.read(chunk_size)
        assert len(buffer) == chunk_size, [len(buffer), chunk_size]
        buffer = io.BytesIO(buffer)
        while True:
            # maybe read max index
            data = buffer.read(sizeof[uint16])
            if len(data) != sizeof[uint16]:
                break
            max = struct.unpack(uint16, data)[0]
            # read types
            size = (max + 1) * sizeof[uint8]
            data = buffer.read(size)
            assert len(data) == size, [len(data), size]
            types = [struct.unpack(uint8, data[i * sizeof[uint8]:i * sizeof[uint8] + sizeof[uint8]])[0] for i in range(size // sizeof[uint8])]
            # read sizes
            size = (max + 1) * sizeof[uint16]
            data = buffer.read(size)
            assert len(data) == size
            sizes = [struct.unpack(uint16, data[i * sizeof[uint16]:i * sizeof[uint16] + sizeof[uint16]])[0] for i in range(size // sizeof[uint16])]
            # read value bytes
            vals = []
            for type, size in zip(types, sizes):
                data = buffer.read(size)
                assert len(data) == size
                if type == BSV_CHAR:
                    pass
                elif type == BSV_INT:
                    data = struct.unpack(bsv_int, data)[0]
                elif type == BSV_FLOAT:
                    data = struct.unpack(bsv_float, data)[0]
                else:
                    assert False
                vals.append(data)
            yield vals

@schema.check
def dump(f: io.IOBase, xss: [[(':or', bytes, int, float)]]) -> None:
    buffer = io.BytesIO()
    for xs in xss:
        # write max index
        assert sizeof[uint16] == buffer.write(struct.pack(uint16, len(xs) - 1))
        # write types
        for x in xs:
            if isinstance(x, bytes):
                x = BSV_CHAR
            elif isinstance(x, int):
                x = BSV_INT
            elif isinstance(x, float):
                x = BSV_FLOAT
            else:
                assert False
            assert sizeof[uint8] == buffer.write(struct.pack(uint8, x))
        # write sizes
        for x in xs:
            if isinstance(x, bytes):
                x = len(x)
            elif isinstance(x, int):
                x = sizeof[bsv_int]
            elif isinstance(x, float):
                x = sizeof[bsv_float]
            else:
                assert False
            assert sizeof[uint16] == buffer.write(struct.pack(uint16, x))
        # write vals
        for x in xs:
            if isinstance(x, bytes):
                pass
            elif isinstance(x, int):
                x = struct.pack(bsv_int, x)
            elif isinstance(x, float):
                x = struct.pack(bsv_float, x)
            else:
                assert False
            assert len(x) == buffer.write(x)
    assert sizeof[int32] == f.write(struct.pack(int32, len(buffer.getvalue())))
    assert len(buffer.getvalue()) == f.write(buffer.getvalue())

if __name__ == '__main__':
    import shell
    with shell.set_stream():
        with shell.climb_git_root():
            shell.run('make clean && make bsv bcat')
        with shell.tempdir():
            data = [[b'a', 1, 2.0]]
            with open('data.bsv', 'wb') as f:
                dump(f, data)
            with open('data.bsv', 'rb') as f:
                val = list(load(f))
            assert val == data, [val, data]
            shell.run('echo a,1,2.0 | bsv > data.bsv')
            with open('data.bsv', 'rb') as f:
                val = list(load(f))
            assert val == data, [val, data]
            val = shell.run('bcat data.bsv')
            assert 'a,1,2.000000' == val, val
