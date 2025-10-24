#include <gtest/gtest.h>
#include "algorithms/lsb/LSBStegoHandler.h"
#include "utils/CryptoModule.h"
#include "utils/ImageIO.h"
#include "../test_helpers.h"
#include <filesystem>

namespace fs = std::filesystem;

// Test fixture for embed/extract integration tests
class EmbedExtractTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestHelpers::CleanOutputDirectory();
    }
};

// Basic Embed + Extract Integration Tests

TEST_F(EmbedExtractTest, SmallTextInSmallImage) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_small.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_small.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "testpass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "testpass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, MediumTextInMediumImage) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("medium.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_medium.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_medium.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "mediumpass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "mediumpass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, BinaryDataRoundTrip) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("binary_data.bin").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_binary.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_binary.bin").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "binarypass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "binarypass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, UnicodeDataRoundTrip) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("unicode.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_unicode.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_unicode.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "unicodepass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "unicodepass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, LargeDataInLargeImage) {
    auto coverPath = TestHelpers::GetFixturePath("huge_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("large.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_large.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_large.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "largepass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "largepass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

// Multi-Format Integration Tests

TEST_F(EmbedExtractTest, PNGFormatRoundTrip) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_png.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_png.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "pngpass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "pngpass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, BMPFormatRoundTrip) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.bmp").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_bmp.bmp").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_bmp.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "bmppass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "bmppass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, GrayscaleVsRGBCapacity) {
    auto grayImage = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    ASSERT_TRUE(grayImage.IsSuccess());
    
    auto rgbImage = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    ASSERT_TRUE(rgbImage.IsSuccess());
    
    auto grayCapacity = LSBStegoHandler::CalculateLSBCapacity(grayImage.GetValue());
    auto rgbCapacity = LSBStegoHandler::CalculateLSBCapacity(rgbImage.GetValue());
    
    // RGB images have ~3x capacity of grayscale (3 color channels vs 1)
    EXPECT_GT(rgbCapacity, grayCapacity);
    EXPECT_NEAR(
        static_cast<double>(rgbCapacity),
        static_cast<double>(grayCapacity) * 3.0,
        static_cast<double>(grayCapacity) * 0.1
    );
}

// Password Integration Tests

TEST_F(EmbedExtractTest, CorrectPasswordExtracts) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_password.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_password.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "correct123");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "correct123");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, WrongPasswordFails) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_wrongpass.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_wrongpass.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "correct");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "wrong");
    EXPECT_TRUE(extractResult.IsError());
    EXPECT_EQ(extractResult.GetErrorCode(), ErrorCode::InvalidPassword);
}

TEST_F(EmbedExtractTest, PasswordCaseSensitivity) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_case.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_case.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "Password");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    // passwords should be case-sensitive
    auto extractResult = handler.Extract(stegoPath, extractPath, "password");
    EXPECT_TRUE(extractResult.IsError());
    EXPECT_EQ(extractResult.GetErrorCode(), ErrorCode::InvalidPassword);
}

TEST_F(EmbedExtractTest, SpecialCharacterPassword) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_special.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_special.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "P@ssw0rd!#$%");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "P@ssw0rd!#$%");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, UnicodePassword) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_unicode_pass.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_unicode_pass.txt").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "пароль🔐");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "пароль🔐");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

// Capacity Limit Integration Tests

TEST_F(EmbedExtractTest, MaxCapacityData) {
    auto coverImage = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    ASSERT_TRUE(coverImage.IsSuccess());
    
    // Calculate max capacity and generate data that approaches it
    auto maxCapacity = LSBStegoHandler::CalculateLSBCapacity(coverImage.GetValue());
    // Account for encryption overhead - use 90% of capacity
    auto safeDataSize = static_cast<size_t>(maxCapacity * 0.9);
    auto randomData = TestHelpers::GenerateRandomData(safeDataSize);
    
    auto dataPath = TestHelpers::GetOutputPath("maxcap_data.bin").string();
    TestHelpers::WriteBinaryFile(dataPath, randomData);
    
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_maxcap.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_maxcap.bin").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "maxpass");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "maxpass");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, OversizedDataFails) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("huge_1mb.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_oversized.png").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "oversized");
    EXPECT_TRUE(embedResult.IsError());
    EXPECT_EQ(embedResult.GetErrorCode(), ErrorCode::InsufficientCapacity);
}

