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

TEST(CryptoModule_Encrypt, OutputHasCorrectHMACStructure) {
    const std::vector<uint8_t> plainData{1, 2, 3, 4, 5};
    const std::string password{"password"};
    auto res = CryptoModule::EncryptData( plainData, password );

    EXPECT_TRUE(res.IsSuccess());
    const auto& encrypted = res.GetValue();
    
    // Verify size: salt + IV + padded_ciphertext + HMAC
    size_t min_expected = CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE 
                        + 16 + CryptoModule::HMAC_SIZE;  // Minimum with 16-byte block
    EXPECT_GE(encrypted.size(), min_expected);
    
    // Verify overhead constant is correct
    EXPECT_EQ(CryptoModule::ENCRYPTION_OVERHEAD, 
              CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE + CryptoModule::HMAC_SIZE);
    EXPECT_EQ(CryptoModule::ENCRYPTION_OVERHEAD, 64);
    
    // Verify format: [salt(16) | IV(16) | ciphertext | HMAC(32)]
    // Last 32 bytes should be HMAC
    EXPECT_GE(encrypted.size(), CryptoModule::HMAC_SIZE);
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
    // Wrong password causes HMAC verification to fail
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

TEST(CryptoModule_Decrypt, DetectsCorruptedCiphertext) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 2, 1, 0};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Corrupt first byte of ciphertext (after salt and IV)
    res_encrypted.GetValue()[CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE] += 1;

    auto res_dencrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_dencrypted.IsError());

    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

// HMAC Tampering Detection Tests

TEST(CryptoModule_HMAC, DetectsSaltTampering) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 4, 5};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Tamper with salt (first 16 bytes)
    res_encrypted.GetValue()[0] ^= 0xFF;

    auto res_decrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_decrypted.IsError());

    EXPECT_EQ(res_decrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

TEST(CryptoModule_HMAC, DetectsIVTampering) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 4, 5};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Tamper with IV (bytes 16-31)
    res_encrypted.GetValue()[CryptoModule::SALT_SIZE] ^= 0xFF;

    auto res_decrypted = CryptoModule::DecryptData( res_encrypted.GetValue(), password );
    EXPECT_TRUE(res_decrypted.IsError());

    EXPECT_EQ(res_decrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

TEST(CryptoModule_HMAC, DetectsHMACTampering) {
    const std::vector<uint8_t> plainData{0, 1, 2, 3, 4, 5};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Tamper with HMAC (last 32 bytes)
    auto& encrypted = res_encrypted.GetValue();
    encrypted[encrypted.size() - 1] ^= 0xFF;

    auto res_decrypted = CryptoModule::DecryptData( encrypted, password );
    EXPECT_TRUE(res_decrypted.IsError());

    EXPECT_EQ(res_decrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

TEST(CryptoModule_HMAC, DetectsMultipleByteCorruption) {
    const std::vector<uint8_t> plainData{1, 2, 3, 4, 5};
    const std::string password{"password"};

    auto res_encrypted = CryptoModule::EncryptData( plainData, password );
    EXPECT_TRUE(res_encrypted.IsSuccess());

    // Corrupt multiple bytes in ciphertext portion
    auto& encrypted = res_encrypted.GetValue();
    size_t cipher_start = CryptoModule::SALT_SIZE + CryptoModule::IV_SIZE;
    size_t cipher_end = encrypted.size() - CryptoModule::HMAC_SIZE;
    
    if (cipher_end > cipher_start + 2) {
        encrypted[cipher_start] ^= 0x01;
        encrypted[cipher_start + 1] ^= 0x02;
        encrypted[cipher_start + 2] ^= 0x03;
    }

    auto res_decrypted = CryptoModule::DecryptData( encrypted, password );
    EXPECT_TRUE(res_decrypted.IsError());

    EXPECT_EQ(res_decrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
}

TEST(CryptoModule_Decrypt, RejectsTooSmallCiphertext) {
    const std::vector<uint8_t> ciphertext(CryptoModule::MIN_SIZE - 1, 1);
    const std::string password{"password"};

    auto res_dencrypted = CryptoModule::DecryptData( ciphertext, password );
    EXPECT_TRUE(res_dencrypted.IsError());
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::CorruptedPayload);
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
    // Different password causes HMAC verification to fail
    EXPECT_EQ(res_dencrypted.GetErrorCode(), ErrorCode::AuthenticationFailed);
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

// Compatibility Test - Verifies ability to decrypt data encrypted with current format

TEST(CryptoModule_Compat, DecryptsDataEncryptedWithHMAC) {
    // Pre-encrypted "Hello, World!" with password "password" using current format
    // Format: [salt(16) | IV(16) | ciphertext(16) | HMAC(32)] = 80 bytes
    std::vector<uint8_t> preEncryptedData = {
        0xa1, 0xea, 0x69, 0xf5, 0x1a, 0x16, 0x95, 0xda, 0x88, 0xe8, 0x93, 0xa0,
        0xe4, 0xc5, 0x40, 0x27, 0x6f, 0xaf, 0x91, 0xe6, 0x13, 0x13, 0x07, 0x11,
        0x0f, 0xfc, 0x4d, 0x80, 0x48, 0x29, 0x9f, 0xb8, 0xdc, 0x55, 0xa4, 0x4b,
        0x28, 0x7a, 0xd5, 0x4b, 0x0a, 0x1e, 0x16, 0xa8, 0x70, 0xb4, 0x15, 0x3b,
        0x7d, 0x48, 0x56, 0xf6, 0x01, 0x1b, 0x78, 0xc7, 0x6a, 0xb6, 0x9f, 0x1c,
        0xf5, 0x02, 0x31, 0x68, 0xf8, 0x14, 0x3a, 0x93, 0x98, 0x15, 0x14, 0x80,
        0x6d, 0x66, 0xa9, 0xf8, 0x77, 0x42, 0x52, 0x24
    };

    const std::string password = "password";
    const std::string expectedPlaintext = "Hello, World!";

    auto result = CryptoModule::DecryptData(preEncryptedData, password);

    ASSERT_TRUE(result.IsSuccess());
    std::string decrypted(result.GetValue().begin(), result.GetValue().end());
    EXPECT_EQ(decrypted, expectedPlaintext);
}


