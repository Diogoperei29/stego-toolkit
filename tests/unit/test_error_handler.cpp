#include <gtest/gtest.h>
#include "utils/ErrorHandler.h"

TEST(ErrorHandler_Result, ConstructsSuccessWithValue) {
    Result<int> result(42);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.IsError());
    EXPECT_EQ(result.GetValue(), 42);
}

TEST(ErrorHandler_Result, ConstructsErrorWithCode) {
    Result<int> result(ErrorCode::FileNotFound, "File does not exist");
    
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::FileNotFound);
    EXPECT_EQ(result.GetErrorMessage(), "File does not exist");
}

TEST(ErrorHandler_Result, ConstructsVoidSuccess) {
    Result<> result;
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.IsError());
}

TEST(ErrorHandler_Result, ConstructsVoidError) {
    Result<> result(ErrorCode::InvalidArgument, "Invalid input");
    
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorCode(), ErrorCode::InvalidArgument);
    EXPECT_EQ(result.GetErrorMessage(), "Invalid input");
}

TEST(ErrorHandler_Result, GetValueReturnsCorrectValue) {
    Result<int> intResult(123);
    EXPECT_EQ(intResult.GetValue(), 123);
    
    Result<std::string> strResult(std::string("test"));
    EXPECT_EQ(strResult.GetValue(), "test");
    
    std::vector<uint8_t> vec{1, 2, 3, 4};
    Result<std::vector<uint8_t>> vecResult(vec);
    EXPECT_EQ(vecResult.GetValue(), vec);
}

TEST(ErrorHandler_Result, BoolConversionWorks) {
    Result<int> success(42);
    Result<int> error(ErrorCode::UnknownError, "Error");
    
    EXPECT_TRUE(static_cast<bool>(success));
    EXPECT_FALSE(static_cast<bool>(error));
    
    if (success) {
        SUCCEED();
    } else {
        FAIL() << "Success result should convert to true";
    }
    
    if (!error) {
        SUCCEED();
    } else {
        FAIL() << "Error result should convert to false";
    }
}

TEST(ErrorHandler_Result, ErrorCodeIsCorrect) {
    Result<int> r1(ErrorCode::FileNotFound, "msg1");
    EXPECT_EQ(r1.GetErrorCode(), ErrorCode::FileNotFound);
    
    Result<int> r2(ErrorCode::EncryptionFailed, "msg2");
    EXPECT_EQ(r2.GetErrorCode(), ErrorCode::EncryptionFailed);
    
    Result<int> r3(ErrorCode::InsufficientCapacity, "msg3");
    EXPECT_EQ(r3.GetErrorCode(), ErrorCode::InsufficientCapacity);
}

TEST(ErrorHandler_Result, ErrorMessageIsAccessible) {
    const std::string customMessage = "Custom error message with details";
    Result<int> result(ErrorCode::UnknownError, customMessage);
    
    EXPECT_EQ(result.GetErrorMessage(), customMessage);
}

TEST(ErrorHandler_ErrorCode, HasFileIOCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::FileNotFound), 100);
    EXPECT_EQ(static_cast<int>(ErrorCode::FileReadError), 101);
    EXPECT_EQ(static_cast<int>(ErrorCode::FileWriteError), 102);
    EXPECT_GE(static_cast<int>(ErrorCode::FileNotFound), 100);
    EXPECT_LT(static_cast<int>(ErrorCode::FileNotFound), 200);
}

TEST(ErrorHandler_ErrorCode, HasImageCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::ImageLoadFailed), 200);
    EXPECT_EQ(static_cast<int>(ErrorCode::ImageSaveFailed), 201);
    EXPECT_GE(static_cast<int>(ErrorCode::ImageLoadFailed), 200);
    EXPECT_LT(static_cast<int>(ErrorCode::ImageLoadFailed), 300);
}

