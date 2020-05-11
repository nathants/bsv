package main

import (
	"bufio"
	"encoding/binary"
	"os"
	"psv/row"
	"strings"

	"github.com/golang/protobuf/proto"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	f := bufio.NewWriter(os.Stdout)
	defer f.Flush()
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		r := &row.Row{Columns: parts}
		res, err := proto.Marshal(r)
		if err != nil {
			panic(err)
		}
		err = binary.Write(f, binary.LittleEndian, int32(len(res)))
		if err != nil {
			panic(err)
		}
		_, err = f.Write(res)
		if err != nil {
			panic(err)
		}
	}
}
