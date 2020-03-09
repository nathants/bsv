from typing import Generator, Union, List
import struct
import io

_BSV_CHAR = 0
_BSV_INT = 1
_BSV_FLOAT = 2
_uint16 = 'H'
_uint8 = 'B'
_int32 = 'i'
_int64 = 'q'
_float32 = 'f'
_float64 = 'd'
_bsv_int = _int64
_bsv_float = _float64
_sizeof = {_int32: 4,
           _int64: 8,
           _float64: 8,
           _uint8: 1,
           _uint16: 2}
_buffer_size = 1024 * 1024 * 5

def load(f: io.IOBase) -> Generator[Union[bytes, int, float], None, None]:
    # read chunk header to get size of chunk
    while True:
        data = f.read(_sizeof[_int32])
        if len(data) == 0:
            break
        elif len(data) == _sizeof[_int32]:
            # read chunk
            chunk_size = struct.unpack(_int32, data)[0]
            buffer = f.read(chunk_size)
            assert len(buffer) == chunk_size, [len(buffer), chunk_size]
            buffer = io.BytesIO(buffer)
            while True:
                # maybe read max index
                data = buffer.read(_sizeof[_uint16])
                assert len(data) in {0, _sizeof[_uint16]}
                if len(data) != _sizeof[_uint16]:
                    break
                max = struct.unpack(_uint16, data)[0]
                # read types
                size = (max + 1) * _sizeof[_uint8]
                data = buffer.read(size)
                assert len(data) == size, [len(data), size]
                types = [struct.unpack(_uint8, data[i * _sizeof[_uint8]:i * _sizeof[_uint8] + _sizeof[_uint8]])[0] for i in range(size // _sizeof[_uint8])]
                # read sizes
                size = (max + 1) * _sizeof[_uint16]
                data = buffer.read(size)
                assert len(data) == size
                sizes = [struct.unpack(_uint16, data[i * _sizeof[_uint16]:i * _sizeof[_uint16] + _sizeof[_uint16]])[0] for i in range(size // _sizeof[_uint16])]
                # read value bytes
                vals = []
                for type, size in zip(types, sizes):
                    data = buffer.read(size)
                    assert buffer.read(1) == b'\0'
                    assert len(data) == size
                    if type == _BSV_CHAR:
                        pass
                    elif type == _BSV_INT:
                        data = struct.unpack(_bsv_int, data)[0]
                    elif type == _BSV_FLOAT:
                        data = struct.unpack(_bsv_float, data)[0]
                    else:
                        assert False
                    vals.append(data)
                yield vals
        else:
            assert False

def dump(f: io.IOBase, xss: List[List[Union[bytes, int, float]]]) -> None:
    buffer = io.BytesIO()
    for xs in xss:
        # write max index
        assert _sizeof[_uint16] == buffer.write(struct.pack(_uint16, len(xs) - 1))
        # write types
        for x in xs:
            if isinstance(x, bytes):
                x = _BSV_CHAR
            elif isinstance(x, int):
                x = _BSV_INT
            elif isinstance(x, float):
                x = _BSV_FLOAT
            else:
                assert False
            assert _sizeof[_uint8] == buffer.write(struct.pack(_uint8, x))
        # write sizes
        for x in xs:
            if isinstance(x, bytes):
                x = len(x)
            elif isinstance(x, int):
                x = _sizeof[_bsv_int]
            elif isinstance(x, float):
                x = _sizeof[_bsv_float]
            else:
                assert False
            assert _sizeof[_uint16] == buffer.write(struct.pack(_uint16, x))
        # write vals
        for x in xs:
            if isinstance(x, bytes):
                pass
            elif isinstance(x, int):
                x = struct.pack(_bsv_int, x)
            elif isinstance(x, float):
                x = struct.pack(_bsv_float, x)
            else:
                assert False
            assert len(x) == buffer.write(x)
            assert 1 == buffer.write(b'\0')
    assert _sizeof[_int32] == f.write(struct.pack(_int32, len(buffer.getvalue())))
    assert len(buffer.getvalue()) < _buffer_size, f'you cant dump more than {_buffer_size} bytes at a time'
    assert len(buffer.getvalue()) == f.write(buffer.getvalue())
