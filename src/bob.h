#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

struct BobOutput {
    std::string frame1;
    std::string frame2;
};

inline BobOutput bob_deinterlacing(const std::string& inputFilename, const BobOutput& output)
{
    std::ifstream input(inputFilename, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Could not open the input file." << std::endl;
        return output;
    }

    std::string line;
    std::getline(input, line);
    if (line != "P6") {
        std::cerr << "Unsupported image format!" << std::endl;
        return output;
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

    std::vector<unsigned char> image(width * height * 3);
    input.read(reinterpret_cast<char*>(image.data()), image.size());

    std::vector<unsigned char> frame1(width * height * 3);
    std::vector<unsigned char> frame2(width * height * 3);

    std::ofstream output_frame1(output.frame1, std::ios::binary);
    if (!output_frame1.is_open()) {
        std::cerr << "Could not open the output frame 1 file." << std::endl;
        return output;
    }

    std::ofstream output_frame2(output.frame2, std::ios::binary);
    if (!output_frame2.is_open()) {
        std::cerr << "Could not open the output frame 2 file." << std::endl;
        return output;
    }

    output_frame1 << "P6\n" << width << " " << height << "\n" << maxval << "\n";
    output_frame2 << "P6\n" << width << " " << height << "\n" << maxval << "\n";

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width * 3; j++)
        {
            if (i % 2 == 0)
            {
                frame1[i * width * 3 + j] = image[i * width * 3 + j];
                frame1[(i + 1) * width * 3 + j] = image[i * width * 3 + j];
            }
            else
            {
                frame2[i * width * 3 + j] = image[i * width * 3 + j];
                frame2[(i - 1) * width * 3 + j] = image[i * width * 3 + j];
            }
        }
    }

    output_frame1.write(reinterpret_cast<char*>(frame1.data()), frame1.size());
    output_frame2.write(reinterpret_cast<char*>(frame2.data()), frame2.size());

    output_frame2.close();
    output_frame1.close();

    input.close();

    return output;
}