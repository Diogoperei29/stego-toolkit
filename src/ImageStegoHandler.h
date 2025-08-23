#ifndef IMAGE_STEGO_HANDLER_H
#define IMAGE_STEGO_HANDLER_H

#include "StegoHandler.h"

/**
 * @brief Handles image-based steganography operations.
 *
 * Provides methods to embed and extract data into/from images
 * using the Least Significant Bit (LSB) method combined with AES encryption.
 */
class ImageStegoHandler : public StegoHandler {
public:
    /**
    * @brief Embeds a file into a cover image.
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
    * @brief Extracts a hidden file from a stego image.
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

    ~ImageStegoHandler() override = default;

    
private:
    void EmbedLSB(std::vector<uint8_t> &pixels,
                  const std::vector<uint8_t> &dataToEmbed);
    void ExtractLSB(const std::vector<uint8_t> &pixels,
                    std::vector<uint8_t> &extractedData);

    void EncryptData(std::vector<uint8_t> &data, const std::string &password);
    void DecryptData(std::vector<uint8_t> &data, const std::string &password);
};

#endif // IMAGE_STEGO_HANDLER_H

