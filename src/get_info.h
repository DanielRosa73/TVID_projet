#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

struct ImageInfo {
    int width;
    int height;
    int depth;
    std::string samplingMode;
};

ImageInfo readPGM(const std::string& filename) {
    std::ifstream file(filename);
    ImageInfo info;
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return info;
    }

    std::getline(file, line);
    if (line != "P2" && line != "P5") {
        std::cerr << "Unsupported PGM format" << std::endl;
        return info;
    }

    while (file.peek() == '#') {
        std::getline(file, line);
    }

    file >> info.width >> info.height >> info.depth;
    info.samplingMode = "Grayscale";

    file.close();
    return info;
}