#include "LSBStegoHandler.h"
#include "../../utils/Utils.h"
#include "../../utils/CryptoModule.h"

#include "stb_image.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

bool LSBStegoHandler::Embed(const std::string &coverFile,
                              const std::string &dataFile,
                              const std::string &outputFile,
                              const std::string &password) {

    std::vector<uint8_t> pixels;
    int width, height, channels;

    if (!LoadImage(coverFile, pixels, width, height, channels)) {
        std::cerr << "Failed to load cover image.\n";
        return false;
    }

    // Read file to embed
    std::ifstream inFile(dataFile, std::ios::binary);
    if (!inFile) {
        std::cerr << "Failed to open data file.\n";
        return false;
    }
    std::vector<uint8_t> plainData((std::istreambuf_iterator<char>(inFile)),
                                    std::istreambuf_iterator<char>());

    // Encrypt data
    std::vector<uint8_t> encryptedData;
    if (!CryptoModule::EncryptData(plainData, password, encryptedData)) {
        std::cerr << "Encryption failed.\n";
        return false;
    }

    // Embed encrypted data into pixels
    EmbedLSB(pixels, encryptedData);

    // Save stego image
    if (!SaveImage(outputFile, pixels, width, height, channels)) {
        std::cerr << "Failed to save stego image.\n";
        return false;
    }

    return true;
}


bool LSBStegoHandler::Extract(const std::string &stegoFile,
                                const std::string &outputFile,
                                const std::string &password)
{
    std::vector<uint8_t> pixels;
    int width, height, channels;

    if (!LoadImage(stegoFile, pixels, width, height, channels)) {
        std::cerr << "Failed to load stego image.\n";
        return false;
    }

    // Extract encrypted data
    std::vector<uint8_t> encryptedData;
    ExtractLSB(pixels, encryptedData);

    // Decrypt data
    std::vector<uint8_t> plainData;
    if (!CryptoModule::DecryptData(encryptedData, password, plainData)) {
        std::cerr << "Decryption failed. Wrong password or corrupted data.\n";
        return false;
    }

    // Write recovered file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file.\n";
        return false;
    }
    outFile.write(reinterpret_cast<const char *>(plainData.data()), plainData.size());

    return true;
}


void LSBStegoHandler::EmbedLSB(std::vector<uint8_t> &pixels,
                                 const std::vector<uint8_t> &dataToEmbed) {

    uint32_t dataSize = static_cast<uint32_t>(dataToEmbed.size());
    if (dataSize * 8 + 32 > pixels.size()) {
        std::cerr << "Data too large to embed in the given image." << std::endl;
        return;
    }

    // Embed data size at the start (32 bits)
    for (size_t idx = 0; idx < 32; ++idx) {
        uint8_t bit = (dataSize >> idx) & 1;
        pixels[idx] = (pixels[idx] & 0xFE) | bit; // Set LSB to the size bit
    }
    
    // Embed actual data
    for (size_t byteIdx = 0; byteIdx < dataSize; ++byteIdx) {
        for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
            uint8_t bit = (dataToEmbed[byteIdx] >> bitIdx) & 1;
            pixels[32 + (byteIdx * 8) + bitIdx] =
                (pixels[32 + (byteIdx * 8) + bitIdx] & 0xFE) | bit; // Set LSB to the data bit
        }
    }
}


void LSBStegoHandler::ExtractLSB(const std::vector<uint8_t> &pixels,
                                   std::vector<uint8_t> &extractedData) {
    
    if (pixels.size() < 32) {
        std::cerr << "Image too small to contain embedded data." << std::endl;
        return;
    }

    // Extract data size from the first 32 bits
    uint32_t dataSize = 0;
    for (size_t idx = 0; idx < 32; ++idx) {
        dataSize |= (pixels[idx] & 1) << idx;
    }

    if (dataSize * 8 + 32 > pixels.size()) {
        std::cerr << "Embedded data size exceeds image capacity." << std::endl;
        return;
    }

    extractedData.clear();
    extractedData.resize(dataSize, 0);

    // Extract actual data
    for (size_t byteIdx = 0; byteIdx < dataSize; ++byteIdx) {
        for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
            uint8_t bit = pixels[32 + (byteIdx * 8) + bitIdx] & 1;
            extractedData[byteIdx] |= (bit << bitIdx);
        }
    }
}
