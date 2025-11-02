#ifndef __STEGO_HANDLER_H_
#define __STEGO_HANDLER_H_

#include <string>
#include <cstdint>
#include "../utils/ErrorHandler.h"
#include "../utils/ImageIO.h"

/**
 * @brief Abstract base class for all steganography handlers.
 * 
 * Provides a common interface for embedding and extracting data
 * from various media types using different algorithms.
 */
class StegoHandler {
public:

    static constexpr uint32_t MAX_REASONABLE_SIZE = 1024 * 1024 * 1024 * 1; // 1GB Max File size sanity check

    /**
     * @brief Embeds data into pixel array using technique from subclass.
     * 
     * Format: [32-bit size header | data bits]
     * 
     * @param pixels Pixel data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @return Result indicating success or embedding error
     */
    virtual Result<> EmbedMethod(std::vector<uint8_t> &pixels,
                      const std::vector<uint8_t> &dataToEmbed,
                      const std::string &password ) = 0;
    
    /**
     * @brief Extracts data from pixel array using technique from subclass.
     * 
     * @param pixels Pixel data to read from
     * @return Result containing extracted data or error
     */
    virtual Result<std::vector<uint8_t>> ExtractMethod(const std::vector<uint8_t> &pixels,
                                            const std::string &password ) = 0;

    /**
    * @brief Embeds a file into a cover image using steganography.
    *
    * @param coverFile Path to the input cover image
    * @param dataFile Path to the file to embed
    * @param outputFile Path to save the stego image
    * @param password Password used for AES encryption
    * @return Result indicating success or detailed error
    */
    Result<> Embed(const std::string &coverFile,
                   const std::string &dataFile,
                   const std::string &outputFile,
                   const std::string &password);

    /**
    * @brief Extracts a hidden file from a stego image using steganography.
    *
    * @param stegoFile Path to the stego image
    * @param outputFile Path to save the recovered file
    * @param password Password used for AES decryption
    * @return Result indicating success or detailed error
    */
    Result<> Extract(const std::string &stegoFile,
                     const std::string &outputFile,
                     const std::string &password);

    /**
     * @brief Calculate LSB steganography capacity in bytes for a given pixel count.
     * 
     * Each pixel can store 1 bit in its LSB. Capacity accounts for the header.
     * 
     * @param pixelCount Total number of pixel values (width * height * channels)
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateCapacity(std::size_t pixelCount , std::size_t headerBits);
    
    /**
     * @brief Calculate LSB steganography capacity for an image.
     * 
     * @param image Image data structure
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateCapacity(const ImageData& image , std::size_t headerBits);

    /**
     * @brief Validate image has sufficient LSB capacity for data.
     * 
     * @param pixelCount Total pixel values available
     * @param dataSize Size of data to embed (in bytes)
     * @return Result indicating success or capacity error with details
     */
    static Result<> ValidateCapacity(std::size_t pixelCount, std::size_t dataSize, std::size_t headerBits, std::size_t fileMaxSize);
    

    virtual ~StegoHandler() = default;

};


#endif // __STEGO_HANDLER_H_