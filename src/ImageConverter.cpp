#include "ImageConverter.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <limits>

void ImageConverter::convertPGMtoPPM(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream inputFile(inputPath, std::ios::binary);
    std::ofstream outputFile(outputPath, std::ios::binary);
    std::string line;
    int width, height, maxVal;

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file: " << inputPath << std::endl;
        return;
    }

    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file: " << outputPath << std::endl;
        return;
    }

    // Read the magic number
    std::getline(inputFile, line);
    if (line != "P5") {
        std::cerr << "Unsupported file format. Only Binary PGM (P5) is supported." << std::endl;
        return;
    }

    // Skip comments
    while (inputFile.peek() == '#') {
        std::getline(inputFile, line);
    }

    inputFile >> width >> height >> maxVal;
    inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Skip to the next line

    // Write PPM header
    outputFile << "P6\n" << width << " " << height << "\n255\n";

    // Read and write pixel data
    std::vector<unsigned char> buffer(width);
    for (int i = 0; i < height; ++i) {
        inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (!inputFile.good()) {
            std::cerr << "Error reading file." << std::endl;
            return;
        }

        for (auto grayValue : buffer) {
            // Since it's grayscale, RGB values are all the same
            for (int j = 0; j < 3; ++j) {
                outputFile << grayValue;
            }
        }
    }

    inputFile.close();
    outputFile.close();
}
