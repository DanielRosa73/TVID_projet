#pragma once

#include "bob.h"


#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <omp.h>

struct AltOutput {
    std::string frame1;
    std::string frame2;
};


const int MOVEMENT_THRESHOLD = 255;
const int ZONE_SIZE = 10;



inline bool is_zone_in_motion(const std::vector<unsigned char> image, int field_index, int zone_index, int width)
{
    int sum_error = 0;
    #pragma omp parallel for
    for (int i = 0; i < ZONE_SIZE; i++)
    {
        int x = zone_index * ZONE_SIZE + i;
        if (x >= width)
            break;
        sum_error += std::pow(image[(field_index * width + x) * 3] - image[((field_index + 1)* width + x) * 3], 2);
        sum_error += std::pow(image[(field_index * width + x) * 3 + 1] - image[((field_index + 1)* width + x) * 3 + 1], 2);
        sum_error += std::pow(image[(field_index * width + x) * 3 + 2] - image[((field_index + 1)* width + x) * 3 + 2] , 2);
    }
    return sum_error >= MOVEMENT_THRESHOLD;
}

inline AltOutput alt_deinterlacing(const std::string& inputFilename, const AltOutput& output, bool tff, bool rff)
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

    #pragma omp parallel for
    for (int i = 0; i < height; i++)
    {
        int nb_zone = width % ZONE_SIZE ? (width / ZONE_SIZE + 1) : width / ZONE_SIZE ;
        for (int zone_index = 0; zone_index < nb_zone; zone_index++ )
        {
            bool in_motion = is_zone_in_motion(image, i, zone_index, width);
            for (int j = 0; j < ZONE_SIZE * 3; j++)
            {
                if (zone_index * ZONE_SIZE * 3 + j >= width * 3)
                    break;
                
                if (in_motion)
                {
                    if (i % 2 == 0)
                    {
                        frame1[(i * width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                        frame1[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                    }
                    else if (i % 2 == 1 && i != height - 1)
                    {
                        frame2[(i * width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                        frame2[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                    }
                }
                else
                {
                    if (i % 2 == 0)
                    {
                        frame1[(i * width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                        frame1[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j] = image[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j];
                    }
                    else if (i % 2 == 1 && i != height - 1)
                    {
                        frame2[(i * width + zone_index * ZONE_SIZE)* 3 + j] = image[(i * width + zone_index * ZONE_SIZE)* 3 + j];
                        frame2[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j] = image[((i + 1)* width + zone_index * ZONE_SIZE)* 3 + j];
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

/*
struct Image {
    int width;
    int height;
    int maxval;
    std::vector<unsigned char> data; 
};

//zone in an image.
struct Zone {
    int startX;
    int startY;
    int width;
    int height;
};

inline void bob_deinterlacing_zone(std::vector<unsigned char>& image, const Zone& zone, int width, int height, bool tff, bool rff, std::vector<unsigned char>& frame1, std::vector<unsigned char>& frame2)
{
    for (int i = zone.startY; i < std::min(zone.startY + zone.height, height); i++)
    {
        for (int j = zone.startX * 3; j < std::min(zone.startX + zone.width, width) * 3; j++)
        {
            if (i % 2 == 0)
            {
                frame1[i * width * 3 + j] = image[i * width * 3 + j];
                if (i + 1 < height) {
                    frame1[(i + 1) * width * 3 + j] = image[i * width * 3 + j];
                }
            }
            else if (i % 2 == 1 && i != height - 1)
            {
                frame2[i * width * 3 + j] = image[i * width * 3 + j];
                frame2[(i + 1) * width * 3 + j] = image[i * width * 3 + j];
            }
        }
    }

    if (!tff)
    {
        frame1.swap(frame2);
    }
}


inline std::vector<Zone> divideIntoZones(const Image& image, int zoneWidth, int zoneHeight) {
    std::vector<Zone> zones;
    for (int y = 0; y < image.height; y += zoneHeight) {
        for (int x = 0; x < image.width; x += zoneWidth) {
            Zone zone = {x, y, zoneWidth, zoneHeight};
            zones.push_back(zone);
        }
    }
    return zones;
}

inline bool isZoneInMotion(const Image& currentField, const Image& previousField, 
                    const Zone& zone, int motionThreshold) {
    int motionSum = 0;
    for (int y = zone.startY; y < zone.startY + zone.height; ++y) {
        for (int x = zone.startX; x < zone.startX + zone.width; ++x) {
            int index = (y * currentField.width + x) * 3; 
            int diff = 0;
            for (int i = 0; i < 3; ++i) { //difference entre les pixels currentField et previousFields
                diff += std::abs(currentField.data[index + i] - previousField.data[index + i]);
            }
            motionSum += diff;
        }
    }
    return motionSum > motionThreshold;
}


inline void applyWeaveDeinterlacing(Image& output, const Image& currentField, 
                             const Image& previousField, const Zone& zone) {
  
    int zoneEndX = std::min(zone.startX + zone.width, currentField.width);
    int zoneEndY = std::min(zone.startY + zone.height, currentField.height);


    for (int y = zone.startY; y < zoneEndY; ++y) {
        for (int x = zone.startX; x < zoneEndX; ++x) {
            int index = (y * currentField.width + x) * 3;

            for (int i = 0; i < 3; ++i) {
                output.data[index + i] = (currentField.data[index + i] + previousField.data[index + i]) / 2;
            }
        }
    }
}


inline Image adaptiveSpatialDeinterlacing(const std::string& currentFieldFilename, 
                                   const std::string& previousFieldFilename,
                                   int zoneWidth, int zoneHeight,
                                   int motionThreshold) {
   
    std::ifstream currentFieldInput(currentFieldFilename, std::ios::binary);
    if (!currentFieldInput.is_open()) {
        throw std::runtime_error("Could not open the current field input file.");
    }

    std::string line;
    std::getline(currentFieldInput, line);
    if (line != "P6") {
        throw std::runtime_error("Unsupported image format for current field!");
    }

    std::getline(currentFieldInput, line);
    while (line[0] == '#') {
        std::getline(currentFieldInput, line);
    }

    std::istringstream iss(line);
    int width, height, maxval;
    iss >> width >> height >> maxval;
    currentFieldInput.ignore();

    std::vector<unsigned char> currentImageData(width * height * 3);
    currentFieldInput.read(reinterpret_cast<char*>(currentImageData.data()), currentImageData.size());

    Image currentField{width, height, maxval, std::move(currentImageData)};

    std::ifstream previousFieldInput(previousFieldFilename, std::ios::binary);
    if (!previousFieldInput.is_open()) {
        throw std::runtime_error("Could not open the previous field input file.");
    }

    std::getline(previousFieldInput, line);
    if (line != "P6") {
        throw std::runtime_error("Unsupported image format for previous field!");
    }

    std::getline(previousFieldInput, line);
    while (line[0] == '#') {
        std::getline(previousFieldInput, line);
    }

    iss.clear();
    iss.str(line);
    iss >> width >> height >> maxval;
    previousFieldInput.ignore();

    std::vector<unsigned char> previousImageData(width * height * 3);
    previousFieldInput.read(reinterpret_cast<char*>(previousImageData.data()), previousImageData.size());

    Image previousField{width, height, maxval, std::move(previousImageData)};


    Image outputField = currentField;

    bool tff = true; // placeholder jsp comment set
    bool rff = true; // placeholder jsp comment set

    std::vector<unsigned char> frame1(width * height * 3, 0);
    std::vector<unsigned char> frame2(width * height * 3, 0);

    // Divide into zones and process each zone
    std::vector<Zone> zones = divideIntoZones(currentField, zoneWidth, zoneHeight);

    for (const Zone& zone : zones) {
        if (isZoneInMotion(currentField, previousField, zone, motionThreshold)) {
            bob_deinterlacing_zone(currentField.data, zone, width, height, tff, rff, frame1, frame2);
        } else {
            applyWeaveDeinterlacing(outputField, currentField, previousField, zone);
        }
    }
    // Combine frame1 and frame2 with outputField
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width * 3; j++) {
            int index = i * width * 3 + j;
            if (i % 2 == 0) {
                // For even lines, check if the pixel was updated by weave deinterlacing
                if (outputField.data[index] == currentField.data[index]) {
                    // If not updated by weave, use frame1's data
                    outputField.data[index] = frame1[index];
                }
            } else {
                // For odd lines, check if the pixel was updated by weave deinterlacing
                if (outputField.data[index] == currentField.data[index]) {
                    // If not updated by weave, use frame2's data
                    outputField.data[index] = frame2[index];
                }
            }
        }
    }
}*/