TEST_F(EmbedExtractTest, HeaderSizeIsAccountedFor) {
    auto coverImage = ImageIO::Load(TestHelpers::GetFixturePath("tiny_gray.png").string());
    ASSERT_TRUE(coverImage.IsSuccess());
    
    // exactly 1 byte over capacity to verify header overhead is accounted for
    auto capacity = LSBStegoHandler::CalculateLSBCapacity(coverImage.GetValue());
    auto oversizedData = TestHelpers::GenerateRandomData(capacity + 1);
    
    auto dataPath = TestHelpers::GetOutputPath("oversized_data.bin").string();
    TestHelpers::WriteBinaryFile(dataPath, oversizedData);
    
    auto coverPath = TestHelpers::GetFixturePath("tiny_gray.png").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_header.png").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "header");
    EXPECT_TRUE(embedResult.IsError());
    EXPECT_EQ(embedResult.GetErrorCode(), ErrorCode::InsufficientCapacity);
}

// Error Recovery Integration Tests

TEST_F(EmbedExtractTest, RecoverFromMissingInputFile) {
    auto result = ImageIO::Load("nonexistent_image.png");
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ImageLoadFailed);
}

TEST_F(EmbedExtractTest, RecoverFromInvalidImageFormat) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small.txt").string());
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ImageLoadFailed);
}

// Data Integrity Integration Tests

TEST_F(EmbedExtractTest, ExactByteMatching) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("binary_data.bin").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_exact.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_exact.bin").string();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "exact");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "exact");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, PreservesFileSize) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("binary_data.bin").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_size.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_size.bin").string();
    
    auto originalData = TestHelpers::ReadBinaryFile(TestHelpers::GetFixturePath("binary_data.bin"));
    auto originalSize = originalData.size();
    
    LSBStegoHandler handler;
    
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "sizetest");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "sizetest");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    // Verify extracted data has exact same size as original (no padding artifacts)
    auto extractedData = TestHelpers::ReadBinaryFile(extractPath);
    EXPECT_EQ(extractedData.size(), originalSize);
}

// Edge Case Integration Tests

TEST_F(EmbedExtractTest, SingleByteData) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("single_byte.bin").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_single.png").string();
    auto extractPath = TestHelpers::GetOutputPath("extracted_single.bin").string();
    
    LSBStegoHandler handler;
    
    // Test edge case: embedding minimal data (1 byte)
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "single");
    ASSERT_TRUE(embedResult.IsSuccess());
    
    auto extractResult = handler.Extract(stegoPath, extractPath, "single");
    ASSERT_TRUE(extractResult.IsSuccess());
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(EmbedExtractTest, EmptyDataFails) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("empty.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_empty.png").string();
    
    LSBStegoHandler handler;
    
    // Empty files cannot be embedded
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "empty");
    EXPECT_TRUE(embedResult.IsError());
}

TEST_F(EmbedExtractTest, TinyImageInsufficientCapacity) {
    auto coverPath = TestHelpers::GetFixturePath("tiny_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("stego_tiny.png").string();
    
    LSBStegoHandler handler;
    
    // small.txt + encryption overhead should exceed tiny image capacity
    auto embedResult = handler.Embed(coverPath, dataPath, stegoPath, "tiny");
    EXPECT_TRUE(embedResult.IsError());
    EXPECT_EQ(embedResult.GetErrorCode(), ErrorCode::InsufficientCapacity);
}
