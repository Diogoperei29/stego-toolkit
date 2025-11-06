#ifndef __IMAGE_IO_H_
#define __IMAGE_IO_H_

#include <vector>
#include <string>
#include <cstdint>
#include "ErrorHandler.h"

// Represents image data with metadata.
struct ImageData {
public:
    std::vector<uint8_t> pixels;
    int width;
    int height;
    int channels;

    ImageData() = default;

    explicit ImageData( std::vector<uint8_t> pixels, int width, int height, int channels )
        : pixels (pixels),
          width (width),
          height (height),
          channels (channels) 
    {   }
    
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

    /**
     * @brief Get pixel indexes in spiral order (clockwise)
     * Spiral: Top-Left -> Top-Right -> Bottom-Right -> Bottom-Left (spiral clockwise)
     * 
     * @return indexes vector into the flat pixels vector for spiral access pattern.
     */
    std::vector<std::size_t> GetPixelsIndexesInSpiral() {
        std::vector<std::size_t> spiralPixels;
        spiralPixels.reserve(static_cast<std::size_t>(width * height));
        
        int start_x = 0, start_y = 0;
        int end_x = width - 1;
        int end_y = height - 1;

        while (start_x <= end_x && start_y <= end_y) { 
            // Top edge: Left to Right
            for (int x = start_x; x <= end_x; ++x)
                spiralPixels.push_back(static_cast<std::size_t>(x + start_y * width));
            start_y++;
            
            // Right edge: Top to Bottom
            for (int y = start_y; y <= end_y; ++y) 
                spiralPixels.push_back(static_cast<std::size_t>(end_x + y * width));
            end_x--;
            
            // Bottom edge: Right to Left (if there's a row)
            if (start_y <= end_y) {
                for (int x = end_x; x >= start_x; --x)
                    spiralPixels.push_back(static_cast<std::size_t>(x + end_y * width));
                end_y--;
            }

            // Left edge: Bottom to Top (if there's a column)
            if (start_x <= end_x) {
                for (int y = end_y; y >= start_y; --y)
                    spiralPixels.push_back(static_cast<std::size_t>(start_x + y * width));
                start_x++;
            }
        }

        return spiralPixels;
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
