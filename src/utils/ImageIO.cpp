#include "ImageIO.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <sstream>
#include <algorithm>
#include <cctype>

Result<ImageData> ImageIO::Load(const std::string &filename) {
    int width = 0, height = 0, channels = 0;
    
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        const char* stbError = stbi_failure_reason();
        std::ostringstream oss;
        oss << "Failed to load image '" << filename << "'. ";
        if (stbError) {
            oss << "Reason: " << stbError;
        } else {
            oss << "File may not exist or format is unsupported.";
        }
        return Result<ImageData>(ErrorCode::ImageLoadFailed, oss.str());
    }
    
    if (width <= 0 || height <= 0 || channels <= 0) {
        stbi_image_free(data);
        std::ostringstream oss;
        oss << "Image loaded but has invalid dimensions: " 
            << width << "x" << height << "x" << channels;
        return Result<ImageData>(ErrorCode::InvalidImageDimensions, oss.str());
    }
    
    ImageData imageData;
    imageData.width = width;
    imageData.height = height;
    imageData.channels = channels;
    
    std::size_t dataSize = static_cast<std::size_t>(width) * height * channels;
    imageData.pixels.assign(data, data + dataSize);
    
    stbi_image_free(data);
    
    return Result<ImageData>(imageData);
}

Result<> ImageIO::Save(const std::string &filename, const ImageData &data) {
    return Save(filename, data.pixels, data.width, data.height, data.channels);
}

Result<> ImageIO::Save(const std::string &filename,
                       const std::vector<uint8_t> &pixels,
                       int width, int height, int channels) {
    
    if (pixels.empty()) {
        return Result<>(ErrorCode::InvalidArgument, "Cannot save image: pixel data is empty");
    }
    
    if (width <= 0 || height <= 0 || channels <= 0) {
        std::ostringstream oss;
        oss << "Cannot save image: invalid dimensions " 
            << width << "x" << height << "x" << channels;
        return Result<>(ErrorCode::InvalidImageDimensions, oss.str());
    }
    
    std::size_t expectedSize = static_cast<std::size_t>(width) * height * channels;
    if (pixels.size() != expectedSize) {
        std::ostringstream oss;
        oss << "Cannot save image: pixel data size mismatch. "
            << "Expected " << expectedSize << " bytes, got " << pixels.size() << " bytes";
        return Result<>(ErrorCode::ImageCorrupted, oss.str());
    }
    
    std::string ext = GetExtension(filename);
    if (ext.empty()) {
        return Result<>(
            ErrorCode::UnsupportedImageFormat,
            "Cannot save image: no file extension specified in '" + filename + "'"
        );
    }
    
    if (!IsSupportedFormat(filename)) {
        return Result<>(
            ErrorCode::UnsupportedImageFormat,
            "Unsupported image format '" + ext + "'. Supported formats: PNG, BMP, JPG/JPEG"
        );
    }
    
    int success = 0;
    
    if (ext == "png") {
        success = stbi_write_png(filename.c_str(), width, height, channels, 
                                 pixels.data(), width * channels);
    } 
    else if (ext == "bmp") {
        success = stbi_write_bmp(filename.c_str(), width, height, channels, pixels.data());
    } 
    else if (ext == "jpg" || ext == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), width, height, channels, 
                                 pixels.data(), JPEG_QUALITY);
    }
    
    if (!success) {
        return Result<>(
            ErrorCode::ImageSaveFailed,
            "Failed to save image to '" + filename + "'. Check write permissions and disk space."
        );
    }
    
    return Result<>();
}

bool ImageIO::IsSupportedFormat(const std::string &filename) {
    std::string ext = GetExtension(filename);
    return ext == "png" || ext == "bmp" || ext == "jpg" || ext == "jpeg";
}

std::string ImageIO::GetExtension(const std::string &filename) {
    std::size_t dotPos = filename.find_last_of(".");
    if (dotPos == std::string::npos || dotPos == filename.length() - 1) {
        return "";
    }
    
    std::string ext = filename.substr(dotPos + 1);
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    return ext;
}
