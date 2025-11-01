#include "LSBShuffleStegoHandler.h"
#include "../../utils/ImageIO.h"
#include "../../utils/CryptoModule.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <iostream>

std::size_t LSBShuffleStegoHandler::CalculateLSBCapacity(std::size_t pixelCount) {
    if (pixelCount <= HEADER_SIZE_BITS) {
        return 0;
    }
    // Each pixel stores 1 bit, subtract header, convert to bytes
    return (pixelCount - HEADER_SIZE_BITS) / 8; 
}

std::size_t LSBShuffleStegoHandler::CalculateLSBCapacity(const ImageData& image) {
    return CalculateLSBCapacity(image.GetPixelCount());
}

Result<> LSBShuffleStegoHandler::ValidateLSBCapacity(std::size_t pixelCount, std::size_t dataSize) {
    if (dataSize > MAX_REASONABLE_SIZE) {
        std::ostringstream oss;
        oss << "Data size (" << dataSize << " bytes) exceeds maximum reasonable size (" 
            << MAX_REASONABLE_SIZE << " bytes)";
        return Result<>(ErrorCode::DataTooLarge, oss.str());
    }
    
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

Result<> LSBShuffleStegoHandler::Embed(const std::string &coverFile,
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
    auto embedResult = EmbedLSB(imageData.pixels, encryptedData, password);
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

Result<> LSBShuffleStegoHandler::Extract(const std::string &stegoFile,
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
    auto extractResult = ExtractLSB(pixels, password);
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

Result<> LSBShuffleStegoHandler::EmbedLSB(std::vector<uint8_t> &pixels,
                                        const std::vector<uint8_t> &dataToEmbed,
                                        const std::string &password) {
    
    if (dataToEmbed.empty()) {
        return Result<>(ErrorCode::InvalidArgument, "Cannot embed empty data");
    }
    
    uint32_t dataSize = static_cast<uint32_t>(dataToEmbed.size());
    uint64_t binarySize = (dataSize * 8) + HEADER_SIZE_BITS;
    std::size_t imgSize = pixels.size();

    if ( binarySize > imgSize) {
        std::ostringstream oss;
        oss << "Internal error: Data size " << dataSize << " bytes exceeds available capacity";
        return Result<>(ErrorCode::EmbeddingFailed, oss.str());
    }

    //start shuffling img bit locations
    std::size_t seed = std::hash<std::string>{}(password);
    std::mt19937_64 shuffler(seed);

    std::vector<std::size_t> imgBitLocationList(imgSize);
    for (std::size_t i = 0; i < imgSize; ++i) {
        imgBitLocationList[i] = i;
    }
    std::shuffle(imgBitLocationList.begin(), imgBitLocationList.end(), shuffler);
    
    std::vector<std::size_t> dataVector(binarySize);
    dataVector.insert(dataVector.begin(), {           //store datasize LSB first to simplify reading
        static_cast<uint8_t>(dataSize         & 0xFF),
        static_cast<uint8_t>((dataSize >>  8) & 0xFF),
        static_cast<uint8_t>((dataSize >> 16) & 0xFF),
        static_cast<uint8_t>((dataSize >> 24) & 0xFF)
    });

    dataVector.insert((dataVector.begin() + HEADER_SIZE_BYTES), dataToEmbed.begin(), dataToEmbed.end());
    
    // Embed data bits
    for (std::size_t byteIdx = 0; byteIdx < (dataSize + HEADER_SIZE_BYTES); ++byteIdx) {
        for (uint8_t bitIdx = 0; bitIdx < 8; ++bitIdx) {
            size_t bitIndex = (byteIdx * 8) + bitIdx;           //get location of bit
            uint8_t bit = (dataVector[byteIdx] >> bitIdx) & 1; //fetch data from linear data location 
            pixels[imgBitLocationList[bitIndex]] = (pixels[imgBitLocationList[bitIndex]] & 0xFE) | bit; // insert data on shuffled pixel location
        }   
    }

    return Result<>();
}

Result<std::vector<uint8_t>> LSBShuffleStegoHandler::ExtractLSB(const std::vector<uint8_t> &pixels, 
                                                                const std::string &password) {
    
    // Extract size header
    uint32_t dataSize = 0;
    std::size_t imgSize = pixels.size();

    //start shuffling pixel list
    std::size_t seed = std::hash<std::string>{}(password);
    std::mt19937_64 shuffler(seed);
    
    std::vector<std::size_t> imgBitLocationList(imgSize);
    for (std::size_t i = 0; i < imgSize; ++i) {
        imgBitLocationList[i] = i;
    }
    std::shuffle(imgBitLocationList.begin(), imgBitLocationList.end(), shuffler);
    
    // Extract img datasize bits in reverse 1st is msb
    for (std::size_t byteIdx = 0; byteIdx < HEADER_SIZE_BYTES; ++byteIdx) {
        for (uint8_t bitIdx = 0; bitIdx < 8; ++bitIdx) {
            size_t bitIndex = (byteIdx * 8) + bitIdx;           //get location of bit
            dataSize |= (pixels[imgBitLocationList[bitIndex]] & 1) << bitIndex; // insert data on shuffled pixel location
        }   
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
    
    // Extract the data bits
    std::vector<uint8_t> extractedData(dataSize, 0);
    for (std::size_t byteIdx = 0; byteIdx < dataSize ; ++byteIdx) {
        for (uint8_t bitIdx = 0; bitIdx < 8; ++bitIdx) {
            size_t bitIndex = ((byteIdx + HEADER_SIZE_BYTES) * 8) + bitIdx; //provide offset from HEADER location
            uint8_t bit = (pixels[imgBitLocationList[bitIndex]] & 1); //retrieve location based on locationlist
            extractedData[byteIdx] |= (bit << bitIdx);
        }
    }

    return Result<std::vector<uint8_t>>(extractedData);
}
