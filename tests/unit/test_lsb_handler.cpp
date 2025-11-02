#include <gtest/gtest.h>
#include "algorithms/lsb/LSBStegoHandler.h"
#include "utils/ImageIO.h"
#include "../test_helpers.h"

// LSB Capacity Calculation Tests

TEST(LSBHandler_Capacity, CalculatesCorrectCapacityFromPixelCount) {
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(100, LSBStegoHandler::HEADER_SIZE_BITS), 8);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(1000, LSBStegoHandler::HEADER_SIZE_BITS), 121);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(1000000, LSBStegoHandler::HEADER_SIZE_BITS), 124996);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(32, LSBStegoHandler::HEADER_SIZE_BITS), 0);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(40, LSBStegoHandler::HEADER_SIZE_BITS), 1);
}

TEST(LSBHandler_Capacity, CalculatesCorrectCapacityFromImageData) {
    auto tinyGray = ImageIO::Load(TestHelpers::GetFixturePath("tiny_gray.png").string());
    EXPECT_TRUE(tinyGray.IsSuccess());
    auto capacity1 = LSBStegoHandler::CalculateCapacity(tinyGray.GetValue(), LSBStegoHandler::HEADER_SIZE_BITS);
    EXPECT_GT(capacity1, 0);
    
    auto smallGray = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(smallGray.IsSuccess());
    auto capacity2 = LSBStegoHandler::CalculateCapacity(smallGray.GetValue(), LSBStegoHandler::HEADER_SIZE_BITS);
    EXPECT_GT(capacity2, capacity1);
    
    auto smallRGB = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(smallRGB.IsSuccess());
    auto capacity3 = LSBStegoHandler::CalculateCapacity(smallRGB.GetValue(), LSBStegoHandler::HEADER_SIZE_BITS);
    EXPECT_GT(capacity3, capacity2);
}

TEST(LSBHandler_Capacity, HandlesEdgeCasesCorrectly) {
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(0, LSBStegoHandler::HEADER_SIZE_BITS), 0);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(10, LSBStegoHandler::HEADER_SIZE_BITS), 0);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(31, LSBStegoHandler::HEADER_SIZE_BITS), 0);
    
    ImageData smallImage;
    smallImage.width = 10;
    smallImage.height = 10;
    smallImage.channels = 1;
    smallImage.pixels.resize(100);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(smallImage, LSBStegoHandler::HEADER_SIZE_BITS), 8);
    
    ImageData multiChannel = smallImage;
    multiChannel.channels = 3;
    multiChannel.pixels.resize(300);
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(multiChannel, LSBStegoHandler::HEADER_SIZE_BITS), 33);
}

// LSB Capacity Validation Tests

TEST(LSBHandler_Validation, AcceptsValidDataSize) {
    auto result = LSBStegoHandler::ValidateCapacity(1000, 100, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result.IsSuccess());
    
    auto result2 = LSBStegoHandler::ValidateCapacity(1000, 121, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result2.IsSuccess());
}

TEST(LSBHandler_Validation, RejectsOversizedData) {
    auto result = LSBStegoHandler::ValidateCapacity(1000, 122, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InsufficientCapacity);
    EXPECT_FALSE(result.GetErrorMessage().empty());
    
    auto result2 = LSBStegoHandler::ValidateCapacity(100, 50, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result2.IsError());
    EXPECT_EQ(result2.GetErrorCode(), ErrorCode::InsufficientCapacity);
}

TEST(LSBHandler_Validation, RejectsUnreasonablyLargeData) {
    size_t maxSize = 100 * 1024 * 1024;
    auto result = LSBStegoHandler::ValidateCapacity(1000000000, maxSize + 1, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::DataTooLarge);
}

// LSB Embedding Tests

TEST(LSBHandler_Embed, EmbedsDataCorrectly) {
    std::vector<uint8_t> pixels(1000, 128);
    std::vector<uint8_t> data{0xAB, 0xCD};
    
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
    
    uint32_t embeddedSize = 0;
    for (int i = 0; i < 32; i++) {
        embeddedSize |= ((pixels[i] & 1) << i);
    }
    EXPECT_EQ(embeddedSize, 2);
}

