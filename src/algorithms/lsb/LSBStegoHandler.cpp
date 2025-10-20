#include "LSBStegoHandler.h"
#include "../../utils/ImageIO.h"
#include "../../utils/CryptoModule.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

std::size_t LSBStegoHandler::CalculateLSBCapacity(std::size_t pixelCount) {
    if (pixelCount <= HEADER_SIZE_BITS) {
        return 0;
    }
    // Each pixel stores 1 bit, subtract header, convert to bytes
    return (pixelCount - HEADER_SIZE_BITS) / 8; 
}

std::size_t LSBStegoHandler::CalculateLSBCapacity(const ImageData& image) {
    return CalculateLSBCapacity(image.GetPixelCount());
}

Result<> LSBStegoHandler::ValidateLSBCapacity(std::size_t pixelCount, std::size_t dataSize) {
    std::size_t availableCapacity = CalculateLSBCapacity(pixelCount);
    
    if (dataSize > availableCapacity) {
        std::ostringstream oss;
        oss << "Data size (" << dataSize << " bytes) exceeds LSB capacity (" << availableCapacity << " bytes).\n"
            << "    Image has " << pixelCount << " pixel values.\n"
            << "    You need an image with at least " << ((dataSize * 8) + HEADER_SIZE_BITS) << " pixel values.";
        return Result<>(ErrorCode::InsufficientCapacity, oss.str());
    }
    
    return Result<>();
}

Result<> LSBStegoHandler::Embed(const std::string &coverFile,
                                const std::string &dataFile,
                                const std::string &outputFile,
                                const std::string &password) {
    
    // Load cover image
    auto imageResult = ImageIO::Load(coverFile);
    if (!imageResult) {
        return Result<>(imageResult.GetErrorCode(), imageResult.GetErrorMessage());
    }
    
    auto imageData = imageResult.GetValue();

    // Load data file
    std::ifstream inFile(dataFile, std::ios::binary);
    if (!inFile) {
        return Result<>(
            ErrorCode::FileNotFound,
            "Failed to open data file '" + dataFile + "'"
        );
    }
    
    // Read text in file into a vector
    std::vector<uint8_t> plainData((std::istreambuf_iterator<char>(inFile)),
                                    std::istreambuf_iterator<char>());
    inFile.close();
    
    if (plainData.empty()) {
        return Result<>(
            ErrorCode::InvalidArgument,
            "Data file '" + dataFile + "' is empty. Nothing to embed."
        );
    }

    // Encrypt data
    auto encryptResult = CryptoModule::EncryptData(plainData, password);
    if (!encryptResult) {
        return Result<>(
            encryptResult.GetErrorCode(),
            "Encryption failed: " + encryptResult.GetErrorMessage()
        );
    }
    
    const auto& encryptedData = encryptResult.GetValue();

    // Validate LSB capacity
    auto capacityCheck = ValidateLSBCapacity(imageData.GetPixelCount(), encryptedData.size());
    if (!capacityCheck) {
        return capacityCheck;
    }

    // Embed data into image
    auto embedResult = EmbedLSB(imageData.pixels, encryptedData);
    if (!embedResult) {
        return embedResult;
    }

    // Save stego image
    auto saveResult = ImageIO::Save(outputFile, imageData);
    if (!saveResult) {
        return saveResult;
    }

    return Result<>();
}

