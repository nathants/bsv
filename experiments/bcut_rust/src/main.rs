use bytelines::ByteLinesReader;
use std::io::{stdin, stdout, BufReader, BufWriter, Write};
use std::env::args;

const MAX_COLUMNS: usize = 1 << 16;

fn main() {
    // parse args
    let fields: Vec<String> = args().collect();
    let fields: Vec<usize> = fields[1]
        .split(",")
        .map(|x| x.parse::<i32>().unwrap())
        .map(|x| { assert!(x > 0 && x < MAX_COLUMNS as i32); (x - 1) as usize})
        .collect();
    // setup io
    let reader = BufReader::with_capacity(1024 * 64, stdin());
    let mut writer = BufWriter::with_capacity(1024 * 64,stdout());
    let mut lines = reader.byte_lines();
    // setup state
    let mut offsets: [usize; MAX_COLUMNS] = [0; MAX_COLUMNS];
    let mut lens:    [usize; MAX_COLUMNS] = [0; MAX_COLUMNS];
    // process input line by line
    while let Some(line) = lines.next() {
        let line = line.unwrap();
        if line.len() > 0 {
            // discover the fields of this row
            let mut max = 0;
            let mut offset = 0;
            for (i, part) in line.split(|val| val == &b',').enumerate() {
                offsets[i] = offset;
                lens[i] = part.len();
                offset += part.len() + 1;
                max = i;
            }
            // output the chosen fields
            let mut i = 0;
            for field in &fields {
                assert!(*field <= max, "found a row without enough columns");
                let offset = offsets[*field];
                let len = lens[*field];
                writer.write_all(&line[offset..offset+len]).unwrap();
                i += 1;
                if i < fields.len() {
                    writer.write_all(&[b',']).unwrap();
                }
            }
            writer.write_all(&[b'\n']).unwrap();
        }
    }
}
