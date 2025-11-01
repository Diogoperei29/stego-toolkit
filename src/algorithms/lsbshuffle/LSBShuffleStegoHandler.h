#ifndef __LSB_SHUFFLE_STEGO_HANDLER_H_
#define __LSB_SHUFFLE_STEGO_HANDLER_H_

#include "../StegoHandler.h"
#include "../../utils/ImageIO.h"

/**
 * @brief Implements LSB (Least Significant Bit) steganography for images.
 *
 * This handler embeds and extracts data into/from images by modifying
 * the least significant bit of each pixel value. The data is encrypted
 * with AES-256-CBC before embedding for security.
 */
class LSBShuffleStegoHandler : public StegoHandler {
public:
    /**
    * @brief Embeds a file into a cover image using LSB steganography.
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
                   const std::string &password) override;

    /**
    * @brief Extracts a hidden file from a stego image using LSB steganography.
    *
    * @param stegoFile Path to the stego image
    * @param outputFile Path to save the recovered file
    * @param password Password used for AES decryption
    * @return Result indicating success or detailed error
    */
    Result<> Extract(const std::string &stegoFile,
                     const std::string &outputFile,
                     const std::string &password) override;

    static constexpr uint32_t HEADER_SIZE_BITS = 32;
    static constexpr uint32_t HEADER_SIZE_BYTES = 4;
    static constexpr uint32_t MAX_REASONABLE_SIZE = 100 * 1024 * 1024; // 100MB sanity check
    
    /**
     * @brief Calculate LSB steganography capacity in bytes for a given pixel count.
     * 
     * Each pixel can store 1 bit in its LSB. Capacity accounts for the header.
     * 
     * @param pixelCount Total number of pixel values (width * height * channels)
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateLSBCapacity(std::size_t pixelCount);
    
    /**
     * @brief Calculate LSB steganography capacity for an image.
     * 
     * @param image Image data structure
     * @return Maximum bytes that can be embedded using LSB steganography
     */
    static std::size_t CalculateLSBCapacity(const ImageData& image);

    /**
     * @brief Validate image has sufficient LSB capacity for data.
     * 
     * @param pixelCount Total pixel values available
     * @param dataSize Size of data to embed (in bytes)
     * @return Result indicating success or capacity error with details
     */
    static Result<> ValidateLSBCapacity(std::size_t pixelCount, std::size_t dataSize);
    
    /**
     * @brief Embeds data into pixel array using LSB technique.
     * 
     * Format: [32-bit size header | data bits]
     * 
     * @param pixels Pixel data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @return Result indicating success or embedding error
     */
    Result<> EmbedLSB(std::vector<uint8_t> &pixels,
                      const std::vector<uint8_t> &dataToEmbed,
                      const std::string &password );
    
    /**
     * @brief Extracts data from pixel array using LSB technique.
     * 
     * @param pixels Pixel data to read from
     * @return Result containing extracted data or error
     */
    Result<std::vector<uint8_t>> ExtractLSB(const std::vector<uint8_t> &pixels,
                                            const std::string &password );

    ~LSBShuffleStegoHandler() override = default;

};


#endif // __LSB_SHUFFLE_STEGO_HANDLER_H_