Result<> LSBStegoHandler::Extract(const std::string &stegoFile,
                                   const std::string &outputFile,
                                   const std::string &password) {
    
    // Load stego image
    auto imageResult = ImageIO::Load(stegoFile);
    if (!imageResult) {
        return Result<>(imageResult.GetErrorCode(), imageResult.GetErrorMessage());
    }
    
    const auto& imageData = imageResult.GetValue();
    const auto& pixels = imageData.pixels;
    
    // Validate minimum size
    if (pixels.size() < HEADER_SIZE_BITS) {
        std::ostringstream oss;
        oss << "Image too small to contain embedded data. "
            << "Has " << pixels.size() << " pixels, needs at least " << HEADER_SIZE_BITS;
        return Result<>(ErrorCode::ImageTooSmall, oss.str());
    }

    // Extract encrypted data
    auto extractResult = ExtractLSB(pixels);
    if (!extractResult) {
        return Result<>(
            extractResult.GetErrorCode(),
            "Extraction failed: " + extractResult.GetErrorMessage()
        );
    }
    
    const auto& encryptedData = extractResult.GetValue();

    // Decrypt data
    auto decryptResult = CryptoModule::DecryptData(encryptedData, password);
    if (!decryptResult) {
        return Result<>(
            decryptResult.GetErrorCode(),
            "Decryption failed: " + decryptResult.GetErrorMessage()
        );
    }
    
    const auto& plainData = decryptResult.GetValue();

    // Write output file
    std::ofstream outFile(outputFile, std::ios::binary);
    if (!outFile) {
        return Result<>(
            ErrorCode::FileWriteError,
            "Failed to open output file '" + outputFile + "' for writing"
        );
    }
    
    outFile.write(reinterpret_cast<const char *>(plainData.data()), plainData.size());
    
    if (!outFile) {
        return Result<>(
            ErrorCode::FileWriteError,
            "Failed to write data to '" + outputFile + "'"
        );
    }

    outFile.close();
    return Result<>();
}

Result<> LSBStegoHandler::EmbedLSB(std::vector<uint8_t> &pixels,
                                    const std::vector<uint8_t> &dataToEmbed) {
    
    uint32_t dataSize = static_cast<uint32_t>(dataToEmbed.size());
    
    if (dataSize * 8 + HEADER_SIZE_BITS > pixels.size()) {
        std::ostringstream oss;
        oss << "Internal error: Data size " << dataSize << " bytes exceeds available capacity";
        return Result<>(ErrorCode::EmbeddingFailed, oss.str());
    }

    // Embed size header
    for (std::size_t idx = 0; idx < HEADER_SIZE_BITS; ++idx) {
        uint8_t bit = (dataSize >> idx) & 1;
        pixels[idx] = (pixels[idx] & 0xFE) | bit;
    }
    
    // Embed data bits
    for (std::size_t byteIdx = 0; byteIdx < dataSize; ++byteIdx) {
        for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
            uint8_t bit = (dataToEmbed[byteIdx] >> bitIdx) & 1;
            std::size_t pixelIdx = HEADER_SIZE_BITS + (byteIdx * 8) + bitIdx;
            pixels[pixelIdx] = (pixels[pixelIdx] & 0xFE) | bit;
        }
    }

    return Result<>();
}

Result<std::vector<uint8_t>> LSBStegoHandler::ExtractLSB(const std::vector<uint8_t> &pixels) {
    
    // Extract size header
    uint32_t dataSize = 0;
    for (std::size_t idx = 0; idx < HEADER_SIZE_BITS; ++idx) {
        dataSize |= (pixels[idx] & 1) << idx;
    }

    // Validate size
    if (dataSize == 0) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::NoEmbeddedData,
            "Extracted size is 0. Image may not contain embedded data."
        );
    }
    
    if (dataSize > MAX_REASONABLE_SIZE) {
        std::ostringstream oss;
        oss << "Extracted size (" << dataSize << " bytes) is unreasonably large (max " 
            << MAX_REASONABLE_SIZE << " bytes). Data is likely corrupted or password is wrong.";
        return Result<std::vector<uint8_t>>(
            ErrorCode::CorruptedPayload,
            oss.str()
        );
    }
    
    if (dataSize * 8 + HEADER_SIZE_BITS > pixels.size()) {
        std::ostringstream oss;
        oss << "Extracted size (" << dataSize << " bytes) exceeds image capacity. "
            << "Image has " << pixels.size() << " pixel values, "
            << "but would need " << (dataSize * 8 + HEADER_SIZE_BITS) << " values. "
            << "Data is corrupted or password may be wrong.";
        return Result<std::vector<uint8_t>>(
            ErrorCode::InvalidDataSize,
            oss.str()
        );
    }

    // Extract data bits
    std::vector<uint8_t> extractedData(dataSize, 0);
    for (std::size_t byteIdx = 0; byteIdx < dataSize; ++byteIdx) {
        for (int bitIdx = 0; bitIdx < 8; ++bitIdx) {
            std::size_t pixelIdx = HEADER_SIZE_BITS + (byteIdx * 8) + bitIdx;
            uint8_t bit = pixels[pixelIdx] & 1;
            extractedData[byteIdx] |= (bit << bitIdx);
        }
    }

    return Result<std::vector<uint8_t>>(extractedData);
}
