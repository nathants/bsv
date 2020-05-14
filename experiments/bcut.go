package main

import (
	"bufio"
	"fmt"
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
		fields = append(fields, x)
	}
	scanner := bufio.NewScanner(os.Stdin)
	f := bufio.NewWriter(os.Stdout)
	defer f.Flush()
	for scanner.Scan() {
		line := scanner.Text()
		parts := strings.Split(line, ",")
		selected := make([]string, len(fields))
		for i, field := range fields {
			selected[i] = parts[field-1]
		}

		fmt.Fprintln(f, strings.Join(selected, ","))
	}
}
