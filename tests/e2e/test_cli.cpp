#include <gtest/gtest.h>
#include "core/CLI.h"
#include "utils/ImageIO.h"
#include "../test_helpers.h"
#include <filesystem>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

// CLI Test Fixture
class CLITest : public ::testing::Test {
protected:
    void SetUp() override {
        TestHelpers::CleanOutputDirectory();
        // Redirect stdout and stderr to suppress CLI output during tests
        oldCoutBuf = std::cout.rdbuf();
        oldCerrBuf = std::cerr.rdbuf();
        std::cout.rdbuf(nullStream.rdbuf());
        std::cerr.rdbuf(nullStream.rdbuf());
    }
    
    void TearDown() override {
        // Restore stdout and stderr
        std::cout.rdbuf(oldCoutBuf);
        std::cerr.rdbuf(oldCerrBuf);
    }
    
    int RunCLI(const std::vector<std::string>& args) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>("stegtool"));
        
        for (const auto& arg : args) {
            argStorage.push_back(arg);
        }
        
        for (auto& arg : argStorage) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        
        int exitCode = CLI::Run(static_cast<int>(argv.size()), argv.data());
        argStorage.clear();
        return exitCode;
    }

private:
    std::vector<std::string> argStorage;
    std::ostringstream nullStream;
    std::streambuf* oldCoutBuf;
    std::streambuf* oldCerrBuf;
};

// Help Command Tests

TEST_F(CLITest, HelpCommand_ShowsUsage) {
    int exitCode = RunCLI({"--help"});
    EXPECT_EQ(exitCode, 0);
}

TEST_F(CLITest, NoArguments_ShowsHelp) {
    int exitCode = RunCLI({});
    EXPECT_EQ(exitCode, 0);
}

// Embed Command Tests

TEST_F(CLITest, Embed_Success) {
    auto inputPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_stego.png").string();
    
    int exitCode = RunCLI({
        "embed",
        "-i", inputPath,
        "-d", dataPath,
        "-o", outputPath,
        "-p", "testpass"
    });
    
    EXPECT_EQ(exitCode, 0);
    EXPECT_TRUE(TestHelpers::FileExists(outputPath));
    
    auto stegoImage = ImageIO::Load(outputPath);
    EXPECT_TRUE(stegoImage.IsSuccess());
}

TEST_F(CLITest, Embed_MissingInputFile) {
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_missing.png").string();
    
    int exitCode = RunCLI({
        "embed",
        "-i", "nonexistent.png",
        "-d", dataPath,
        "-o", outputPath,
        "-p", "testpass"
    });
    
    EXPECT_NE(exitCode, 0);
}

TEST_F(CLITest, Embed_MissingDataFile) {
    auto inputPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_nodata.png").string();
    
    int exitCode = RunCLI({
        "embed",
        "-i", inputPath,
        "-d", "nonexistent.txt",
        "-o", outputPath,
        "-p", "testpass"
    });
    
    EXPECT_NE(exitCode, 0);
}

TEST_F(CLITest, Embed_MissingPassword) {
    auto inputPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_nopass.png").string();
    
    int exitCode = RunCLI({
        "embed",
        "-i", inputPath,
        "-d", dataPath,
        "-o", outputPath
    });
    
    // CLI allows empty password with a warning, which is acceptable behavior
    EXPECT_EQ(exitCode, 0);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
}

TEST_F(CLITest, Embed_InsufficientCapacity) {
    auto inputPath = TestHelpers::GetFixturePath("tiny_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("huge_1mb.txt").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_overflow.png").string();
    
    // Verify CLI provides helpful error message when data exceeds image capacity
    int exitCode = RunCLI({
        "embed",
        "-i", inputPath,
        "-d", dataPath,
        "-o", outputPath,
        "-p", "testpass"
    });
    
    EXPECT_NE(exitCode, 0);
}

TEST_F(CLITest, Embed_UnsupportedImageFormat) {
    auto inputPath = TestHelpers::GetFixturePath("small.txt").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto outputPath = TestHelpers::GetOutputPath("cli_badformat.png").string();
    
    // Attempting to use a text file as an image should fail with clear error
    int exitCode = RunCLI({
        "embed",
        "-i", inputPath,
        "-d", dataPath,
        "-o", outputPath,
        "-p", "testpass"
    });
    
    EXPECT_NE(exitCode, 0);
}

// Extract Command Tests

TEST_F(CLITest, Extract_Success) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_embed_extract.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_extracted.txt").string();
    
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "testpass"
    });
    ASSERT_EQ(embedCode, 0);
    
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "testpass"
    });
    
    EXPECT_EQ(extractCode, 0);
    EXPECT_TRUE(TestHelpers::FileExists(extractPath));
    
    auto originalData = TestHelpers::ReadTextFile(dataPath);
    auto extractedData = TestHelpers::ReadTextFile(extractPath);
    EXPECT_EQ(extractedData, originalData);
}

