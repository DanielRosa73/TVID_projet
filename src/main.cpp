#include "get_info.h"

int main() {
    std::string filename = "/home/david/Desktop/Image/TVID/Projet/TVID_projet/test_images/pgm/bw_numbers/0.pgm";
    ImageInfo info = readPGM(filename);

    std::cout << "Width: " << info.width << std::endl;
    std::cout << "Height: " << info.height << std::endl;
    std::cout << "Depth: " << info.depth << std::endl;
    std::cout << "Sampling Mode: " << info.samplingMode << std::endl;

    return EXIT_SUCCESS;
}