TEST(ErrorHandler_ErrorCode, HasCapacityCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::InsufficientCapacity), 300);
    EXPECT_GE(static_cast<int>(ErrorCode::InsufficientCapacity), 300);
    EXPECT_LT(static_cast<int>(ErrorCode::InsufficientCapacity), 400);
}

TEST(ErrorHandler_ErrorCode, HasCryptoCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::EncryptionFailed), 400);
    EXPECT_EQ(static_cast<int>(ErrorCode::DecryptionFailed), 401);
    EXPECT_GE(static_cast<int>(ErrorCode::EncryptionFailed), 400);
    EXPECT_LT(static_cast<int>(ErrorCode::EncryptionFailed), 500);
}

TEST(ErrorHandler_ErrorCode, HasEmbeddingCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::EmbeddingFailed), 500);
    EXPECT_EQ(static_cast<int>(ErrorCode::ExtractionFailed), 501);
    EXPECT_GE(static_cast<int>(ErrorCode::EmbeddingFailed), 500);
    EXPECT_LT(static_cast<int>(ErrorCode::EmbeddingFailed), 600);
}

TEST(ErrorHandler_ErrorCode, HasGeneralCodes) {
    EXPECT_EQ(static_cast<int>(ErrorCode::UnknownError), 900);
    EXPECT_EQ(static_cast<int>(ErrorCode::InvalidArgument), 901);
    EXPECT_GE(static_cast<int>(ErrorCode::UnknownError), 900);
    EXPECT_LT(static_cast<int>(ErrorCode::UnknownError), 1000);
}

static Result<int> FunctionThatCanFail(bool shouldFail) {
    if (shouldFail) {
        return Result<int>(ErrorCode::InvalidArgument, "Failed");
    }
    return Result<int>(100);
}

static Result<int> FunctionThatCallsAnother(bool shouldFail) {
    auto result = FunctionThatCanFail(shouldFail);
    if (result.IsError()) {
        return result;
    }
    return Result<int>(result.GetValue() * 2);
}

TEST(ErrorHandler_Usage, SupportsChaining) {
    auto success = FunctionThatCallsAnother(false);
    EXPECT_TRUE(success.IsSuccess());
    EXPECT_EQ(success.GetValue(), 200);
    
    auto error = FunctionThatCallsAnother(true);
    EXPECT_TRUE(error.IsError());
    EXPECT_EQ(error.GetErrorCode(), ErrorCode::InvalidArgument);
}

TEST(ErrorHandler_Usage, SupportsValueExtraction) {
    Result<int> result(42);
    int value = result.GetValue();
    EXPECT_EQ(value, 42);
    
    Result<std::string> strResult(std::string("extracted"));
    std::string str = strResult.GetValue();
    EXPECT_EQ(str, "extracted");
}

TEST(ErrorHandler_Usage, SupportsErrorPropagation) {
    auto innerError = FunctionThatCanFail(true);
    EXPECT_TRUE(innerError.IsError());
    
    auto propagated = FunctionThatCallsAnother(true);
    EXPECT_TRUE(propagated.IsError());
    EXPECT_EQ(propagated.GetErrorCode(), innerError.GetErrorCode());
    EXPECT_EQ(propagated.GetErrorMessage(), innerError.GetErrorMessage());
}

TEST(ErrorHandler_Edge, HandlesEmptyErrorMessage) {
    Result<int> result(ErrorCode::UnknownError, "");
    
    EXPECT_TRUE(result.IsError());
    EXPECT_TRUE(result.GetErrorMessage().empty());
}

TEST(ErrorHandler_Edge, HandlesLongErrorMessage) {
    std::string longMessage(1500, 'x');
    Result<int> result(ErrorCode::UnknownError, longMessage);
    
    EXPECT_TRUE(result.IsError());
    EXPECT_EQ(result.GetErrorMessage().length(), 1500);
    EXPECT_EQ(result.GetErrorMessage(), longMessage);
}
