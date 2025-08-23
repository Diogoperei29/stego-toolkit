#ifndef CRYPTOMODULE_H
#define CRYPTOMODULE_H

#include <vector>
#include <string>
#include <cstdint>

/**
 * @brief Provides AES-256-CBC encryption and decryption utilities.
 *
 * Uses PBKDF2-HMAC-SHA256 for key derivation with a random salt,
 * and AES-256-CBC for encryption/decryption with a random IV.
 */
class CryptoModule
{
public:
    /**
    * @brief Encrypts data with AES-256-CBC using a password.
    *
    * The output format is: [salt | iv | ciphertext].
    *
    * @param plainData Input plaintext data.
    * @param password Password used for key derivation.
    * @param encryptedData Output encrypted data.
    * @return true if encryption succeeded, false otherwise.
    */
    static bool EncryptData(const std::vector<uint8_t> &plainData,
                            const std::string &password,
                            std::vector<uint8_t> &encryptedData);

    /**
    * @brief Decrypts data with AES-256-CBC using a password.
    *
    * Expects input format: [salt | iv | ciphertext].
    *
    * @param encryptedData Input encrypted data.
    * @param password Password used for key derivation.
    * @param plainData Output decrypted plaintext data.
    * @return true if decryption succeeded, false otherwise.
    */
    static bool DecryptData(const std::vector<uint8_t> &encryptedData,
                            const std::string &password,
                            std::vector<uint8_t> &plainData);
};

#endif // CRYPTOMODULE_H