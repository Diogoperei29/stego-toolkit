#include <gtest/gtest.h>
#include "utils/CryptoModule.h"
#include "../test_helpers.h"



// Basic Encryption Tests

TEST(CryptoModule_Encrypt, EncryptsDataSuccessfully) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};
    auto res = CryptoModule::EncryptData( plainData, password );

    EXPECT_TRUE(res.IsSuccess());
    EXPECT_NE(plainData, res.GetValue());
    // 7 bytes -> padded to 16 bytes
    EXPECT_EQ(res.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 16 );
}

TEST(CryptoModule_Encrypt, ProducesDifferentCiphertextEachTime) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};
    auto res1 = CryptoModule::EncryptData( plainData, password );
    auto res2 = CryptoModule::EncryptData( plainData, password );

    EXPECT_NE(res1.GetValue(), res2.GetValue()); 
}

TEST(CryptoModule_Encrypt, HandlesVariousDataSizes) {
    const std::vector<uint8_t> plainData1(1, 1);    // 1 byte
    const std::vector<uint8_t> plainData2(15, 2);   // 15 bytes (less than AES block size)
    const std::vector<uint8_t> plainData3(16, 3);   // 16 bytes (exactly one block)
    const std::vector<uint8_t> plainData4(17, 4);   // 17 bytes (requires padding)
    const std::vector<uint8_t> plainData5(1000, 5); // 1000 bytes (multiple blocks)
    const std::string password{"password"};

    auto res1 = CryptoModule::EncryptData( plainData1, password );
    EXPECT_TRUE(res1.IsSuccess()); 
    // Plaintext size 1 byte -> Padded to 16 bytes
    EXPECT_EQ(res1.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 16 );

    auto res2 = CryptoModule::EncryptData( plainData2, password );
    EXPECT_TRUE(res2.IsSuccess());
    // Plaintext size 15 bytes -> Padded to 16 bytes
    EXPECT_EQ(res2.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 16 );

    auto res3 = CryptoModule::EncryptData( plainData3, password );
    EXPECT_TRUE(res3.IsSuccess());
    // Plaintext size 16 bytes -> Padded to 32 bytes
    EXPECT_EQ(res3.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 32 );

    auto res4 = CryptoModule::EncryptData( plainData4, password );
    EXPECT_TRUE(res4.IsSuccess());
    // Plaintext size 17 bytes -> Padded to 32 bytes
    EXPECT_EQ(res4.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 32 );

    auto res5 = CryptoModule::EncryptData( plainData5, password );
    EXPECT_TRUE(res5.IsSuccess());
    // Plaintext size 1000 bytes -> Padded to 1008 bytes
    EXPECT_EQ(res5.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 1008 );
}

TEST(CryptoModule_Encrypt, RejectsEmptyData) {
    const std::vector<uint8_t> plainData;
    const std::string password{"password"};

    auto res = CryptoModule::EncryptData( plainData, password );

    EXPECT_TRUE(res.IsError());
    EXPECT_EQ(res.GetErrorCode(), ErrorCode::InvalidArgument);
}

TEST(CryptoModule_Encrypt, AcceptsEmptyPassword) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{""};

    auto res = CryptoModule::EncryptData( plainData, password );

    // Should work (PBKDF2 will derive key from empty string)
    EXPECT_TRUE(res.IsSuccess());
    EXPECT_NE(plainData, res.GetValue());
    // 7 bytes -> padded to 16 bytes
    EXPECT_EQ(res.GetValue().size(), CryptoModule::ENCRYPTION_OVERHEAD + 16 );
}

// Basic Decryption Tests

TEST(CryptoModule_Decrypt, DecryptsDataSuccessfully) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_Decrypt, RequiresCorrectPassword) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};
    const std::string wrongPassword{"wrong password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), wrongPassword );
    EXPECT_TRUE(res_dencrypted.IsError());
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::InvalidPassword);
}

TEST(CryptoModule_Decrypt, DetectsCorruptedCiphertext) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Corrupt first byte in ciphertext
    res_encrypted.GetValue()[CryptoModule::ENCRYPTION_OVERHEAD] += 1; // if it overflows, goes to 0

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsError());
    // Corrupted ciphertext causes padding verification failure, which returns InvalidPassword
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::InvalidPassword);
}

TEST(CryptoModule_Decrypt, RejectsTooSmallCiphertext) {
    const std::vector<uint8_t> ciphertext(CryptoModule::MIN_SIZE - 1, 1);
    const std::string password{"password"};

    auto res_dencrypted = CryptoModule::DecryptData( ciphertext, password );
    EXPECT_TRUE(res_dencrypted.IsError());
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::CorruptedPayload);
}

TEST(CryptoModule_Decrypt, HandlesInvalidPadding) {
        const std::vector<uint8_t> ciphertext(CryptoModule::MIN_SIZE + 1, 1); // not %16 
    const std::string password{"password"};

    auto res_dencrypted = CryptoModule::DecryptData( ciphertext, password );
    EXPECT_TRUE(res_dencrypted.IsError());
    // Invalid padding causes padding verification failure, which returns InvalidPassword
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::InvalidPassword);
}

// Round-Trip Tests (Encrypt + Decrypt)

TEST(CryptoModule_RoundTrip, SmallTextRoundTrip) {
    std::string str = "Hello, World!";
    const std::vector<uint8_t> plainData (str.begin(), str.end());
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
    
    std::string decryptedStr(res_dencrypted.GetValue().begin(), res_dencrypted.GetValue().end());
    EXPECT_EQ(decryptedStr, str);
}

