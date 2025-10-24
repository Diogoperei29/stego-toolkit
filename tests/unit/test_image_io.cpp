#include <gtest/gtest.h>
#include "utils/ImageIO.h"
#include "../test_helpers.h"
#include <filesystem>

namespace fs = std::filesystem;

// Image Loading Tests

TEST(ImageIO_Load, LoadsValidPNG) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(result.IsSuccess());
    
    auto image = result.GetValue();
    EXPECT_GT(image.width, 0);
    EXPECT_GT(image.height, 0);
    EXPECT_GT(image.channels, 0);
    EXPECT_FALSE(image.pixels.empty());
    EXPECT_EQ(image.pixels.size(), image.GetPixelCount());
}

TEST(ImageIO_Load, LoadsValidBMP) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.bmp").string());
    EXPECT_TRUE(result.IsSuccess());
    
    auto image = result.GetValue();
    EXPECT_GT(image.width, 0);
    EXPECT_GT(image.height, 0);
    EXPECT_FALSE(image.pixels.empty());
}

TEST(ImageIO_Load, LoadsValidJPEG) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.jpg").string());
    EXPECT_TRUE(result.IsSuccess());
    
    auto image = result.GetValue();
    EXPECT_GT(image.width, 0);
    EXPECT_GT(image.height, 0);
    EXPECT_FALSE(image.pixels.empty());
}

TEST(ImageIO_Load, HandlesGrayscaleImages) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue().channels, 1);
}

TEST(ImageIO_Load, HandlesRGBImages) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue().channels, 3);
}

TEST(ImageIO_Load, HandlesRGBAImages) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("rgba_test.png").string());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetValue().channels, 4);
}

TEST(ImageIO_Load, FailsOnMissingFile) {
    auto result = ImageIO::Load("nonexistent_file.png");
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ImageLoadFailed);
    
    std::string msg = result.GetErrorMessage();
    EXPECT_FALSE(msg.empty());
}

TEST(ImageIO_Load, FailsOnInvalidImageData) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("empty.txt").string());
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ImageLoadFailed);
}

TEST(ImageIO_Load, FailsOnCorruptedImage) {
    auto corruptData = std::vector<uint8_t>{0x89, 0x50, 0x4E, 0x47, 0x00, 0x00};
    auto corruptFile = TestHelpers::CreateTempFile("corrupt.png", corruptData);
    
    auto result = ImageIO::Load(corruptFile.string());
    EXPECT_TRUE(result.IsError());
}

TEST(ImageIO_Load, ReturnsCorrectDimensions) {
    auto tiny = ImageIO::Load(TestHelpers::GetFixturePath("tiny_gray.png").string());
    EXPECT_TRUE(tiny.IsSuccess());
    EXPECT_EQ(tiny.GetValue().width, 32);
    EXPECT_EQ(tiny.GetValue().height, 32);
    
    auto small = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(small.IsSuccess());
    EXPECT_EQ(small.GetValue().width, 100);
    EXPECT_EQ(small.GetValue().height, 100);
}

// Image Saving Tests

