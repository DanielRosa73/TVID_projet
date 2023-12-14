#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <string>

class ImageConverter {
public:
    void convertPGMtoPPM(const std::string& inputPath, const std::string& outputPath);
private:
};

#endif // IMAGECONVERTER_H
