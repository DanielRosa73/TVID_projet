#include <iostream>

#include "application.h"

// #include "get_info.h"
// #include "imageConverter.h"
// int main() {
//     std::string filename = "/home/david/Desktop/Image/TVID/Projet/TVID_projet/test_images/pgm/bw_numbers/0.pgm";
//     ImageInfo info = readPGM(filename);

//     std::cout << "Width: " << info.width << std::endl;
//     std::cout << "Height: " << info.height << std::endl;
//     std::cout << "Depth: " << info.depth << std::endl;
//     std::cout << "Sampling Mode: " << info.samplingMode << std::endl;

//     std::string outputFilename = "/home/david/Desktop/Image/TVID/Projet/TVID_projet/test_images/ppm/bw_numbers/0.ppm";
//     ConvertPGMtoPPM(filename, outputFilename);

//     return EXIT_SUCCESS;
// }

int main(int arg, char **argv)
{
    Application::Application app;

    app.Run();

    return EXIT_SUCCESS;
}