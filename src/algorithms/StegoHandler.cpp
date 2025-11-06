#include "StegoHandler.h"
#include "../utils/ImageIO.h"
#include "../utils/CryptoModule.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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
    auto embedResult = EmbedMethod(imageData, encryptedData, password);
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
    
    // Extract encrypted data
    auto extractResult = ExtractMethod(imageData, password);
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