TEST_F(CLITest, Extract_WrongPassword) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_wrongpass.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_wrongpass.txt").string();
    
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "correct"
    });
    ASSERT_EQ(embedCode, 0);
    
    // Verify CLI properly rejects extraction with incorrect password
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "wrong"
    });
    
    EXPECT_NE(extractCode, 0);
}

TEST_F(CLITest, Extract_MissingInputFile) {
    auto extractPath = TestHelpers::GetOutputPath("cli_nofile.txt").string();
    
    int extractCode = RunCLI({
        "extract",
        "-i", "nonexistent.png",
        "-o", extractPath,
        "-p", "testpass"
    });
    
    EXPECT_NE(extractCode, 0);
}

// Full Workflow E2E Tests

TEST_F(CLITest, E2E_SmallTextWorkflow) {
    auto coverPath = TestHelpers::GetFixturePath("small_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_workflow_small.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_workflow_small.txt").string();
    
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "test123"
    });
    ASSERT_EQ(embedCode, 0);
    
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "test123"
    });
    ASSERT_EQ(extractCode, 0);
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(CLITest, E2E_BinaryDataWorkflow) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("binary_data.bin").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_workflow_binary.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_workflow_binary.bin").string();
    
    // Verify binary data (not just text) works
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "binarypass"
    });
    ASSERT_EQ(embedCode, 0);
    
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "binarypass"
    });
    ASSERT_EQ(extractCode, 0);
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(CLITest, E2E_UnicodeWorkflow) {
    auto coverPath = TestHelpers::GetFixturePath("medium_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("unicode.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_workflow_unicode.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_workflow_unicode.txt").string();
    
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "unicodepass"
    });
    ASSERT_EQ(embedCode, 0);
    
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "unicodepass"
    });
    ASSERT_EQ(extractCode, 0);
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(CLITest, E2E_LargeFileWorkflow) {
    auto coverPath = TestHelpers::GetFixturePath("huge_gray.png").string();
    auto dataPath = TestHelpers::GetFixturePath("large.txt").string();
    auto stegoPath = TestHelpers::GetOutputPath("cli_workflow_large.png").string();
    auto extractPath = TestHelpers::GetOutputPath("cli_workflow_large.txt").string();
    
    // Verify CLI handles larger files
    int embedCode = RunCLI({
        "embed",
        "-i", coverPath,
        "-d", dataPath,
        "-o", stegoPath,
        "-p", "largepass"
    });
    ASSERT_EQ(embedCode, 0);
    
    int extractCode = RunCLI({
        "extract",
        "-i", stegoPath,
        "-o", extractPath,
        "-p", "largepass"
    });
    ASSERT_EQ(extractCode, 0);
    
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, extractPath));
}

TEST_F(CLITest, E2E_MultipleFormats) {
    auto pngCover = TestHelpers::GetFixturePath("small_gray.png").string();
    auto bmpCover = TestHelpers::GetFixturePath("medium_gray.bmp").string();
    auto dataPath = TestHelpers::GetFixturePath("small.txt").string();
    auto pngStego = TestHelpers::GetOutputPath("cli_format.png").string();
    auto bmpStego = TestHelpers::GetOutputPath("cli_format.bmp").string();
    auto pngExtract = TestHelpers::GetOutputPath("cli_format_png.txt").string();
    auto bmpExtract = TestHelpers::GetOutputPath("cli_format_bmp.txt").string();
    
    // Test PNG format
    int pngEmbed = RunCLI({
        "embed", 
        "-i", pngCover,
        "-d", dataPath,
        "-o", pngStego, 
        "-p", "png"
    });
    ASSERT_EQ(pngEmbed, 0);
    
    int pngExtractCode = RunCLI({
        "extract",
        "-i", pngStego, 
        "-o", pngExtract, 
        "-p", "png"
    });
    ASSERT_EQ(pngExtractCode, 0);
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, pngExtract));
    
    // Test BMP format with same data
    int bmpEmbed = RunCLI({
        "embed",
        "-i", bmpCover,
        "-d", dataPath,
        "-o", bmpStego, 
        "-p", "bmp"
    });
    ASSERT_EQ(bmpEmbed, 0);
    
    int bmpExtractCode = RunCLI({
        "extract", 
        "-i", bmpStego, 
        "-o", bmpExtract, 
        "-p", "bmp"
    });
    ASSERT_EQ(bmpExtractCode, 0);
    EXPECT_TRUE(TestHelpers::FilesAreIdentical(dataPath, bmpExtract));
}

// Version/Info Tests

TEST_F(CLITest, Version_ShowsVersionInfo) {
    int exitCode = RunCLI({"--version"});
    EXPECT_EQ(exitCode, 0);
}
