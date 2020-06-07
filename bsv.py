from typing import Generator, Sequence, IO
import struct
import io

u16 = 'H'
i32 = 'i'
sizeof_i32 = 4
sizeof_u16 = 2
buffer_size = 1024 * 1024 * 5

def load(f: IO[bytes]) -> Generator[Sequence[bytes], None, None]:
    # read chunk header to get size of chunk
    while True:
        data = f.read(sizeof_i32)
        if len(data) == 0:
            break
        elif len(data) == sizeof_i32:
            # read chunk
            chunk_size = struct.unpack(i32, data)[0]
            buffer = f.read(chunk_size)
            assert len(buffer) == chunk_size, [len(buffer), chunk_size]
            # buffer = io.BytesIO(buffer)
            offset = 0
            while True:
                # maybe read max index
                data = buffer[offset:offset + sizeof_u16]
                offset += sizeof_u16
                assert len(data) in {0, sizeof_u16}
                if len(data) != sizeof_u16:
                    break
                max = struct.unpack(u16, data)[0]
                # read sizes
                size = (max + 1) * sizeof_u16
                data = buffer[offset:offset + size]
                offset += size
                assert len(data) == size
                sizes = [struct.unpack(u16, data[i * sizeof_u16:i * sizeof_u16 + sizeof_u16])[0] for i in range(size // sizeof_u16)]
                # read value bytes
                vals = []
                for size in sizes:
                    data = buffer[offset:offset + size]
                    offset += size
                    assert len(data) == size
                    assert buffer[offset:offset + 1] == b'\0'
                    offset += 1
                    vals.append(data)
                yield vals
        else:
            assert False

def dump(f: IO[bytes], xss: Sequence[Sequence[bytes]]) -> None:
    buffer = io.BytesIO()
    for xs in xss:
        # write max index
        assert sizeof_u16 == buffer.write(struct.pack(u16, len(xs) - 1))
        # write sizes
        for x in xs:
            assert sizeof_u16 == buffer.write(struct.pack(u16, len(x)))
        # write vals
        for x in xs:
            assert len(x) == buffer.write(x)
            assert 1 == buffer.write(b'\0')
    assert sizeof_i32 == f.write(struct.pack(i32, len(buffer.getvalue())))
    assert len(buffer.getvalue()) < buffer_size, f'you cant dump more than {buffer_size} bytes at a time'
    assert len(buffer.getvalue()) == f.write(buffer.getvalue())
