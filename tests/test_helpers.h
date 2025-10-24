#ifndef __TEST_HELPERS_H_
#define __TEST_HELPERS_H_

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>
#include "utils/ErrorHandler.h"

class TestHelpers {
public:
    TestHelpers() = delete;
    
    // Get directory path to test fixtures
    static std::filesystem::path GetFixturesDir();
    // Get directory path to test output
    static std::filesystem::path GetOutputDir();
    // Get full path to fixture file
    static std::filesystem::path GetFixturePath(const std::string& filename);
    // Get full path to output file
    static std::filesystem::path GetOutputPath(const std::string& filename);
    
    // Read file as binary data
    static std::vector<uint8_t> ReadBinaryFile(const std::filesystem::path& filepath);
    // Read file as text string
    static std::string ReadTextFile(const std::filesystem::path& filepath);
    // Write binary data to file
    static void WriteBinaryFile(const std::filesystem::path& filepath, const std::vector<uint8_t>& data);
    // Write text string to file
    static void WriteTextFile(const std::filesystem::path& filepath, const std::string& content);
    
    // Compare two files byte-by-byte
    static bool FilesAreIdentical(const std::filesystem::path& file1, const std::filesystem::path& file2);
    // Compare two byte vectors
    static bool ByteVectorsMatch(const std::vector<uint8_t>& data1, const std::vector<uint8_t>& data2);
    
    // Clean all files from output directory
    static void CleanOutputDirectory();
    // Remove specific file from output directory
    static void RemoveOutputFile(const std::string& filename);
    
    // Generate random binary data
    static std::vector<uint8_t> GenerateRandomData(std::size_t size);
    // Create temporary file in output directory
    static std::filesystem::path CreateTempFile(const std::string& filename, const std::vector<uint8_t>& data);
    
    // Check if file exists
    static bool FileExists(const std::filesystem::path& filepath);
    // Get size of file in bytes
    static std::size_t GetFileSize(const std::filesystem::path& filepath);
    
    // Error handling test helpers
    // Create Result that can fail based on condition
    static Result<int> FunctionThatCanFail(bool shouldFail);
    // Create Result that calls another failing function
    static Result<int> FunctionThatCallsAnother(bool shouldFail);
};

#endif // __TEST_HELPERS_H_
