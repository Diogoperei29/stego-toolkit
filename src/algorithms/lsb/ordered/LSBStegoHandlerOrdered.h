#ifndef __LSB_STEGO_HANDLER_ORDERED_H_
#define __LSB_STEGO_HANDLER_ORDERED_H_

#include "../LSBStegoHandler.h"
#include "../../../utils/ImageIO.h"

#include <vector>
#include <string>

/**
 * @brief Implements LSB (Least Significant Bit) steganography for images.
 *
 * This handler embeds and extracts data into/from images by modifying
 * the least significant bit of each pixel value. The data is encrypted
 * with AES-256-CBC before embedding for security.
 */
class LSBStegoHandlerOrdered : public LSBStegoHandler {
public:
    /**
     * @brief Embeds data into pixel array using LSB technique.
     * 
     * Format: [32-bit size header | data bits]
     * 
     * @param pixels Pixel data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @param password unused
     * @return Result indicating success or embedding error
     */
    Result<> EmbedMethod(ImageData &imageData,
                         const std::vector<uint8_t> &dataToEmbed,
                         const std::string &password ) override;
    
    /**
     * @brief Extracts data from pixel array using LSB technique.
     * 
     * @param pixels Pixel data to read from
     * @param password unused
     * @return Result containing extracted data or error
     */
    Result<std::vector<uint8_t>> ExtractMethod(const ImageData &imageData,
                                               const std::string &password ) override;

    ~LSBStegoHandlerOrdered() override = default;

};


#endif // __LSB_STEGO_HANDLER_ORDERED_H_
