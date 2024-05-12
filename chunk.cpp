#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct FileHeader {
    int totalChunks;
    std::string fileName;
    long fileSize;
};

std::string createOutputDir(const std::string& outputDir) {
    std::string newOutputDirectory = outputDir;
    int counter = 1;
    while (fs::exists(newOutputDirectory)) {
        newOutputDirectory = outputDir + "_" + std::to_string(counter);
        counter++;
    }
    fs::create_directory(newOutputDirectory);
    return newOutputDirectory;
}

void encode(const std::string& input, const std::string& outputDir, int chunkSize) {
    std::ifstream inputFile(input, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Exception: Cannot open file " << input << std::endl;
        return;
    }

    std::vector<std::string> chunkFiles;
    int partNumber = 1;
    char* buffer = new char[chunkSize];
    while (!inputFile.eof()) {
        inputFile.read(buffer, chunkSize);
        std::streamsize bytesRead = inputFile.gcount();

        if (bytesRead == 0) {
            break;
        }

        std::ofstream outFile(outputDir + "/part_" + std::to_string(partNumber) + ".chunk", std::ios::binary);
        outFile.write(buffer, bytesRead);
        outFile.close();
        chunkFiles.push_back("part_" + std::to_string(partNumber) + ".chunk");
        partNumber++;
    }

    delete[] buffer;
    inputFile.close();

    // Write metadata to header file
    FileHeader header;
    header.totalChunks = partNumber - 1;
    header.fileName = input;
    header.fileSize = fs::file_size(input);

    // Write header to a text file
    std::ofstream headerFile(outputDir + "/header.txt");
    if (!headerFile) {
        std::cerr << "Exception: Cannot create header file" << std::endl;
        return;
    }
    headerFile << "Total Chunks: " << header.totalChunks << std::endl;
    headerFile << "File Name: " << header.fileName << std::endl;
    headerFile << "File Size: " << header.fileSize << std::endl;
    headerFile.close();

    // Write chunk file names to a text file
    std::ofstream chunkListFile(outputDir + "/chunk_list.txt");
    if (!chunkListFile) {
        std::cerr << "Exception: Cannot create chunk list file" << std::endl;
        return;
    }
    for (const auto& chunkFile : chunkFiles) {
        chunkListFile << chunkFile << std::endl;
    }
    chunkListFile.close();
}

void decode(const std::string& inputDir, const std::string& outputDir) {
    // Read metadata from header file
    FileHeader header;
    std::ifstream headerFile(inputDir + "/header.txt");
    if (!headerFile) {
        std::cerr << "Exception: Cannot open header file" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(headerFile, line)) {
        if (line.find("Total Chunks: ") != std::string::npos) {
            header.totalChunks = std::stoi(line.substr(14));
        } else if (line.find("File Name: ") != std::string::npos) {
            header.fileName = line.substr(11);
        } else if (line.find("File Size: ") != std::string::npos) {
            header.fileSize = std::stol(line.substr(11));
        }
    }
    headerFile.close();

    std::string decodeOutputDir;
    if (outputDir.empty()) {
        decodeOutputDir = fs::current_path().string();
    } else {
        decodeOutputDir = outputDir;
    }

    std::ofstream outputFile(decodeOutputDir + "/" + header.fileName, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Exception: Cannot create output file" << std::endl;
        return;
    }

    // Read chunk file names from the chunk list file
    std::ifstream chunkListFile(inputDir + "/chunk_list.txt");
    if (!chunkListFile) {
        std::cerr << "Exception: Cannot open chunk list file" << std::endl;
        return;
    }
    std::string chunkFileName;
    while (std::getline(chunkListFile, chunkFileName)) {
        std::ifstream inFile(inputDir + "/" + chunkFileName, std::ios::binary);
        if (!inFile) {
            std::cerr << "Exception: Cannot open chunk file" << std::endl;
            return;
        }

        // Get size of the file
        inFile.seekg(0, std::ios::end);
        int fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        // Read chunk file and write to output file
        char* buffer = new char[fileSize];
        inFile.read(buffer, fileSize);
        outputFile.write(buffer, fileSize);
        delete[] buffer;
        inFile.close();
    }
    chunkListFile.close();

    outputFile.close();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage for encode: " << argv[0] << " encode <input_file> <output_directory> <chunk_size>" << std::endl;
        std::cerr << "Usage for decode: " << argv[0] << " decode <input_directory> [output_directory]" << std::endl;
        return 1;
    }

    std::string operation = argv[1];
    if (operation != "encode" && operation != "decode") {
        std::cerr << "Invalid operation. Please specify 'encode' or 'decode'." << std::endl;
        return 1;
    }

    if (operation == "encode" && argc != 5) {
        std::cerr << "Usage for encode: " << argv[0] << " encode <input_file> <output_directory> <chunk_size>" << std::endl;
        return 1;
    }

    if (operation == "decode" && argc != 4 && argc != 5) {
        std::cerr << "Usage for decode: " << argv[0] << " decode <input_directory> [output_directory]" << std::endl;
        return 1;
    }

    if (operation == "encode") {
        std::string inputFileName = argv[2];
        std::string outputDirectory = argv[3];
        int chunkSize = std::stoi(argv[4]);

        // Check if the output directory exists, if not, create it
        outputDirectory = createOutputDir(outputDirectory);

        // Encode the file
        encode(inputFileName, outputDirectory, chunkSize);
    } else if (operation == "decode") {
        std::string inputDirectory = argv[2];
        std::string outputDirectory = (argc == 5) ? argv[3] : "";

        // Decode the file
        decode(inputDirectory, outputDirectory);
    }

    return 0;
}
