#include "StegoHandler.h"
#include "../utils/ImageIO.h"
#include "../utils/CryptoModule.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

std::size_t StegoHandler::CalculateCapacity(std::size_t pixelCount, std::size_t headerBits) {
    if (pixelCount <= headerBits) {
        return 0;
    }
    // Each pixel stores 1 bit, subtract header, convert to bytes
    return (pixelCount - headerBits) / 8; 
}

std::size_t StegoHandler::CalculateCapacity(const ImageData& image , std::size_t headerBits) {
    return CalculateCapacity(image.GetPixelCount(), headerBits);
}

Result<> StegoHandler::ValidateCapacity(std::size_t pixelCount, std::size_t dataSize , std::size_t headerBits, std::size_t fileMaxSize) {
    if (dataSize > fileMaxSize) {
        std::ostringstream oss;
        oss << "Data size (" << dataSize << " bytes) exceeds maximum allowed size (" 
            << fileMaxSize << " bytes)";
        return Result<>(ErrorCode::DataTooLarge, oss.str());
    }
    
    std::size_t availableCapacity = CalculateCapacity(pixelCount, headerBits);
    
    if (availableCapacity == 0) {
        std::ostringstream oss;
        oss << "Provided image is too small to contain embedded data. "
            << "    Image has " << pixelCount << " pixel values.\n"
            << "    You need an image with at least " << ((dataSize * 8) + headerBits) << " pixel values.";
        return Result<>(ErrorCode::ImageTooSmall, oss.str());
    }
    
    if (dataSize > availableCapacity) {
        std::ostringstream oss;
        oss << "Data size (" << dataSize << " bytes) exceeds Image capacity (" << availableCapacity << " bytes).\n"
            << "    Image has " << pixelCount << " pixel values.\n"
            << "    You need an image with at least " << ((dataSize * 8) + headerBits) << " pixel values.";
        return Result<>(ErrorCode::InsufficientCapacity, oss.str());
    }
    
    return Result<>();
}

Result<> StegoHandler::Embed(const std::string &coverFile,
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

    // Embed data into image
    auto embedResult = EmbedMethod(imageData.pixels, encryptedData, password);
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

Result<> StegoHandler::Extract(const std::string &stegoFile,
                                   const std::string &outputFile,
                                   const std::string &password) {
    
    // Load stego image
    auto imageResult = ImageIO::Load(stegoFile);
    if (!imageResult) {
        return Result<>(imageResult.GetErrorCode(), imageResult.GetErrorMessage());
    }
    
    const auto& imageData = imageResult.GetValue();
    const auto& pixels = imageData.pixels;
    
    // Extract encrypted data
    auto extractResult = ExtractMethod(pixels,password);
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

