use std::io::{stdin, stdout, BufReader, BufWriter, Write, Read};
use std::env::args;

const MAX_COLUMNS: usize = 1 << 16;

#[inline]
fn read_i32(buf: &[u8]) -> usize {
    let mut out: usize = 0;
    let ptr_out = &mut out as *mut usize as *mut u8;
    unsafe {
        std::ptr::copy_nonoverlapping(buf.as_ptr(), ptr_out, 4);
    }
    out
}


#[inline]
fn read_u16(buf: &[u8]) -> usize {
    let mut out: usize = 0;
    let ptr_out = &mut out as *mut usize as *mut u8;
    unsafe {
        std::ptr::copy_nonoverlapping(buf.as_ptr(), ptr_out, 2);
    }
    out
}

fn main() {

    // parse args
    let fields: Vec<String> = args().collect();
    assert!(fields.len() == 2, "usage: bcut field1,field2,fieldN,...");
    let fields: Vec<usize> = fields[1]
        .split(",")
        .map(|x| x.parse::<i32>().unwrap())
        .map(|x| { assert!(x > 0 && x < MAX_COLUMNS as i32); (x - 1) as usize})
        .collect();

    // setup io
    let mut reader = BufReader::with_capacity(1024 * 512, stdin());
    let mut writer = BufWriter::with_capacity(1024 * 512, stdout());
    let mut buffer4: [u8; 4] = [0; 4];
    let mut chunk_offset: usize;
    let mut chunk_buffer: [u8; 1024*1024*5] = [0; 1024*1024*5];

    // setup state
    let mut max: usize;
    let mut sizes: [usize; MAX_COLUMNS] = [0; MAX_COLUMNS];
    let mut offsets: [usize; MAX_COLUMNS] = [0; MAX_COLUMNS];

    // process input line by line
    while let Ok(_) = reader.read_exact(&mut buffer4) {
        // read chunk size
        let chunk_size = read_i32(&buffer4);

        // read next chunk
        let mut chunk_buffer = &mut chunk_buffer[..chunk_size];
        reader.read_exact(&mut chunk_buffer).unwrap();

        // read all rows in chunk
        chunk_offset = 0;
        while chunk_offset < chunk_size {

            // read row max
            max = read_u16(&chunk_buffer[chunk_offset..]);
            chunk_offset += 2;

            // read row sizes
            for i in 0..max+1 {
                sizes[i] = read_u16(&chunk_buffer[chunk_offset..]);
                chunk_offset += 2;
            }

            // setup row offsets
            for i in 0..max+1 {
                offsets[i] = chunk_offset;
                chunk_offset += sizes[i] + 1;
            }

            // handle row
            let mut i = 0;
            for field in &fields {
                assert!(*field <= max, "found a row without enough columns");
                let offset = offsets[*field];
                let size = sizes[*field];
                writer.write_all(&chunk_buffer[offset..offset+size]).unwrap();
                i += 1;
                if i < fields.len() {
                    writer.write_all(&[b',']).unwrap();
                }
            }
            writer.write_all(&[b'\n']).unwrap();

        }
    }
}
