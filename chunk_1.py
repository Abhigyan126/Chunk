import os
import sys
import shutil
import struct

def create_output_dir(output_dir):
    new_output_dir = output_dir
    counter = 1
    while os.path.exists(new_output_dir):
        new_output_dir = f"{output_dir}_{counter}"
        counter += 1
    os.makedirs(new_output_dir)
    return new_output_dir

def encode(input_file, output_dir, chunk_size):
    with open(input_file, 'rb') as f:
        part_number = 1
        while True:
            data = f.read(chunk_size)
            if not data:
                break
            with open(os.path.join(output_dir, f"part_{part_number}.chunk"), 'wb') as chunk_file:
                chunk_file.write(data)
            part_number += 1
    
    # Write metadata to header file
    total_chunks = part_number - 1
    file_size = os.path.getsize(input_file)
    file_name = os.path.basename(input_file)
    with open(os.path.join(output_dir, "header.hdr"), 'wb') as header_file:
        header_file.write(struct.pack('<i', total_chunks))
        header_file.write(struct.pack('<q', file_size))
        header_file.write(file_name.encode('utf-8'))

def decode(input_dir, output_dir=None):
    # Read metadata from header file
    with open(os.path.join(input_dir, "header.hdr"), 'rb') as header_file:
        total_chunks = struct.unpack('<i', header_file.read(4))[0]
        file_size = struct.unpack('<q', header_file.read(8))[0]
        file_name = header_file.read().decode('utf-8')

    if output_dir is None:
        output_dir = os.getcwd()

    with open(os.path.join(output_dir, file_name), 'wb') as output_file:
        for i in range(1, total_chunks + 1):
            with open(os.path.join(input_dir, f"part_{i}.chunk"), 'rb') as chunk_file:
                output_file.write(chunk_file.read())

def main():
    if len(sys.argv) < 4:
        print("Usage for encode:", sys.argv[0], "encode <input_file> <output_directory> <chunk_size>")
        print("Usage for decode:", sys.argv[0], "decode <input_directory> [output_directory]")
        sys.exit(1)

    operation = sys.argv[1]
    if operation not in ["encode", "decode"]:
        print("Invalid operation. Please specify 'encode' or 'decode'.")
        sys.exit(1)

    if operation == "encode" and len(sys.argv) != 5:
        print("Usage for encode:", sys.argv[0], "encode <input_file> <output_directory> <chunk_size>")
        sys.exit(1)

    if operation == "decode" and len(sys.argv) not in [4, 5]:
        print("Usage for decode:", sys.argv[0], "decode <input_directory> [output_directory]")
        sys.exit(1)

    if operation == "encode":
        input_file = sys.argv[2]
        output_dir = create_output_dir(sys.argv[3])
        chunk_size = int(sys.argv[4])

        encode(input_file, output_dir, chunk_size)
    elif operation == "decode":
        input_dir = sys.argv[2]
        output_dir = sys.argv[3] if len(sys.argv) == 5 else None

        decode(input_dir, output_dir)

if __name__ == "__main__":
    main()
