#include "ErrorHandler.h"

std::string ErrorHandler::GetErrorCategory(ErrorCode code) {
    int codeValue = static_cast<int>(code);
    
    if (codeValue == 0) return "Success";
    if (codeValue >= 100 && codeValue < 200) return "File I/O Error";
    if (codeValue >= 200 && codeValue < 300) return "Image Error";
    if (codeValue >= 300 && codeValue < 400) return "Capacity Error";
    if (codeValue >= 400 && codeValue < 500) return "Encryption Error";
    if (codeValue >= 500 && codeValue < 600) return "Embedding Error";
    if (codeValue >= 900) return "General Error";
    
    return "Unknown Error";
}

std::string ErrorHandler::GetErrorDescription(ErrorCode code) {
    switch (code) {
        case ErrorCode::Success:
            return "Operation completed successfully";
            
        // File I/O errors
        case ErrorCode::FileNotFound:
            return "File not found or does not exist";
        case ErrorCode::FileReadError:
            return "Failed to read file";
        case ErrorCode::FileWriteError:
            return "Failed to write file";
        case ErrorCode::FilePermissionDenied:
            return "Permission denied to access file";
            
        // Image errors
        case ErrorCode::ImageLoadFailed:
            return "Failed to load image";
        case ErrorCode::ImageSaveFailed:
            return "Failed to save image";
        case ErrorCode::ImageTooSmall:
            return "Image is too small to contain embedded data";
        case ErrorCode::ImageCorrupted:
            return "Image data is corrupted or invalid";
        case ErrorCode::UnsupportedImageFormat:
            return "Image format is not supported";
        case ErrorCode::InvalidImageDimensions:
            return "Image dimensions are invalid";
            
        // Capacity errors
        case ErrorCode::InsufficientCapacity:
            return "Image does not have enough capacity for the data";
        case ErrorCode::DataTooLarge:
            return "Data is too large to embed in the image";
            
        // Encryption/Decryption errors
        case ErrorCode::EncryptionFailed:
            return "Encryption operation failed";
        case ErrorCode::DecryptionFailed:
            return "Decryption operation failed";
        case ErrorCode::AuthenticationFailed:
            return "Authentication failed (incorrect password or corrupted data)";
            
        // Embedding/Extraction errors
        case ErrorCode::EmbeddingFailed:
            return "Failed to embed data into image";
        case ErrorCode::ExtractionFailed:
            return "Failed to extract data from image";
        case ErrorCode::InvalidDataSize:
            return "Embedded data size is invalid or corrupted";
        case ErrorCode::CorruptedPayload:
            return "Data structure is corrupted or invalid";
        case ErrorCode::NoEmbeddedData:
            return "No embedded data found in image";
            
        // General errors
        case ErrorCode::UnknownError:
            return "An unknown error occurred";
        case ErrorCode::InvalidArgument:
            return "Invalid argument provided";
        case ErrorCode::NotImplemented:
            return "Feature not implemented";
            
        default:
            return "Undefined error";
    }
}

std::string ErrorHandler::FormatError(ErrorCode code, const std::string& context) {
    std::string message = GetErrorCategory(code) + ": " + GetErrorDescription(code);
    
    if (!context.empty()) {
        message += "\n    Details: " + context;
    }
    
    return message;
}
