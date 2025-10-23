#include "test_helpers.h"
#include <fstream>
#include <random>
#include <stdexcept>

// Path Helpers
std::filesystem::path TestHelpers::GetFixturesDir() {
    return std::filesystem::path(TEST_FIXTURES_DIR);
}

std::filesystem::path TestHelpers::GetOutputDir() {
    return std::filesystem::path(TEST_OUTPUT_DIR);
}

std::filesystem::path TestHelpers::GetFixturePath(const std::string& filename) {
    return GetFixturesDir() / filename;
}

std::filesystem::path TestHelpers::GetOutputPath(const std::string& filename) {
    return GetOutputDir() / filename;
}

// File I/O Helpers
std::vector<uint8_t> TestHelpers::ReadBinaryFile(const std::filesystem::path& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) throw std::runtime_error("open failed");

    std::vector<uint8_t> out((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());

    ifs.close();

    return out;
}

std::string TestHelpers::ReadTextFile(const std::filesystem::path& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) throw std::runtime_error("open failed");

    std::string out((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());

    ifs.close();
    
    return out;
}

void TestHelpers::WriteBinaryFile(const std::filesystem::path& filepath, 
                                  const std::vector<uint8_t>& data) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) throw std::runtime_error("open failed");
    if (!data.empty()) ofs.write(reinterpret_cast<const char*>(data.data()),
                                 static_cast<std::streamsize>(data.size()));
}

void TestHelpers::WriteTextFile(const std::filesystem::path& filepath, 
                                const std::string& content) {
    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) throw std::runtime_error("open failed");
    if (!content.empty()) ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
}

// File Comparison Helpers Implementation
bool TestHelpers::FilesAreIdentical(const std::filesystem::path& file1, 
                                    const std::filesystem::path& file2) {
    return (ReadBinaryFile(file1) == ReadBinaryFile(file2));
}

bool TestHelpers::ByteVectorsMatch(const std::vector<uint8_t>& data1, 
                                   const std::vector<uint8_t>& data2) {
    return (data1 == data2);
}

// Test Cleanup Helpers Implementation
void TestHelpers::CleanOutputDirectory() {
    for (const auto& entry : std::filesystem::directory_iterator(GetOutputDir())) {
        std::filesystem::remove(entry);
    }
}

void TestHelpers::RemoveOutputFile(const std::string& filename) {
    std::filesystem::remove(GetOutputPath(filename));
}

// Test Data Generation Helpers Implementation
std::vector<uint8_t> TestHelpers::GenerateRandomData(std::size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (auto& byte : data) byte = static_cast<uint8_t>(dis(gen));
    return data;
}

std::filesystem::path TestHelpers::CreateTempFile(const std::string& filename, 
                                                  const std::vector<uint8_t>& data) {
    auto path = GetOutputPath(filename);
    WriteBinaryFile(path, data);
    return path;
}

// Assertion Helpers Implementation
bool TestHelpers::FileExists(const std::filesystem::path& filepath) {
    return std::filesystem::exists(filepath);
}

std::size_t TestHelpers::GetFileSize(const std::filesystem::path& filepath) {
    if (!FileExists(filepath)) return 0;
    return std::filesystem::file_size(filepath);
}
