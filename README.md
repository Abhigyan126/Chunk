# Chunk

This program allows you to encode large files into smaller chunks and decode them back to the original file.

## Usage
### Decoding
```bash

./chunk decode <input_directory> <output_directory>
./chunk decode container decoded_output
```
### Encoding
```bash
./chunk encode <input_file> <output_directory> <chunk_size>

./chunk encode image.webp container 1048576
encodes image.webp to 1mb chunk
```

Requirements
C++17
Compiler with C++17 support
