#include "CryptoModule.h"

#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Encrypts data with AES-256-CBC using a password
bool CryptoModule::EncryptData(const std::vector<uint8_t> &plainData,
                               const std::string &password,
                               std::vector<uint8_t> &encryptedData) {
    const int SALT_SIZE = 16;   // Salt size for PBKDF2
    const int IV_SIZE   = 16;   // AES block size (128-bit IV)
    const int KEY_SIZE  = 32;   // AES-256

    // Generate a random salt
    std::vector<uint8_t> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Derive a 256-bit key from the password + salt using PBKDF2-HMAC-SHA256
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          10000, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        ERR_print_errors_fp(stderr);
        return false;
    }
    
    // Generate a random IV
    std::vector<uint8_t> iv(IV_SIZE);
    if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Create and initialize the encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Initialize AES-256-CBC encryption with derived key and random IV
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Allocate buffer for ciphertext
    std::vector<uint8_t> ciphertext(plainData.size() + IV_SIZE);
    int len = 0, ciphertext_len = 0;

    // Encrypt the plaintext in chunks
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          plainData.data(), static_cast<int>(plainData.size())) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Final output format: [salt | iv | ciphertext]
    encryptedData.clear();
    encryptedData.insert(encryptedData.end(), salt.begin(), salt.end());
    encryptedData.insert(encryptedData.end(), iv.begin(), iv.end());
    encryptedData.insert(encryptedData.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);

    return true;
}

// Decrypts data with AES-256-CBC using a password
bool CryptoModule::DecryptData(const std::vector<uint8_t> &encryptedData,
                               const std::string &password,
                               std::vector<uint8_t> &plainData) {
    const int SALT_SIZE = 16;   // Salt size for PBKDF2
    const int IV_SIZE   = 16;   // AES block size (128-bit IV)
    const int KEY_SIZE  = 32;   // AES-256

    // Check minimum size
    if (encryptedData.size() < SALT_SIZE + IV_SIZE) {
        std::cerr << "Encrypted data too small." << std::endl;
        return false;
    }

    // Extract salt and IV
    std::vector<uint8_t> salt(encryptedData.begin(),
                              encryptedData.begin() + SALT_SIZE);
    std::vector<uint8_t> iv(encryptedData.begin() + SALT_SIZE,
                            encryptedData.begin() + SALT_SIZE + IV_SIZE);

    // Extract ciphertext
    std::vector<uint8_t> ciphertext(encryptedData.begin() + SALT_SIZE + IV_SIZE,
                                    encryptedData.end());

    // Derive the same 256-bit key from password + salt
    std::vector<uint8_t> key(KEY_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt.data(), SALT_SIZE,
                          10000, EVP_sha256(),
                          KEY_SIZE, key.data()) != 1) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Create and initialize the decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Allocate buffer for plaintext
    std::vector<uint8_t> plaintext(ciphertext.size() + IV_SIZE);
    int len = 0, plaintext_len = 0;

    // Decrypt ciphertext
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                          ciphertext.data(), static_cast<int>(ciphertext.size())) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len = len;

    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        ERR_print_errors_fp(stderr);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Resize plaintext to actual length
    plainData.assign(plaintext.begin(), plaintext.begin() + plaintext_len);

    return true;
}