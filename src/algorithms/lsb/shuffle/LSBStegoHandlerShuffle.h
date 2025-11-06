#ifndef __LSB_SHUFFLE_STEGO_HANDLER_H_
#define __LSB_SHUFFLE_STEGO_HANDLER_H_

#include "../LSBStegoHandler.h"
#include "../../../utils/ImageIO.h"

/**
 * @brief Implements LSB (Least Significant Bit) steganography for images with password-based pixel shuffling.
 *
 * This handler embeds and extracts data into/from images by modifying
 * the least significant bit of each pixel value. Before embedding, the pixel order is shuffled
 * using a pseudorandom sequence derived from the provided password, making extraction dependent
 * on the correct password. The data is also encrypted with AES-256-CBC before embedding for security.
 */
class LSBStegoHandlerShuffle : public LSBStegoHandler {
public:
    /**
     * @brief Embeds data into pixel array using LSB Shuffled technique.
     * 
     * Format: [32-bit size header | data bits]
     * 
     * @param pixels Pixel data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @param password Password used to shuffle the data
     * @return Result indicating success or embedding error
     */
    Result<> EmbedMethod(ImageData &imageData,
                         const std::vector<uint8_t> &dataToEmbed,
                         const std::string &password ) override;
    
    /**
     * @brief Extracts data from pixel array using LSB Shuffled technique.
     * 
     * @param pixels Pixel data to read from
     * @param password Password used to unshuffle the data
     * @return Result containing extracted data or error
     */
    Result<std::vector<uint8_t>> ExtractMethod(const ImageData &imageData,
                                               const std::string &password ) override;     

    ~LSBStegoHandlerShuffle() override = default;

};

#endif // __LSB_SHUFFLE_STEGO_HANDLER_H_
