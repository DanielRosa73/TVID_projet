#include "ImageConverter.h"

int main() {
    std::string inputFilePath = "test_images/pgm/3.pgm";
    std::string outputFilePath = "test_images/ppm/output.ppm";

    ImageConverter converter;
    converter.convertPGMtoPPM(inputFilePath, outputFilePath);

    return 0;
}
