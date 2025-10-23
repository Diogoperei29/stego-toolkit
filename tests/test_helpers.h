#ifndef __TEST_HELPERS_H_
#define __TEST_HELPERS_H_

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>


class TestHelpers {
public:
    TestHelpers() = delete;
    static std::filesystem::path GetFixturesDir();
    static std::filesystem::path GetOutputDir();
    static std::filesystem::path GetFixturePath(const std::string& filename);
    static std::filesystem::path GetOutputPath(const std::string& filename);
    static std::vector<uint8_t> ReadBinaryFile(const std::filesystem::path& filepath);
    static std::string ReadTextFile(const std::filesystem::path& filepath);
    static void WriteBinaryFile(const std::filesystem::path& filepath, const std::vector<uint8_t>& data);
    static void WriteTextFile(const std::filesystem::path& filepath, const std::string& content);
    static bool FilesAreIdentical(const std::filesystem::path& file1, const std::filesystem::path& file2);
    static bool ByteVectorsMatch(const std::vector<uint8_t>& data1, const std::vector<uint8_t>& data2);
    static void CleanOutputDirectory();
    static void RemoveOutputFile(const std::string& filename);
    static std::vector<uint8_t> GenerateRandomData(std::size_t size);
    static std::filesystem::path CreateTempFile(const std::string& filename, const std::vector<uint8_t>& data);
    static bool FileExists(const std::filesystem::path& filepath);
    static std::size_t GetFileSize(const std::filesystem::path& filepath);
};

#endif // __TEST_HELPERS_H_
