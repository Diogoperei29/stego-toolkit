#ifndef __LSB_STEGO_HANDLER_H_
#define __LSB_STEGO_HANDLER_H_

#include "../StegoHandler.h"
#include "../../utils/ImageIO.h"

#include <vector>
#include <string>

/**
 * @brief Abstract base class for LSB (Least Significant Bit) steganography algorithms.
 *
 * This handler provides common functionality for LSB-based steganography methods,
 * including capacity calculation and validation. Concrete implementations define
 * the specific embedding and extraction strategies.
 */
class LSBStegoHandler : public StegoHandler {
public:
    /**
    * Size of the header for steganography decoding
    **/
    static constexpr uint32_t HEADER_SIZE_BITS = 32;  
    static constexpr uint32_t HEADER_SIZE_BYTES = 4;
    
    /**
     * @brief Calculate LSB steganography capacity in bytes for a given pixel count.
     * 
     * Each pixel can store 1 bit in its LSB. Capacity accounts for the header.
     * 
     * @param pixelCount Total number of pixel values (width * height * channels)
     * @param headerBits Size of header in bits
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateCapacity(std::size_t pixelCount, std::size_t headerBits);
    
    /**
     * @brief Calculate LSB steganography capacity for an image.
     * 
     * @param image Image data structure
     * @param headerBits Size of header in bits
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateCapacity(const ImageData& image, std::size_t headerBits);

    /**
     * @brief Validate image has sufficient LSB capacity for data.
     * 
     * @param pixelCount Total pixel values available
     * @param dataSize Size of data to embed (in bytes)
     * @param headerBits Size of header in bits
     * @param fileMaxSize Maximum allowed file size
     * @return Result indicating success or capacity error with details
     */
    static Result<> ValidateCapacity(std::size_t pixelCount, std::size_t dataSize, 
                                     std::size_t headerBits, std::size_t fileMaxSize);

    /**
     * @brief Visualizes data stored into pixel array using LSB technique.
     * 
     * @param imageData Pixel data to read from
     * @return Result indicating success or error
     */
    Result<> VisualizeMethod( ImageData &imageData) override;
    
    virtual ~LSBStegoHandler() = default;
};

#endif // __LSB_STEGO_HANDLER_H_
