#include "CompressionModule.h"


Result<std::vector<uint8_t>> CompressionModule::CompressData(
    const std::vector<uint8_t> &data,
    const int level
) {
    std::size_t const bound = ZSTD_compressBound(data.size());
    std::vector<std::uint8_t> output(bound);

    std::size_t const result = ZSTD_compress(
        output.data(), bound,
        data.data(), data.size(),
        level
    );

    if (ZSTD_isError(result)) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::CompressionFailed,
            "Cannot compress data"
        );
    }

    output.resize(result);
    return Result<std::vector<uint8_t>>(output);
}

Result<std::vector<uint8_t>> CompressionModule::DecompressData(
    const std::vector<uint8_t> &data
) {
    std::uint64_t originalSize = ZSTD_getFrameContentSize(data.data(), data.size());
    if (originalSize == ZSTD_CONTENTSIZE_ERROR) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::BadCompressedData,
            "Not a valid zstd frame"
        );
    }
    if (originalSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::BadCompressedData,
            "Original size unknown, use streaming API"
        );
    }

    std::vector<uint8_t> output(originalSize);

    size_t const result = ZSTD_decompress(
        output.data(), originalSize,
        data.data(), data.size()
    );

    if (ZSTD_isError(result)) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecompressionFailed,
            "Failed to Decompress"
        );
    }

    return Result<std::vector<uint8_t>>(output);
}