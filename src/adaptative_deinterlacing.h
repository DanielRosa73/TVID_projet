#pragma once

#include "bob.h"


#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>



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
}

