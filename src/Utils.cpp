#include "Utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

bool LoadImage(const std::string &filename,
               std::vector<uint8_t> &pixels,
               int &width, int &height, int &channels) {
                                   
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return false;
    }

    pixels.assign(data, data + (width * height * channels));
    stbi_image_free(data);

    return true;
}

bool SaveImage(const std::string &filename,
               const std::vector<uint8_t> &pixels,
               int width, int height, int channels) {

    int success = 0;
    std::string ext = filename.substr(filename.find_last_of(".") + 1);

    if (ext == "png") {
        success = stbi_write_png(filename.c_str(), width, height, channels, pixels.data(), width * channels);

    } else if (ext == "bmp") {
        success = stbi_write_bmp(filename.c_str(), width, height, channels, pixels.data());

    } else if (ext == "jpg" || ext == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), width, height, channels, pixels.data(), 90); // Quality set to 90

    } else {
        
        std::cerr << "Unsupported image format: " << filename << std::endl;
        return false;
    }

    if (!success) {
        std::cerr << "Failed to save image: " << filename << std::endl;
        return false;
    }

    return true;
}