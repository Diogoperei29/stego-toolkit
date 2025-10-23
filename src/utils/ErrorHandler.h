#ifndef __ERROR__HANDLER_H_
#define __ERROR__HANDLER_H_

#include <string>
#include <optional>

// Error codes for steganography operations.
enum class ErrorCode {
    // Success
    Success = 0,
    
    // File I/O errors
    FileNotFound = 100,
    FileReadError = 101,
    FileWriteError = 102,
    FilePermissionDenied = 103,
    
    // Image errors
    ImageLoadFailed = 200,
    ImageSaveFailed = 201,
    ImageTooSmall = 202,
    ImageCorrupted = 203,
    UnsupportedImageFormat = 204,
    InvalidImageDimensions = 205,
    
    // Capacity errors
    InsufficientCapacity = 300,
    DataTooLarge = 301,
    
    // Encryption/Decryption errors
    EncryptionFailed = 400,
    DecryptionFailed = 401,
    InvalidPassword = 402,
    KeyDerivationFailed = 403,
    
    // Embedding/Extraction errors
    EmbeddingFailed = 500,
    ExtractionFailed = 501,
    InvalidDataSize = 502,
    CorruptedPayload = 503,
    NoEmbeddedData = 504,
    
    // General errors
    UnknownError = 900,
    InvalidArgument = 901,
    NotImplemented = 902
};



// Result type for operations that can fail.
template<typename T = void>
class Result {
private:
    ErrorCode errorCode_;
    std::string errorMessage_;
    std::optional<T> value_;
    
public:
    // Success constructor (for types with value)
    Result(T value) 
        : errorCode_(ErrorCode::Success),
          value_(std::move(value)) 
    {   }
    
    // Error constructor
    Result(ErrorCode code, const std::string& message)
        : errorCode_(code),
          errorMessage_(message),
          value_(std::nullopt)
    {   }
    
    bool IsSuccess() const { return errorCode_ == ErrorCode::Success; }
    bool IsError() const { return !IsSuccess(); }
    ErrorCode GetErrorCode() const { return errorCode_; }
    const std::string& GetErrorMessage() const { return errorMessage_; }
    const T& GetValue() const { return value_.value(); }
    T& GetValue() { return value_.value(); }
    explicit operator bool() const { return IsSuccess(); }
};


// Specialization for void (operations with no return value).
template<>
class Result<void> {
private:
    ErrorCode errorCode_;
    std::string errorMessage_;
    
public:
    // Success constructor
    Result() 
        : errorCode_(ErrorCode::Success)
    {   }
    
    // Error constructor
    Result(ErrorCode code, const std::string& message)
        : errorCode_(code),
          errorMessage_(message)
    {   }
    
    bool IsSuccess() const { return errorCode_ == ErrorCode::Success; }
    bool IsError() const { return !IsSuccess(); }
    ErrorCode GetErrorCode() const { return errorCode_; }
    const std::string& GetErrorMessage() const { return errorMessage_; }
    explicit operator bool() const { return IsSuccess(); }
};


// Utility functions for error handling.
class ErrorHandler {
public:
    static std::string GetErrorCategory(ErrorCode code);
    static std::string GetErrorDescription(ErrorCode code);
    static std::string FormatError(ErrorCode code, const std::string& context = "");
};


#endif // __ERROR__HANDLER_H_
