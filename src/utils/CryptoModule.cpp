#include "CryptoModule.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/err.h>

std::string CryptoModule::GetOpenSSLError() {
    BIO *bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);
    char *buf;
    std::size_t len = BIO_get_mem_data(bio, &buf);
    std::string errorStr(buf, len);
    BIO_free(bio);
    return errorStr;
}

Result<std::vector<uint8_t>> CryptoModule::EncryptData(
    const std::vector<uint8_t> &plainData,
    const std::string &password) {
    
    // Validate input
    if (plainData.empty()) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::InvalidArgument,
            "Cannot encrypt empty data"
        );
    }
    
    // Generate random salt and IV
    std::vector<uint8_t> salt(SALT_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1 || RAND_bytes(iv.data(), IV_SIZE) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Cryptographic random number generation failed"
        );
    }

    // Derive key from password + salt using PBKDF2-HMAC-SHA256
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Key derivation failed"
        );
    }

    // Create and initialize encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to create encryption context"
        );
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to initialize AES-256-CBC encryption"
        );
    }

    // Perform encryption
    std::vector<uint8_t> ciphertext(plainData.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          plainData.data(), static_cast<int>(plainData.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Encryption failed"
        );
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Encryption failed"
        );
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Resize ciphertext to actual length
    ciphertext.resize(ciphertext_len);

    // Build authenticated data: [salt | IV | ciphertext]
    std::vector<uint8_t> dataToAuth;
    dataToAuth.reserve(SALT_SIZE + IV_SIZE + ciphertext_len);
    dataToAuth.insert(dataToAuth.end(), salt.begin(), salt.end());
    dataToAuth.insert(dataToAuth.end(), iv.begin(), iv.end());
    dataToAuth.insert(dataToAuth.end(), ciphertext.begin(), ciphertext.end());

    // Compute HMAC-SHA256 over [salt | IV | ciphertext] using derived key
    std::vector<uint8_t> hmac(HMAC_SIZE);
    uint32_t hmac_len = 0;
    if (HMAC(EVP_sha256(), key.data(), KEY_SIZE,
             dataToAuth.data(), dataToAuth.size(),
             hmac.data(), &hmac_len) == nullptr || hmac_len != HMAC_SIZE) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "HMAC computation failed"
        );
    }

    // Build final output: [salt | IV | ciphertext | HMAC]
    std::vector<uint8_t> encryptedData;
    encryptedData.reserve(SALT_SIZE + IV_SIZE + ciphertext_len + HMAC_SIZE);
    encryptedData.insert(encryptedData.end(), salt.begin(), salt.end());
    encryptedData.insert(encryptedData.end(), iv.begin(), iv.end());
    encryptedData.insert(encryptedData.end(), ciphertext.begin(), ciphertext.end());
    encryptedData.insert(encryptedData.end(), hmac.begin(), hmac.end());

    return Result<std::vector<uint8_t>>(encryptedData);
}

Result<std::vector<uint8_t>> CryptoModule::DecryptData(
    const std::vector<uint8_t> &encryptedData,
    const std::string &password) {
    
    // Validate input size (salt + IV + HMAC + at least one block)
    if (encryptedData.size() < static_cast<std::size_t>(MIN_SIZE)) {
        std::ostringstream oss;
        oss << "Data too small for decryption (" << encryptedData.size() 
            << " bytes, expected at least " << MIN_SIZE << ")";
        return Result<std::vector<uint8_t>>(
            ErrorCode::CorruptedPayload,
            oss.str()
        );
    }

    std::vector<uint8_t> salt(encryptedData.begin(), encryptedData.begin() + SALT_SIZE);
    std::vector<uint8_t> iv(encryptedData.begin() + SALT_SIZE, 
                            encryptedData.begin() + SALT_SIZE + IV_SIZE);
    
    // Ciphertext is between IV and HMAC
    const size_t ciphertext_start = SALT_SIZE + IV_SIZE;
    const size_t ciphertext_end = encryptedData.size() - HMAC_SIZE;
    std::vector<uint8_t> ciphertext(encryptedData.begin() + ciphertext_start,
                                    encryptedData.begin() + ciphertext_end);
    
    // HMAC is the last HMAC_SIZE bytes
    std::vector<uint8_t> receivedHmac(encryptedData.end() - HMAC_SIZE, encryptedData.end());

    // Derive key from password + salt
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Key derivation failed"
        );
    }

    // Verify HMAC before decrypting (Encrypt-then-MAC)
    // Compute HMAC over [salt | IV | ciphertext]
    std::vector<uint8_t> dataToAuth;
    dataToAuth.reserve(SALT_SIZE + IV_SIZE + ciphertext.size());
    dataToAuth.insert(dataToAuth.end(), salt.begin(), salt.end());
    dataToAuth.insert(dataToAuth.end(), iv.begin(), iv.end());
    dataToAuth.insert(dataToAuth.end(), ciphertext.begin(), ciphertext.end());

    std::vector<uint8_t> computedHmac(HMAC_SIZE);
    unsigned int hmac_len = 0;
    if (HMAC(EVP_sha256(), key.data(), KEY_SIZE,
             dataToAuth.data(), dataToAuth.size(),
             computedHmac.data(), &hmac_len) == nullptr || hmac_len != HMAC_SIZE) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "HMAC computation failed"
        );
    }

    // Constant-time comparison to prevent timing attacks
    if (CRYPTO_memcmp(computedHmac.data(), receivedHmac.data(), HMAC_SIZE) != 0) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::AuthenticationFailed,
            "HMAC verification failed (incorrect password or corrupted data)"
        );
    }

    // Create and initialize decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Failed to create decryption context"
        );
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Failed to initialize AES-256-CBC decryption"
        );
    }

    // Perform decryption
    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                          ciphertext.data(), static_cast<int>(ciphertext.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Decryption failed"
        );
    }
    plaintext_len = len;

    // Finalize decryption and verify padding
    // Note: HMAC already verified, so padding errors indicate implementation bugs or edge cases
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Decryption padding verification failed"
        );
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Resize to actual plaintext length
    plaintext.resize(plaintext_len);

    return Result<std::vector<uint8_t>>(plaintext);
}
