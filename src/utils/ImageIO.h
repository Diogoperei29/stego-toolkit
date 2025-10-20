#ifndef __IMAGE_IO_H_
#define __IMAGE_IO_H_

#include <vector>
#include <string>
#include <cstdint>
#include "ErrorHandler.h"

// Represents image data with metadata.
struct ImageData {
    std::vector<uint8_t> pixels;
    int width;
    int height;
    int channels;
    
    /**
     * @brief Get total number of pixel values (width * height * channels).
     */
    std::size_t GetPixelCount() const {
        return static_cast<std::size_t>(width) * height * channels;
    }
    
    /**
     * @brief Check if image dimensions are valid.
     */
    bool IsValid() const {
        return width > 0 && height > 0 && channels > 0 && !pixels.empty();
    }
};

/**
 * @brief Image I/O operations with comprehensive error handling.
 * 
 * Supports PNG, BMP, and JPEG formats.
 */
class ImageIO {
public:
    /**
     * @brief Load an image file into memory.
     * 
     * @param filename Path to the image file
     * @return Result containing ImageData on success or detailed error
     */
    static Result<ImageData> Load(const std::string &filename);
    
    /**
     * @brief Save image data to a file.
     * 
     * Format is determined by file extension (.png, .bmp, .jpg/.jpeg).
     * 
     * @param filename Output file path
     * @param data Image data to save
     * @return Result indicating success or detailed error
     */
    static Result<> Save(const std::string &filename, const ImageData &data);
    
    /**
     * @brief Save image data to a file
     * 
     * @param filename Output file path
     * @param pixels Pixel data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @return Result indicating success or detailed error
     */
    static Result<> Save(const std::string &filename,
                         const std::vector<uint8_t> &pixels,
                         int width, int height, int channels);
    
                         
    private:
    static constexpr int JPEG_QUALITY = 90;

    static bool IsSupportedFormat(const std::string &filename);
    static std::string GetExtension(const std::string &filename);
    
};


#endif // __IMAGE_IO_H_
