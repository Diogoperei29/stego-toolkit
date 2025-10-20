#ifndef __STEGO_HANDLER_H_
#define __STEGO_HANDLER_H_

#include <string>
#include <vector>
#include <cstdint>
#include "../utils/ErrorHandler.h"

/**
 * @brief Abstract base class for all steganography handlers.
 * 
 * Provides a common interface for embedding and extracting data
 * from various media types using different algorithms.
 */
class StegoHandler {
public:
    /**
     * @brief Embed data into a cover file.
     * 
     * @param coverFile Path to the input cover file
     * @param dataFile Path to the data to embed
     * @param outputFile Path to save the stego file
     * @param password Password for encryption
     * @return Result indicating success or detailed error
     */
    virtual Result<> Embed(const std::string &coverFile,
                           const std::string &dataFile,
                           const std::string &outputFile,
                           const std::string &password) = 0;

    /**
     * @brief Extract data from a stego file.
     * 
     * @param stegoFile Path to the stego file
     * @param outputFile Path to save the extracted data
     * @param password Password for decryption
     * @return Result indicating success or detailed error
     */
    virtual Result<> Extract(const std::string &stegoFile,
                             const std::string &outputFile,
                             const std::string &password) = 0;

    virtual ~StegoHandler() = default;
};


#endif // __STEGO_HANDLER_H_