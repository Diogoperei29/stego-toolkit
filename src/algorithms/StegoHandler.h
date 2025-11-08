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
     * @param imageData Image data to modify (in-place)
     * @param dataToEmbed Data to embed (already encrypted)
     * @param password Password that may be used in Extraction
     * @return Result indicating success or embedding error
     */
    virtual Result<> EmbedMethod(ImageData &imageData,
                                 const std::vector<uint8_t> &dataToEmbed,
                                 const std::string &password ) = 0;


    /**
     * @brief Visualizes data stored into pixel array using a Stego technique.
     * 
     * @param imageData Pixel data to read from
     * @return Result indicating success or error
     */
    virtual Result<> VisualizeMethod(ImageData &imageData) = 0;
    
    /**
     * @brief Extracts data from pixel array using technique from subclass.
     * 
     * @param imageData Image data to read from
     * @param password Password that may be used in Extraction
     * @return Result containing extracted data (encrypted) or error
     */
    virtual Result<std::vector<uint8_t>> ExtractMethod(const ImageData &imageData,
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
    * @brief Visualizes the embedding of a file into a cover image using steganography.
    *
    * @param coverFile Path to the input cover image
    * @param dataFile Path to the file to embed
    * @param outputFile Path to save the stego image
    * @param password Password used for AES encryption
    * @return Result indicating success or detailed error
    */
    Result<> Visual(const std::string &coverFile,
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

    virtual ~StegoHandler() = default;

};


#endif // __STEGO_HANDLER_H_