#ifndef __COMPRESSION___MODULE_H_
#define __COMPRESSION___MODULE_H_

#include <cstdint>
#include <vector>
#include <zstd.h>
#include "ErrorHandler.h"

/**
 * @brief Static class to provide compression and de-compression utilities.
 */
class CompressionModule
{
public:
    
    CompressionModule() = delete; // static class
    
    /**
    * @brief Compresses data to a specified level
    *
    * @param data Data to compress
    * @param level ZSTD compression level
    * @return Result containing compression data or error
    */
    static Result<std::vector<uint8_t>> CompressData(
        const std::vector<uint8_t> &data,
        const int level = ZSTD_CLEVEL_DEFAULT
    );

    /**
    * @brief Decompresses ZSTD-compressed data.
    *
    * @param data Data to decompress
    * @return Result containing decompressed data or error
    */
    static Result<std::vector<uint8_t>> DecompressData(
        const std::vector<uint8_t> &data
    );
};


#endif // __COMPRESSION___MODULE_H_