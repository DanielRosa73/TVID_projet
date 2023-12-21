#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

inline void ConvertPGMtoPPM(const std::string &inputFilename, const std::string &outputFilename) {
    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Could not open the input file." << std::endl;
        return;
    }

    std::ofstream output(outputFilename, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Could not open the output file." << std::endl;
        return;
    }

    std::string line;
    std::getline(input, line);
    if (line != "P5") {
        std::cerr << "Unsupported image format!" << std::endl;
        return;
    }

    std::getline(input, line);
    while (line[0] == '#') {
        std::getline(input, line);
    }

    std::istringstream iss(line);
    int width, height, maxval;
    iss >> width >> height;
    input >> maxval;
    input.get();

    std::vector<unsigned char> image(width * height);

    input.read(reinterpret_cast<char*>(image.data()), image.size());

    int yHeight = (2 * height) / 3;
    int uvHeight = height / 3;
    int uvWidth = width / 2;

    output << "P6\n" << width << " " << yHeight << "\n" << maxval << "\n";

    std::vector<unsigned char> colorImage(width * yHeight * 3);

    // Initialize the color image with the Y U V components
    std::vector<unsigned char> yComponent(image.begin(), image.begin() + yHeight * width);
    std::vector<unsigned char> uComponent(image.begin() + yHeight * width, image.begin() + yHeight * width + uvHeight * uvWidth);
    std::vector<unsigned char> vComponent(image.begin() + yHeight * width + uvHeight * uvWidth, image.end());

    // Fill the components with the correct values
    for (int i = 0; i < yHeight; i++)
    {
        for (int j = 0; j < width; j++)
        {
            yComponent[i * width + j] = image[i * width + j];
        }
    }

    for (int i = 0; i < uvHeight; i++)
    {
        for (int j = 0; j < uvWidth; j++)
        {
            uComponent[i * uvWidth + j] = image[yHeight * width + i * width + j];
            vComponent[i * uvWidth + j] = image[yHeight * width + i * width + uvWidth + j];
        }
    }

    // Convert the Y U V components to RGB
    std::vector<unsigned char> fullU(yHeight * width);
    std::vector<unsigned char> fullV(yHeight * width);

    for (int y = 0; y < yHeight; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int uvIndex = (y / 2) * uvWidth + (x / 2);
            fullU[y * width + x] = uComponent[uvIndex];
            fullV[y * width + x] = vComponent[uvIndex];
        }
    }

    for (int i = 0; i < yHeight * width; i++)
    {
        int y = yComponent[i];
        int u = fullU[i] - 128;
        int v = fullV[i] - 128;

        int r = y + 1.402 * v;
        int g = y - 0.34414 * u - 0.71414 * v;
        int b = y + 1.772 * u;

        r = std::min(255, std::max(0, r));
        g = std::min(255, std::max(0, g));
        b = std::min(255, std::max(0, b));

        colorImage[i * 3] = r;
        colorImage[i * 3 + 1] = g;
        colorImage[i * 3 + 2] = b;
    }

    output.write(reinterpret_cast<char*>(colorImage.data()), colorImage.size());

    if (input.bad()) {
        std::cerr << "An I/O error occurred while reading the input file." << std::endl;
        return;
    }
    if (output.bad()) {
        std::cerr << "An I/O error occurred while writing the output file." << std::endl;
        return;
    }
}
