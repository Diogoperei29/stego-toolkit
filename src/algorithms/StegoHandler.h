#ifndef STEGO_HANDLER_H
#define STEGO_HANDLER_H

#include <string>
#include <vector>
#include <cstdint>

// Abstract base class for all steganography handlers
class StegoHandler {
public:
    // Pure virtual functions for embedding and extracting data
    virtual bool Embed(const std::string &coverFile,
                       const std::string &embedFile,
                       const std::string &outputFile,
                       const std::string &password) = 0;

    virtual bool Extract(const std::string &stegoFile,
                         const std::string &outputFile,
                         const std::string &password) = 0;

    virtual ~StegoHandler() = default;
};

#endif // STEGO_HANDLER_H