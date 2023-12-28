#pragma once

#include "bob.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

struct AltOutput {
    std::string frame1;
    std::string frame2;
};

inline bool is_zone_in_motion(const std::vector<unsigned char> image, int field_index, int zone_index, int width, int zone_size, int movement_threshold)
{
    int sum_error = 0;
    #pragma omp parallel for
    for (int i = 0; i < zone_size; i++)
    {
        int x = zone_index * zone_size + i;
        if (x >= width)
            break;
        sum_error += std::pow(image[(field_index * width + x) * 3] - image[((field_index + 1)* width + x) * 3], 2);
        sum_error += std::pow(image[(field_index * width + x) * 3 + 1] - image[((field_index + 1)* width + x) * 3 + 1], 2);
        sum_error += std::pow(image[(field_index * width + x) * 3 + 2] - image[((field_index + 1)* width + x) * 3 + 2] , 2);
    }
    return sum_error >= movement_threshold;
}

inline AltOutput alt_deinterlacing(const std::string& inputFilename, const AltOutput& output, bool tff, bool rff, int zone_size, int movement_threshold)
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
        int nb_zone = width % zone_size ? (width / zone_size + 1) : width / zone_size;
        for (int zone_index = 0; zone_index < nb_zone; zone_index++ )
        {
            bool in_motion = is_zone_in_motion(image, i, zone_index, width, zone_size, movement_threshold);
            for (int j = 0; j < zone_size * 3; j++)
            {
                if (zone_index * zone_size * 3 + j >= width * 3)
                    break;

                if (in_motion)
                {
                    if (i % 2 == 0)
                    {
                        frame1[(i * width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                        frame1[((i + 1)* width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                    }
                    else if (i % 2 == 1 && i != height - 1)
                    {
                        frame2[(i * width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                        frame2[((i + 1)* width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                    }
                }
                else
                {
                    if (i % 2 == 0)
                    {
                        frame1[(i * width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                        frame1[((i + 1)* width + zone_index * zone_size)* 3 + j] = image[((i + 1)* width + zone_index * zone_size)* 3 + j];
                    }
                    else if (i % 2 == 1 && i != height - 1)
                    {
                        frame2[(i * width + zone_index * zone_size)* 3 + j] = image[(i * width + zone_index * zone_size)* 3 + j];
                        frame2[((i + 1)* width + zone_index * zone_size)* 3 + j] = image[((i + 1)* width + zone_index * zone_size)* 3 + j];
                    }   
                }
            }
        }
    }

    if (!tff)
    {
        frame1.swap(frame2);
    }

    output_frame1.write(reinterpret_cast<char*>(frame1.data()), frame1.size());
    if (rff)
        output_frame2.write(reinterpret_cast<char*>(frame1.data()), frame1.size());
    else
        output_frame2.write(reinterpret_cast<char*>(frame2.data()), frame2.size());

    output_frame2.close();
    output_frame1.close();

    input.close();

    return output;
}