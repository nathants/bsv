package main

import (
	"bufio"
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
	starts := make([]int, 1<<16)
	ends := make([]int, 1<<16)
	r := bufio.NewReader(os.Stdin)
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	for {
		// read row
		line, isPrefix, err := r.ReadLine()
		if isPrefix {
			panic("line too long")
		}
		if err != nil {
			if err == io.EOF || err == io.ErrUnexpectedEOF {
				break
			}
			panic(err)
		}
		// parse row
		offset := 0
		max := 0
		for i := 0; i < len(line); i++ {
			switch line[i] {
			case byte(','):
				starts[max] = offset
				ends[max] = i
				offset = i + 1
				max += 1
			}
		}
		starts[max] = offset
		ends[max] = len(line)
		// handle row
		for i, f := range fields {
			_, err = w.Write(line[starts[f]:ends[f]])
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
