#include "CryptoModule.h"

#include <iostream>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
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
    
    if (password.empty()) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::InvalidArgument,
            "Password cannot be empty"
        );
    }
    
    // Generate random salt
    std::vector<uint8_t> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to generate random salt: " + GetOpenSSLError()
        );
    }

    // Derive key from password + salt using PBKDF2-HMAC-SHA256
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::KeyDerivationFailed,
            "PBKDF2 key derivation failed: " + GetOpenSSLError()
        );
    }
    
    // Generate random IV
    std::vector<uint8_t> iv(IV_SIZE);
    if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to generate random IV: " + GetOpenSSLError()
        );
    }

    // Create encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to create encryption context: " + GetOpenSSLError()
        );
    }

    // Initialize AES-256-CBC encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        std::string error = GetOpenSSLError();
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Failed to initialize AES-256-CBC: " + error
        );
    }

    // Allocate buffer for ciphertext (plaintext size + one block for padding)
    std::vector<uint8_t> ciphertext(plainData.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, ciphertext_len = 0;

    // Encrypt data
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          plainData.data(), static_cast<int>(plainData.size())) != 1) {
        std::string error = GetOpenSSLError();
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Encryption update failed: " + error
        );
    }
    ciphertext_len = len;

    // Finalize encryption (padding)
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        std::string error = GetOpenSSLError();
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::EncryptionFailed,
            "Encryption finalization failed: " + error
        );
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Build output: [salt | IV | ciphertext]
    std::vector<uint8_t> encryptedData;
    encryptedData.reserve(SALT_SIZE + IV_SIZE + ciphertext_len);
    encryptedData.insert(encryptedData.end(), salt.begin(), salt.end());
    encryptedData.insert(encryptedData.end(), iv.begin(), iv.end());
    encryptedData.insert(encryptedData.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);

    return Result<std::vector<uint8_t>>(encryptedData);
}

Result<std::vector<uint8_t>> CryptoModule::DecryptData(
    const std::vector<uint8_t> &encryptedData,
    const std::string &password) {
    
    // Validate input size
    if (encryptedData.size() < static_cast<std::size_t>(MIN_SIZE)) {
        std::ostringstream oss;
        oss << "Encrypted data too small (" << encryptedData.size() 
            << " bytes). Expected at least " << MIN_SIZE << " bytes.";
        return Result<std::vector<uint8_t>>(
            ErrorCode::CorruptedPayload,
            oss.str()
        );
    }
    
    if (password.empty()) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::InvalidArgument,
            "Password cannot be empty"
        );
    }

    // Extract salt, IV and ciphertext
    std::vector<uint8_t> salt(encryptedData.begin(),
                              encryptedData.begin() + SALT_SIZE);
    std::vector<uint8_t> iv(encryptedData.begin() + SALT_SIZE,
                            encryptedData.begin() + SALT_SIZE + IV_SIZE);
    std::vector<uint8_t> ciphertext(encryptedData.begin() + SALT_SIZE + IV_SIZE,
                                    encryptedData.end());

    // Derive the same key from password + salt
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::KeyDerivationFailed,
            "PBKDF2 key derivation failed: " + GetOpenSSLError()
        );
    }

    // Create decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Failed to create decryption context: " + GetOpenSSLError()
        );
    }

    // Initialize AES-256-CBC decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        std::string error = GetOpenSSLError();
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Failed to initialize AES-256-CBC decryption: " + error
        );
    }

    // Allocate buffer for plaintext
    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, plaintext_len = 0;

    // Decrypt data
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                          ciphertext.data(), static_cast<int>(ciphertext.size())) != 1) {
        std::string error = GetOpenSSLError();
        EVP_CIPHER_CTX_free(ctx);
        return Result<std::vector<uint8_t>>(
            ErrorCode::DecryptionFailed,
            "Decryption update failed: " + error
        );
    }
    plaintext_len = len;

    // Finalize decryption (handles padding removal and authentication)
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        // This typically fails due to wrong password or corrupted data
        return Result<std::vector<uint8_t>>(
            ErrorCode::InvalidPassword,
            "Decryption failed. Likely incorrect password or the corrupted data."
        );
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Resize to actual plaintext length
    plaintext.resize(plaintext_len);

    return Result<std::vector<uint8_t>>(plaintext);
}