TEST(ImageIO_Save, SavesImageSuccessfully) {
    auto loaded = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(loaded.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("saved_image.png");
    auto saveResult = ImageIO::Save(outputPath.string(), loaded.GetValue());
    EXPECT_TRUE(saveResult.IsSuccess());
    EXPECT_TRUE(TestHelpers::FileExists(outputPath));
    
    auto reloaded = ImageIO::Load(outputPath.string());
    EXPECT_TRUE(reloaded.IsSuccess());
}

TEST(ImageIO_Save, SavesPNGFormat) {
    auto loaded = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(loaded.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("test_output.png");
    auto result = ImageIO::Save(outputPath.string(), loaded.GetValue());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(TestHelpers::FileExists(outputPath));
}

TEST(ImageIO_Save, SavesBMPFormat) {
    auto loaded = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(loaded.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("test_output.bmp");
    auto result = ImageIO::Save(outputPath.string(), loaded.GetValue());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(TestHelpers::FileExists(outputPath));
}

TEST(ImageIO_Save, PreservesImageData) {
    auto original = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(original.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("preserved.png");
    ImageIO::Save(outputPath.string(), original.GetValue());
    
    auto reloaded = ImageIO::Load(outputPath.string());
    EXPECT_TRUE(reloaded.IsSuccess());
    EXPECT_EQ(reloaded.GetValue().pixels, original.GetValue().pixels);
}

TEST(ImageIO_Save, PreservesChannelCount) {
    auto gray = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(gray.IsSuccess());
    auto grayOut = TestHelpers::GetOutputPath("gray_preserved.png");
    ImageIO::Save(grayOut.string(), gray.GetValue());
    auto grayReload = ImageIO::Load(grayOut.string());
    EXPECT_EQ(grayReload.GetValue().channels, 1);
    
    auto rgb = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(rgb.IsSuccess());
    auto rgbOut = TestHelpers::GetOutputPath("rgb_preserved.png");
    ImageIO::Save(rgbOut.string(), rgb.GetValue());
    auto rgbReload = ImageIO::Load(rgbOut.string());
    EXPECT_EQ(rgbReload.GetValue().channels, 3);
}

TEST(ImageIO_Save, FailsOnInvalidPath) {
    ImageData validImage;
    validImage.width = 10;
    validImage.height = 10;
    validImage.channels = 1;
    validImage.pixels.resize(100, 128);
    
    auto result = ImageIO::Save("Z:\\nonexistent\\path\\image.png", validImage);
    EXPECT_TRUE(result.IsError());
}

TEST(ImageIO_Save, FailsOnInvalidImageData) {
    ImageData invalidImage;
    invalidImage.width = 10;
    invalidImage.height = 10;
    invalidImage.channels = 1;
    
    auto result = ImageIO::Save(TestHelpers::GetOutputPath("invalid.png").string(), invalidImage);
    EXPECT_TRUE(result.IsError());
}

TEST(ImageIO_Save, CreatesDirectoriesIfNeeded) {
    auto loaded = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(loaded.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("subdir/test.png");
    auto result = ImageIO::Save(outputPath.string(), loaded.GetValue());
    
    if (result.IsSuccess()) {
        EXPECT_TRUE(TestHelpers::FileExists(outputPath));
    }
}

// Load + Save Round-Trip Tests

TEST(ImageIO_RoundTrip, GrayscalePNGRoundTrip) {
    auto original = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(original.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("gray_roundtrip.png");
    ImageIO::Save(outputPath.string(), original.GetValue());
    
    auto reloaded = ImageIO::Load(outputPath.string());
    EXPECT_TRUE(reloaded.IsSuccess());
    EXPECT_EQ(reloaded.GetValue().pixels, original.GetValue().pixels);
    EXPECT_EQ(reloaded.GetValue().width, original.GetValue().width);
    EXPECT_EQ(reloaded.GetValue().height, original.GetValue().height);
}

TEST(ImageIO_RoundTrip, RGBPNGRoundTrip) {
    auto original = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(original.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("rgb_roundtrip.png");
    ImageIO::Save(outputPath.string(), original.GetValue());
    
    auto reloaded = ImageIO::Load(outputPath.string());
    EXPECT_TRUE(reloaded.IsSuccess());
    EXPECT_EQ(reloaded.GetValue().pixels, original.GetValue().pixels);
}

TEST(ImageIO_RoundTrip, BMPRoundTrip) {
    auto original = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.bmp").string());
    EXPECT_TRUE(original.IsSuccess());
    
    auto outputPath = TestHelpers::GetOutputPath("bmp_roundtrip.bmp");
    ImageIO::Save(outputPath.string(), original.GetValue());
    
    auto reloaded = ImageIO::Load(outputPath.string());
    EXPECT_TRUE(reloaded.IsSuccess());
    EXPECT_EQ(reloaded.GetValue().width, original.GetValue().width);
    EXPECT_EQ(reloaded.GetValue().height, original.GetValue().height);
}

TEST(ImageIO_RoundTrip, FormatConversion) {
    auto original = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(original.IsSuccess());
    
    auto bmpPath = TestHelpers::GetOutputPath("converted.bmp");
    ImageIO::Save(bmpPath.string(), original.GetValue());
    
    auto asBmp = ImageIO::Load(bmpPath.string());
    EXPECT_TRUE(asBmp.IsSuccess());
    EXPECT_EQ(asBmp.GetValue().width, original.GetValue().width);
    EXPECT_EQ(asBmp.GetValue().height, original.GetValue().height);
}

// ImageData Struct Tests

TEST(ImageData_Struct, GetPixelCountIsCorrect) {
    ImageData img1;
    img1.width = 100;
    img1.height = 100;
    img1.channels = 1;
    EXPECT_EQ(img1.GetPixelCount(), 10000);
    
    ImageData img2;
    img2.width = 100;
    img2.height = 100;
    img2.channels = 3;
    EXPECT_EQ(img2.GetPixelCount(), 30000);
    
    ImageData img3;
    img3.width = 512;
    img3.height = 512;
    img3.channels = 3;
    EXPECT_EQ(img3.GetPixelCount(), 786432);
}

TEST(ImageData_Struct, IsValidChecksCorrectly) {
    ImageData valid;
    valid.width = 100;
    valid.height = 100;
    valid.channels = 1;
    valid.pixels.resize(10000);
    EXPECT_TRUE(valid.IsValid());
    
    ImageData invalidWidth;
    invalidWidth.width = 0;
    invalidWidth.height = 100;
    invalidWidth.channels = 1;
    invalidWidth.pixels.resize(100);
    EXPECT_FALSE(invalidWidth.IsValid());
    
    ImageData emptyPixels;
    emptyPixels.width = 100;
    emptyPixels.height = 100;
    emptyPixels.channels = 1;
    EXPECT_FALSE(emptyPixels.IsValid());
}

TEST(ImageData_Struct, DefaultConstructorCreatesInvalidImage) {
    ImageData defaultImage;
    EXPECT_FALSE(defaultImage.IsValid());
}

// Format Detection Tests

TEST(ImageIO_Format, IsSupportedFormatDetectsPNG) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(result.IsSuccess());
}

TEST(ImageIO_Format, IsSupportedFormatDetectsBMP) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.bmp").string());
    EXPECT_TRUE(result.IsSuccess());
}

TEST(ImageIO_Format, IsSupportedFormatDetectsJPEG) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.jpg").string());
    EXPECT_TRUE(result.IsSuccess());
}

TEST(ImageIO_Format, IsSupportedFormatRejectsUnsupported) {
    auto txtResult = ImageIO::Load(TestHelpers::GetFixturePath("small.txt").string());
    EXPECT_TRUE(txtResult.IsError());
    
    auto binResult = ImageIO::Load(TestHelpers::GetFixturePath("binary_data.bin").string());
    EXPECT_TRUE(binResult.IsError());
}

TEST(ImageIO_Format, IsSupportedFormatIsCaseInsensitive) {
    auto loaded = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(loaded.IsSuccess());
    
    auto upperPath = TestHelpers::GetOutputPath("TEST.PNG");
    auto result = ImageIO::Save(upperPath.string(), loaded.GetValue());
    EXPECT_TRUE(result.IsSuccess());
}

TEST(ImageIO_Format, GetExtensionWorksCorrectly) {
    auto png = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(png.IsSuccess());
    
    auto bmp = ImageIO::Load(TestHelpers::GetFixturePath("medium_gray.bmp").string());
    EXPECT_TRUE(bmp.IsSuccess());
}

// Pixel Data Validation Tests

TEST(ImageIO_Pixels, PixelVectorHasCorrectSize) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    auto data = image.GetValue();
    EXPECT_EQ(data.pixels.size(), data.width * data.height * data.channels);
}

TEST(ImageIO_Pixels, GrayscaleHasSingleChannel) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    auto data = image.GetValue();
    EXPECT_EQ(data.channels, 1);
    EXPECT_EQ(data.pixels.size(), data.width * data.height);
}

TEST(ImageIO_Pixels, RGBHasThreeChannels) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_rgb.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    auto data = image.GetValue();
    EXPECT_EQ(data.channels, 3);
    EXPECT_EQ(data.pixels.size(), data.width * data.height * 3);
}

TEST(ImageIO_Pixels, PixelValuesAreInRange) {
    auto image = ImageIO::Load(TestHelpers::GetFixturePath("small_gray.png").string());
    EXPECT_TRUE(image.IsSuccess());
    
    for (uint8_t pixel : image.GetValue().pixels) {
        EXPECT_GE(pixel, 0);
        EXPECT_LE(pixel, 255);
    }
}

// Error Message Quality Tests

TEST(ImageIO_Errors, FileNotFoundHasHelpfulMessage) {
    auto result = ImageIO::Load("missing_file.png");
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::ImageLoadFailed);
    
    std::string msg = result.GetErrorMessage();
    EXPECT_FALSE(msg.empty());
}

TEST(ImageIO_Errors, InvalidFormatHasHelpfulMessage) {
    auto result = ImageIO::Load(TestHelpers::GetFixturePath("small.txt").string());
    EXPECT_TRUE(result.IsError());
    
    std::string msg = result.GetErrorMessage();
    EXPECT_FALSE(msg.empty());
}