TEST(CryptoModule_RoundTrip, BinaryDataRoundTrip) {
    std::vector<uint8_t> binaryData = TestHelpers::ReadBinaryFile( TestHelpers::GetFixturePath("binary_data.bin")) ;
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( binaryData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(binaryData, res_dencrypted.GetValue());
}

TEST(CryptoModule_RoundTrip, UnicodeRoundTrip) {
    std::string str = TestHelpers::ReadTextFile( TestHelpers::GetFixturePath("unicode.txt")) ;
    const std::vector<uint8_t> plainData (str.begin(), str.end());
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());

    std::string decryptedStr(res_dencrypted.GetValue().begin(), res_dencrypted.GetValue().end());
    EXPECT_EQ(decryptedStr, str);
}

TEST(CryptoModule_RoundTrip, LargeDataRoundTrip) {
    std::string str = TestHelpers::ReadTextFile( TestHelpers::GetFixturePath("large.txt")) ;
    const std::vector<uint8_t> plainData (str.begin(), str.end());
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());

    std::string decryptedStr(res_dencrypted.GetValue().begin(), res_dencrypted.GetValue().end());
    EXPECT_EQ(decryptedStr, str);
}

TEST(CryptoModule_RoundTrip, SingleByteRoundTrip) {
    const std::vector<uint8_t> plainData (1, 1);
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_RoundTrip, ExactBlockSizeRoundTrip) {
    const std::vector<uint8_t> plainData (16, 1); // One AES block
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

// Password Strength Tests

TEST(CryptoModule_Password, WorksWithShortPassword) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"a"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_Password, WorksWithLongPassword) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password(100, 'a');

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_Password, WorksWithSpecialCharacters) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"!@#$%^&*(){}[]|;:\'\",.<>?"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_Password, WorksWithUnicodePassword) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"Пр你こ🌍£©®🔒"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsSuccess());
    EXPECT_EQ(plainData, res_dencrypted.GetValue());
}

TEST(CryptoModule_Password, IsCaseSensitive) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};
    const std::string wrongPassword{"Password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), wrongPassword );
    EXPECT_TRUE(res_dencrypted.IsError());
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::InvalidPassword);
}

// Cryptographic Properties Tests

TEST(CryptoModule_Crypto, UsesRandomSalt) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};

    auto res1 = CryptoModule::EncryptData(plainData, password);
    EXPECT_TRUE(res1.IsSuccess());
    auto res2 = CryptoModule::EncryptData(plainData, password);
    EXPECT_TRUE(res2.IsSuccess());

    std::vector<uint8_t> salt1(res1.GetValue().begin(), 
                                res1.GetValue().begin() + CryptoModule::SALT_SIZE);
    std::vector<uint8_t> salt2(res2.GetValue().begin(), 
                                res2.GetValue().begin() + CryptoModule::SALT_SIZE);
    
    // Salts should be different
    EXPECT_NE(salt1, salt2);
}

TEST(CryptoModule_Crypto, UsesRandomIV) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};

    auto res1 = CryptoModule::EncryptData(plainData, password);
    EXPECT_TRUE(res1.IsSuccess());
    auto res2 = CryptoModule::EncryptData(plainData, password);
    EXPECT_TRUE(res2.IsSuccess());

    std::vector<uint8_t> iv1(res1.GetValue().begin() + CryptoModule::SALT_SIZE, 
                             res1.GetValue().begin() + CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE);
    std::vector<uint8_t> iv2(res2.GetValue().begin() + CryptoModule::SALT_SIZE, 
                             res2.GetValue().begin() + CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE);
    
    // IVs should be different
    EXPECT_NE(iv1, iv2);
}

// Error Reporting Tests

TEST(CryptoModule_Errors, ReturnsCorrectErrorCodes) {
    const std::string password{"password"};
    
    // Empty data should produce InvalidArgument error
    std::vector<uint8_t> emptyData;
    auto res1 = CryptoModule::EncryptData(emptyData, password);
    EXPECT_TRUE(res1.IsError());
    EXPECT_EQ(res1.GetErrorCode(), ErrorCode::InvalidArgument);
    
    // Too small ciphertext should produce CorruptedPayload error
    std::vector<uint8_t> tooSmall(10, 0x00);
    auto res2 = CryptoModule::DecryptData(tooSmall, password);
    EXPECT_TRUE(res2.IsError());
    EXPECT_EQ(res2.GetErrorCode(), ErrorCode::CorruptedPayload);
}

// Compatibility Tests

TEST(CryptoModule_Compat, DecryptsDataEncryptedEarlier) {
    const std::string password{"password"};
    const std::string expectedPlaintext{"Hello, World!"};

    const std::vector<uint8_t> preEncrypted{
        0x31, 0xB4, 0x53, 0xF6, 0xF1, 0xA1, 0x92, 0x34, 0x4E, 0x26, 0xEA, 0xAB, 0xEA, 0x41, 0xF6, 0xC5, 
        0xA3, 0xBD, 0xFE, 0xC5, 0x10, 0xF9, 0x3B, 0xC3, 0xD4, 0xA3, 0x2A, 0x98, 0x30, 0xD3, 0x7A, 0x52, 
        0x04, 0x72, 0x16, 0x86, 0xEB, 0x60, 0xAF, 0x6C, 0x20, 0xD8, 0x0C, 0xC8, 0x25, 0x12, 0x6B, 0xAF
    };
    
    auto decrypted = CryptoModule::DecryptData(preEncrypted, password);
    EXPECT_TRUE(decrypted.IsSuccess());

    std::string result(decrypted.GetValue().begin(), decrypted.GetValue().end());
    EXPECT_EQ(result, expectedPlaintext);
}
