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

Result<> LSBShuffleStegoHandler::EmbedMethod(std::vector<uint8_t> &pixels,
                                        const std::vector<uint8_t> &dataToEmbed,
                                        const std::string &password) {
    
    if (dataToEmbed.empty()) {
        return Result<>(ErrorCode::InvalidArgument, "Cannot embed empty data");
    }
    
    // Validate capacity
    auto capacityCheck = StegoHandler::ValidateCapacity(pixels.size(), dataToEmbed.size(), HEADER_SIZE_BITS, MAX_REASONABLE_SIZE);
    if (!capacityCheck) {
        return capacityCheck;
    }

    std::size_t imgSize = pixels.size();
    
    //start shuffling img bit locations
    std::size_t seed = std::hash<std::string>{}(password);
    std::mt19937_64 shuffler(seed);

    std::vector<std::size_t> imgBitLocationList(imgSize);
    for (std::size_t i = 0; i < imgSize; ++i) {
        imgBitLocationList[i] = i;
    }
    std::shuffle(imgBitLocationList.begin(), imgBitLocationList.end(), shuffler);
    
    uint32_t dataSize = static_cast<uint32_t>(dataToEmbed.size());
    uint64_t binarySize = (dataSize * 8) + HEADER_SIZE_BITS;

    std::vector<uint8_t> dataVector(binarySize);
    // Store datasize LSB first to simplify reading
    dataVector[0] = static_cast<uint8_t>(dataSize         & 0xFF);
    dataVector[1] = static_cast<uint8_t>((dataSize >>  8) & 0xFF);
    dataVector[2] = static_cast<uint8_t>((dataSize >> 16) & 0xFF);
    dataVector[3] = static_cast<uint8_t>((dataSize >> 24) & 0xFF);
    // Copy dataToEmbed into dataVector after header
    std::copy(dataToEmbed.begin(), dataToEmbed.end(), dataVector.begin() + HEADER_SIZE_BYTES);
    
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

Result<std::vector<uint8_t>> LSBShuffleStegoHandler::ExtractMethod(const std::vector<uint8_t> &pixels, 
                                                                const std::string &password) {
    

    std::size_t imgSize = pixels.size();
    
    // Validate minimum size
    if (imgSize < HEADER_SIZE_BITS) {
        std::ostringstream oss;
        oss << "Image too small to contain embedded data. "
            << "Has " << imgSize << " pixels, needs at least " << HEADER_SIZE_BITS;
        return Result<std::vector<uint8_t>>(ErrorCode::ImageTooSmall, oss.str());
    }

    // Extract size header
    uint32_t dataSize = 0;
    
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
    
    if (dataSize * 8 + HEADER_SIZE_BITS > imgSize) {
        std::ostringstream oss;
        oss << "Extracted size (" << dataSize << " bytes) exceeds image capacity. "
            << "Image has " << imgSize << " pixel values, "
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