TEST(LSBHandler_Embed, ModifiesOnlyLSBs) {
    std::vector<uint8_t> original(1000, 128);
    std::vector<uint8_t> pixels = original;
    std::vector<uint8_t> data{0x55, 0xAA, 0xFF};
    
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
    
    for (size_t i = 0; i < pixels.size(); i++) {
        int diff = std::abs(static_cast<int>(pixels[i]) - static_cast<int>(original[i]));
        EXPECT_LE(diff, 1);
    }
}

TEST(LSBHandler_Embed, EmbedsSizeHeaderCorrectly) {
    std::vector<uint8_t> pixels(1000, 0);
    std::vector<uint8_t> data(42, 0xFF);
    
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
    
    uint32_t size = 0;
    for (int i = 0; i < 32; i++) {
        size |= ((pixels[i] & 1) << i);
    }
    EXPECT_EQ(size, 42);
}

TEST(LSBHandler_Embed, HandlesSingleByteData) {
    auto singleByte = TestHelpers::ReadBinaryFile(TestHelpers::GetFixturePath("single_byte.bin"));
    EXPECT_EQ(singleByte.size(), 1);
    
    std::vector<uint8_t> pixels(1000, 0);
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, singleByte, "");
    EXPECT_TRUE(result.IsSuccess());
}

TEST(LSBHandler_Embed, HandlesMaxCapacityData) {
    size_t pixelCount = 1000;
    size_t maxCapacity = (pixelCount - 32) / 8;
    std::vector<uint8_t> pixels(pixelCount, 0);
    std::vector<uint8_t> data(maxCapacity, 0x42);
    
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
}

// LSB Extraction Tests

TEST(LSBHandler_Extract, ExtractsEmbeddedData) {
    std::vector<uint8_t> pixels(1000, 0);
    std::vector<uint8_t> original{1, 2, 3, 4, 5};
    
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(pixels, original, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue(), original);
}

TEST(LSBHandler_Extract, ReadsCorrectDataSize) {
    LSBStegoHandler handler;
    
    std::vector<uint8_t> pixels1(1000, 0);
    std::vector<uint8_t> data1(1, 0xAA);
    handler.EmbedMethod(pixels1, data1, "");
    auto result1 = handler.ExtractMethod(pixels1, "");
    EXPECT_TRUE(result1.IsSuccess());
    EXPECT_EQ(result1.GetValue().size(), 1);
    
    std::vector<uint8_t> pixels2(10000, 0);
    std::vector<uint8_t> data2(100, 0xBB);
    handler.EmbedMethod(pixels2, data2, "");
    auto result2 = handler.ExtractMethod(pixels2, "");
    EXPECT_TRUE(result2.IsSuccess());
    EXPECT_EQ(result2.GetValue().size(), 100);
}

TEST(LSBHandler_Extract, HandlesCorruptSizeHeader) {
    LSBStegoHandler handler;
    std::vector<uint8_t> pixels(1000, 0);
    
    for (int i = 0; i < 32; i++) {
        pixels[i] = (i < 16) ? 1 : 0;
    }
    auto result = handler.ExtractMethod(pixels, "");
    EXPECT_TRUE(result.IsError());
    
    std::vector<uint8_t> pixels2(1000, 0xFF);
    auto result2 = handler.ExtractMethod(pixels2, "");
    EXPECT_TRUE(result2.IsError());
}

// Embed + Extract Round-Trip Tests

TEST(LSBHandler_RoundTrip, SmallDataRoundTrip) {
    auto smallData = TestHelpers::ReadTextFile(TestHelpers::GetFixturePath("small.txt"));
    std::vector<uint8_t> data(smallData.begin(), smallData.end());
    
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, data, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue(), data);
}

TEST(LSBHandler_RoundTrip, BinaryDataRoundTrip) {
    auto binaryData = TestHelpers::ReadBinaryFile(TestHelpers::GetFixturePath("binary_data.bin"));
    
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, binaryData, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue(), binaryData);
}

TEST(LSBHandler_RoundTrip, UnicodeDataRoundTrip) {
    auto unicodeText = TestHelpers::ReadTextFile(TestHelpers::GetFixturePath("unicode.txt"));
    std::vector<uint8_t> data(unicodeText.begin(), unicodeText.end());
    
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, data, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    
    std::string extracted(extractResult.GetValue().begin(), extractResult.GetValue().end());
    EXPECT_EQ(extracted, unicodeText);
}

