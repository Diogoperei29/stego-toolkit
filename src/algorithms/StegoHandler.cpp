#include "StegoHandler.h"
#include "../utils/ImageIO.h"
#include "../utils/CryptoModule.h"
#include "../utils/CompressionModule.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

Result<> StegoHandler::Embed(const std::string &coverFile,
                             const std::string &dataFile,
                             const std::string &outputFile,
                             const std::string &password,
                             int compressionLevel) {
    
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

    // Optionally compress data before encryption
    std::vector<uint8_t> processedData = plainData;
    if (compressionLevel > 0) {
        auto compressResult = CompressionModule::CompressData(plainData, compressionLevel);
        if (!compressResult) {
            return Result<>(
                compressResult.GetErrorCode(),
                "Compression failed: " + compressResult.GetErrorMessage()
            );
        }
        processedData = compressResult.GetValue();
    }

    // Encrypt data
    auto encryptResult = CryptoModule::EncryptData(processedData, password);
    if (!encryptResult) {
        return Result<>(
            encryptResult.GetErrorCode(),
            "Encryption failed: " + encryptResult.GetErrorMessage()
        );
    }
    
    const auto& encryptedData = encryptResult.GetValue();

    // Build payload: [1-byte compression level header | encrypted data]
    std::vector<uint8_t> payload;
    payload.reserve(1 + encryptedData.size());
    payload.push_back(static_cast<uint8_t>(compressionLevel));
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());

    // Embed payload into image
    auto embedResult = EmbedMethod(imageData, payload, password);
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
    
    // Extract payload (compression header byte + encrypted data)
    auto extractResult = ExtractMethod(imageData, password);
    if (!extractResult) {
        return Result<>(
            extractResult.GetErrorCode(),
            "Extraction failed: " + extractResult.GetErrorMessage()
        );
    }
    
    const auto& payload = extractResult.GetValue();

    // Parse compression header: first byte is the level used during embedding
    if (payload.empty()) {
        return Result<>(ErrorCode::CorruptedPayload, "Extracted payload is empty");
    }
    uint8_t embeddedCompressionLevel = payload[0];
    std::vector<uint8_t> encryptedData(payload.begin() + 1, payload.end());

    // Decrypt data
    auto decryptResult = CryptoModule::DecryptData(encryptedData, password);
    if (!decryptResult) {
        return Result<>(
            decryptResult.GetErrorCode(),
            "Decryption failed: " + decryptResult.GetErrorMessage()
        );
    }
    
    const auto& decryptedData = decryptResult.GetValue();

    // Decompress if the header indicates compression was applied
    std::vector<uint8_t> plainData;
    if (embeddedCompressionLevel > 0) {
        std::cout << "  Compression: ZSTD level " << static_cast<int>(embeddedCompressionLevel)
                  << " detected in header — decompressing...\n";
        auto decompressResult = CompressionModule::DecompressData(decryptedData);
        if (!decompressResult) {
            return Result<>(
                decompressResult.GetErrorCode(),
                "Decompression failed: " + decompressResult.GetErrorMessage()
            );
        }
        plainData = decompressResult.GetValue();
    } else {
        plainData = decryptedData;
    }

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

Result<> StegoHandler::Visual(const std::string &coverFile,
                             const std::string &dataFile,
                             const std::string &outputFile,
                             const std::string &password,
                             int compressionLevel) {
    
    // Load cover image
    auto imageResult = ImageIO::Load(coverFile);
    if (!imageResult) {
        return Result<>(imageResult.GetErrorCode(), imageResult.GetErrorMessage());
    }
    
    auto inputImage = imageResult.GetValue();

    //create a duplicate image with 0 filled pixels
    std::vector<uint8_t> imagePixels(inputImage.GetPixelCount());
    std::fill(imagePixels.begin(), imagePixels.end(), 0);
    auto imageData = ImageData(imagePixels, inputImage.width, inputImage.height, inputImage.channels);

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

    // Optionally compress data before encryption
    std::vector<uint8_t> processedData = plainData;
    if (compressionLevel > 0) {
        auto compressResult = CompressionModule::CompressData(plainData, compressionLevel);
        if (!compressResult) {
            return Result<>(
                compressResult.GetErrorCode(),
                "Compression failed: " + compressResult.GetErrorMessage()
            );
        }
        processedData = compressResult.GetValue();
    }

    // Encrypt data
    auto encryptResult = CryptoModule::EncryptData(processedData, password);
    if (!encryptResult) {
        return Result<>(
            encryptResult.GetErrorCode(),
            "Encryption failed: " + encryptResult.GetErrorMessage()
        );
    }
    
    const auto& encryptedData = encryptResult.GetValue();

    // Build payload: [1-byte compression level header | encrypted data]
    std::vector<uint8_t> payload;
    payload.reserve(1 + encryptedData.size());
    payload.push_back(static_cast<uint8_t>(compressionLevel));
    payload.insert(payload.end(), encryptedData.begin(), encryptedData.end());

    // Embed payload into image
    auto embedResult = EmbedMethod(imageData, payload, password);
    if (!embedResult) {
        return embedResult;
    }

    //change stego bits to visible color
    auto visualResult = VisualizeMethod(imageData);
    if (!visualResult) {
        return visualResult;
    }

    // Save stego image
    auto saveResult = ImageIO::Save(outputFile, imageData);
    if (!saveResult) {
        return saveResult;
    }

    return Result<>();
}
