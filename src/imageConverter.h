#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

void ConvertPGMtoPPM(const std::string &inputFilename, const std::string &outputFilename) {
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

    output << "P6\n" << width << " " << height << "\n" << maxval << "\n";

    std::vector<unsigned char> image(width * height);
    std::vector<unsigned char> colorImage(width * height * 3);

    input.read(reinterpret_cast<char*>(image.data()), image.size());

    int yHeight = (2 * height) / 3;
    int uvHeight = height / 3;
    int uvWidth = width / 2;

    // Initialize the color image with the Y U V components
    std::vector<unsigned char> yComponent(image.begin(), image.begin() + yHeight * width);
    std::vector<unsigned char> uComponent(image.begin() + yHeight * width, image.begin() + yHeight * width + uvHeight * uvWidth);
    std::vector<unsigned char> vComponent(image.begin() + yHeight * width + uvHeight * uvWidth, image.end());

    std::cout << "Y component size: " << yComponent.size() << std::endl;
    std::cout << "U component size: " << uComponent.size() << std::endl;
    std::cout << "V component size: " << vComponent.size() << std::endl;

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
    std::vector<unsigned char> fullY(height * width);
    std::vector<unsigned char> fullU(height * width);
    std::vector<unsigned char> fullV(height * width);

    for (int y = 0; y < height; y++)
    {
        int correspondingY = (y * yHeight) / height;
        for (int x = 0; x < width; x++)
        {
            fullY[y * width + x] = yComponent[correspondingY * width + x];
        }
    }

    for (int y = 0; y < height; ++y)
    {
        int correspondingU = (y * uvHeight) / height;
        int correspondingV = (y * uvHeight) / height;

        for (int x = 0; x < width; ++x)
        {
            fullU[y * width + x] = uComponent[correspondingU + (x / 2)];
            fullV[y * width + x] = vComponent[correspondingV + (x / 2)];
        }
    }

    for (int i = 0; i < height * width; i++)
    {
        int y = fullY[i];
        int u = fullU[i];
        int v = fullV[i];

        int r = y + v * (1 - 0.299);
        int g = y - u * (1 - 0.114) * 0.114 / 0.587 - v * (1 - 0.299) * 0.299 / 0.587;
        int b = y + u * (1 - 0.114);

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