TEST(LSBHandler_RoundTrip, LargeDataRoundTrip) {
    auto largeText = TestHelpers::ReadTextFile(TestHelpers::GetFixturePath("large.txt"));
    std::vector<uint8_t> data(largeText.begin(), largeText.end());
    
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("huge_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, data, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue().size(), data.size());
}

// Image Format Support Tests

TEST(LSBHandler_Formats, WorksWithPNG) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    std::vector<uint8_t> data{1, 2, 3, 4, 5};
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, data, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue(), data);
}

TEST(LSBHandler_Formats, WorksWithBMP) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.bmp").string());
    EXPECT_TRUE(image.IsSuccess());
    
    std::vector<uint8_t> data{0xDE, 0xAD, 0xBE, 0xEF};
    LSBStegoHandler handler;
    auto embedResult = handler.EmbedMethod(image.GetValue().pixels, data, "");
    EXPECT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.ExtractMethod(image.GetValue().pixels, "");
    EXPECT_TRUE(extractResult.IsSuccess());
    EXPECT_EQ(extractResult.GetValue(), data);
}

TEST(LSBHandler_Formats, HandlesGrayscaleImages) {
    auto grayImage = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(grayImage.IsSuccess());
    EXPECT_EQ(grayImage.GetValue().channels, 1);
    
    std::vector<uint8_t> data{0xAA, 0xBB, 0xCC};
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(grayImage.GetValue().pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
    
    auto extracted = handler.ExtractMethod(grayImage.GetValue().pixels, "");
    EXPECT_TRUE(extracted.IsSuccess());
    EXPECT_EQ(extracted.GetValue(), data);
}

TEST(LSBHandler_Formats, HandlesRGBImages) {
    auto rgbImage = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(rgbImage.IsSuccess());
    EXPECT_EQ(rgbImage.GetValue().channels, 3);
    
    auto grayImage = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(grayImage.IsSuccess());
    
    auto rgbCapacity = LSBStegoHandler::CalculateCapacity(rgbImage.GetValue(), LSBStegoHandler::HEADER_SIZE_BITS);
    auto grayCapacity = LSBStegoHandler::CalculateCapacity(grayImage.GetValue(), LSBStegoHandler::HEADER_SIZE_BITS);
    EXPECT_GT(rgbCapacity, grayCapacity);
}

TEST(LSBHandler_Formats, HandlesRGBAImages) {
    auto rgbaImage = ImageIO::Load(TestHelpers::GetFixturePath("rgba_test.png").string());
    EXPECT_TRUE(rgbaImage.IsSuccess());
    EXPECT_EQ(rgbaImage.GetValue().channels, 4);
    
    std::vector<uint8_t> data(50, 0x55);
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(rgbaImage.GetValue().pixels, data, "");
    EXPECT_TRUE(result.IsSuccess());
    
    auto extracted = handler.ExtractMethod(rgbaImage.GetValue().pixels, "");
    EXPECT_TRUE(extracted.IsSuccess());
    EXPECT_EQ(extracted.GetValue(), data);
}

// Error Handling Tests

TEST(LSBHandler_Errors, RejectsInvalidImageData) {
    ImageData invalidImage;
    invalidImage.width = 0;
    invalidImage.height = 0;
    invalidImage.channels = 0;
    
    EXPECT_FALSE(invalidImage.IsValid());
    EXPECT_EQ(LSBStegoHandler::CalculateCapacity(invalidImage, LSBStegoHandler::HEADER_SIZE_BITS), 0);
}

TEST(LSBHandler_Errors, RejectsEmptyData) {
    std::vector<uint8_t> pixels(1000, 0);
    std::vector<uint8_t> emptyData;
    
    LSBStegoHandler handler;
    auto result = handler.EmbedMethod(pixels, emptyData, "");
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidArgument);
}

TEST(LSBHandler_Errors, ProvidesDescriptiveErrorMessages) {
    auto result = LSBStegoHandler::ValidateCapacity(100, 50, LSBStegoHandler::HEADER_SIZE_BITS, LSBStegoHandler::MAX_REASONABLE_SIZE );
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InsufficientCapacity);
    
    std::string msg = result.GetErrorMessage();
    EXPECT_FALSE(msg.empty());
    EXPECT_NE(msg.find("capacity"), std::string::npos);
}
