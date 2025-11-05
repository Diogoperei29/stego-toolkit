#include "LSBStegoHandler.h"
#include "../../utils/ImageIO.h"

#include <sstream>

std::size_t LSBStegoHandler::CalculateCapacity(std::size_t pixelCount, std::size_t headerBits) {
    if (pixelCount <= headerBits) {
        return 0;
    }
    // Each pixel stores 1 bit, subtract header, convert to bytes
    return (pixelCount - headerBits) / 8; 
}

std::size_t LSBStegoHandler::CalculateCapacity(const ImageData& image, std::size_t headerBits) {
    return CalculateCapacity(image.GetPixelCount(), headerBits);
}

Result<> LSBStegoHandler::ValidateCapacity(std::size_t pixelCount, std::size_t dataSize, 
                                            std::size_t headerBits, std::size_t fileMaxSize) {
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
