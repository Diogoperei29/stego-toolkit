#ifndef __LSB_SHUFFLE_STEGO_HANDLER_H_
#define __LSB_SHUFFLE_STEGO_HANDLER_H_

#include "../StegoHandler.h"
#include "../../utils/ImageIO.h"

/**
 * @brief Implements LSB (Least Significant Bit) steganography for images with password-based pixel shuffling.
 *
 * This handler embeds and extracts data into/from images by modifying
 * the least significant bit of each pixel value. Before embedding, the pixel order is shuffled
 * using a pseudorandom sequence derived from the provided password, making extraction dependent
 * on the correct password. The data is also encrypted with AES-256-CBC before embedding for security.
 */
class LSBShuffleStegoHandler : public StegoHandler {
public:
    /**
    * Size of the header for steganography decoding
    **/
    static constexpr uint32_t HEADER_SIZE_BITS = 32;  
    static constexpr uint32_t HEADER_SIZE_BYTES = 4;
    
    /**
     * @brief Embeds data into pixel array using LSB Shuffled technique.
     * 
     * Format: [32-bit size header | data bits]
     * 
     * @param pixels Pixel data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @return Result indicating success or embedding error
     */
    Result<> EmbedMethod(std::vector<uint8_t> &pixels,
                      const std::vector<uint8_t> &dataToEmbed,
                      const std::string &password ) override;
    
    /**
     * @brief Extracts data from pixel array using LSB Shuffled technique.
     * 
     * @param pixels Pixel data to read from
     * @return Result containing extracted data or error
     */
    Result<std::vector<uint8_t>> ExtractMethod(const std::vector<uint8_t> &pixels,
                                            const std::string &password ) override;
                                            

    ~LSBShuffleStegoHandler() override = default;

};

#endif // __LSB_SHUFFLE_STEGO_HANDLER_H_
