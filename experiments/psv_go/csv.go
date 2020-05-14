package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
	"psv/row"

	"github.com/golang/protobuf/proto"
)

func main() {
	r := bufio.NewReader(os.Stdin)
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	for {
		var size int32
		err := binary.Read(r, binary.LittleEndian, &size)
		if err != nil {
			if err == io.EOF {
				break
			}
			panic(err)
		}
		row_buf := make([]byte, size)
		n, err := io.ReadFull(r, row_buf)
		if n != int(size) {
			panic(fmt.Sprintf("not size: %d %d", n, int(size)))
		}
		if err != nil {
			panic(err)
		}
		current_row := &row.Row{}
		err = proto.Unmarshal(row_buf, current_row)
		if err != nil {
			panic(err)
		}
		_, err = fmt.Fprintf(w, "%s,%s\n", current_row.Columns[2], current_row.Columns[6])
		if err != nil {
			panic(err)
		}
	}
}
