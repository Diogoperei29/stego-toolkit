#include "LSBStegoHandlerOrdered.h"
#include "../../../utils/ImageIO.h"
#include "../../../utils/CryptoModule.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

Result<> LSBStegoHandlerOrdered::EmbedMethod(ImageData &imageData,
                                             const std::vector<uint8_t> &dataToEmbed,
                                             const std::string &password ) {
    
    (void) password; //Avoid unused parameter warning for LSB Method
    
    if (dataToEmbed.empty()) {
        return Result<>(ErrorCode::InvalidArgument, "Cannot embed empty data");
    }

    auto &pixels = imageData.pixels;
    
    // Validate capacity
    auto capacityCheck = LSBStegoHandler::ValidateCapacity(pixels.size(), dataToEmbed.size(), HEADER_SIZE_BITS, MAX_REASONABLE_SIZE);
    if (!capacityCheck) {
        return capacityCheck;
    }

    uint32_t dataSize = static_cast<uint32_t>(dataToEmbed.size());

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

Result<std::vector<uint8_t>> LSBStegoHandlerOrdered::ExtractMethod(const ImageData &imageData, 
                                                                   const std::string &password ) {
    
    (void) password; //Avoid unused parameter warning for LSB Method

    auto &pixels = imageData.pixels;

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
