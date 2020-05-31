import sys

fields = [int(x) - 1 for x in sys.argv[1].split(',')]

buffer_size = 1024 * 512

# row metadata
starts = [0 for _ in range(1 << 16)] # type: ignore
ends   = [0 for _ in range(1 << 16)] # type: ignore

# delimiters
comma   = bytearray(b',')[0]
newline = bytearray(b'\n')[0]

# write read_buffer
write_buffer = bytearray(buffer_size)

while True:
    # read read_buffer size
    read_buffer = sys.stdin.buffer.read(buffer_size) # type: ignore
    stop = len(read_buffer) != buffer_size
    # on a full read, extend with the next full line so the read_buffer always ends with a newline
    if len(read_buffer) == buffer_size:
        read_buffer += sys.stdin.buffer.readline()
    read_offset = 0
    write_offset = 0
    max = 0
    # process read_buffer byte by byte
    for i in range(len(read_buffer)):
        # found the next column
        if read_buffer[i] == comma:
            starts[max] = read_offset
            ends[max] = i
            read_offset = i + 1
            max += 1
        # found the row end
        elif read_buffer[i] == newline:
            starts[max] = read_offset
            ends[max] = i
            read_offset = i
            # handle row
            val = b''
            for i, f in enumerate(fields):
                val += read_buffer[starts[f]:ends[f]]
                if i != len(fields) - 1:
                    val += b','
            val += b'\n'
            # maybe flush and write
            if len(val) > len(write_buffer) - write_offset:
                sys.stdout.buffer.write(write_buffer[:write_offset])
                write_offset = 0
            write_buffer[write_offset:write_offset + len(val)] = val
            write_offset += len(val)
            # reset for next row
            max = 0
    # flush
    sys.stdout.buffer.write(write_buffer[:write_offset])
    if stop:
        break
