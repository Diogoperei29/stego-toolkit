#ifndef LSB_STEGO_HANDLER_H
#define LSB_STEGO_HANDLER_H

#include "../StegoHandler.h"

/**
 * @brief Implements LSB (Least Significant Bit) steganography for images.
 *
 * This handler embeds and extracts data into/from images by modifying
 * the least significant bit of each pixel value. The data is encrypted
 * with AES-256-CBC before embedding for security.
 */
class LSBStegoHandler : public StegoHandler {
public:
    /**
    * @brief Embeds a file into a cover image using LSB steganography.
    *
    * Loads the cover image, encrypts the data file with AES-256-CBC,
    * and embeds the encrypted data into the image pixels using LSB.
    *
    * @param coverFile Path to the input cover image.
    * @param dataFile Path to the file to embed.
    * @param outputFile Path to save the stego image.
    * @param password Password used for AES encryption.
    * @return true if embedding succeeded, false otherwise.
    */
    bool Embed(const std::string &coverFile,
               const std::string &dataFile,
               const std::string &outputFile,
               const std::string &password) override;

    /**
    * @brief Extracts a hidden file from a stego image using LSB steganography.
    *
    * Loads the stego image, extracts the encrypted payload using LSB,
    * and decrypts it with AES-256-CBC using the provided password.
    *
    * @param stegoFile Path to the stego image.
    * @param outputFile Path to save the recovered file.
    * @param password Password used for AES decryption.
    * @return true if extraction succeeded, false otherwise.
    */
    bool Extract(const std::string &stegoFile,
                 const std::string &outputFile,
                 const std::string &password) override;

    ~LSBStegoHandler() override = default;

    
private:
    /**
     * @brief Embeds data into pixel array using LSB technique.
     * 
     * @param pixels Pixel data to modify (in-place).
     * @param dataToEmbed Data to embed (already encrypted).
     */
    void EmbedLSB(std::vector<uint8_t> &pixels,
                  const std::vector<uint8_t> &dataToEmbed);
    
    /**
     * @brief Extracts data from pixel array using LSB technique.
     * 
     * @param pixels Pixel data to read from.
     * @param extractedData Output buffer for extracted data.
     */
    void ExtractLSB(const std::vector<uint8_t> &pixels,
                    std::vector<uint8_t> &extractedData);

};

#endif // LSB_STEGO_HANDLER_H
