package main

import (
	"bufio"
	"encoding/binary"
	"io"
	"os"
	"strconv"
	"strings"
)

func main() {
	var fields []int
	for _, x := range strings.Split(os.Args[1], ",") {
		x, err := strconv.Atoi(x)
		if err != nil {
			panic(err)
		}
		fields = append(fields, x-1)
	}
	max := int32(0)
	sizes := make([]int32, 1<<16)
	offsets := make([]int32, 1<<16)
	r := bufio.NewReader(os.Stdin)
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	// buffer4 := make([]byte, 4)
	chunk_offset := int32(0)
	chunk_size := int32(0)
	chunk_buffer := make([]byte, 1024*1024*5)
	for {

		// read chunk size
		err := binary.Read(r, binary.LittleEndian, &chunk_size)
		if err != nil {
			break
		}

		// read chunk
		_, err = io.ReadFull(r, chunk_buffer[:chunk_size])
		if err != nil {
			panic(err)
		}

		// read all rows in chunk
		chunk_offset = 0
		for chunk_offset < chunk_size {
			// read row max
			max = int32(binary.LittleEndian.Uint16(chunk_buffer[chunk_offset:]))
			chunk_offset += 2

			// read row sizes
			for i := int32(0); i <= max; i++ {
				sizes[i] = int32(binary.LittleEndian.Uint16(chunk_buffer[chunk_offset:]))
				chunk_offset += 2
			}

			// setup row offsets
			for i := int32(0); i <= max; i++ {
				offsets[i] = chunk_offset
				chunk_offset += sizes[i] + 1
			}

			// handle row
			for i, f := range fields {
				_, err = w.Write(chunk_buffer[offsets[f] : offsets[f]+sizes[f]])
				if err != nil {
					panic(err)
				}
				if i != len(fields)-1 {
					_, err = w.Write([]byte(","))
					if err != nil {
						panic(err)
					}
				}
			}
			_, err = w.Write([]byte("\n"))
			if err != nil {
				panic(err)
			}
		}
	}

}
