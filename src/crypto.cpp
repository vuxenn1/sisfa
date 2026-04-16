#include "crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>

const int KEY_SIZE = 32; // 256 bits for AES-256
const int IV_SIZE = 16;  // 128 bits for AES block size
const int SALT_SIZE = 16; // 128 bits for salt
const int PBKDF2_ITERATIONS = 100000;

std::vector<uint8_t> encryptData(const std::vector<uint8_t>& plainData, const std::string& password)
{
    // Salt makes PBKDF2 output different every time
    // even if the same password is used
    std::vector<uint8_t> salt(SALT_SIZE);
    std::vector<uint8_t> iv(IV_SIZE);

    // RAND_bytes fills our vectors with cryptographically secure random bytes
    RAND_bytes(salt.data(), SALT_SIZE);
    RAND_bytes(iv.data(), IV_SIZE);

    std::vector<uint8_t> key(KEY_SIZE);

    PKCS5_PBKDF2_HMAC(
        password.c_str(),           // password as C string
        password.size(),            // password length
        salt.data(),                // random salt
        SALT_SIZE,                  // salt length
        PBKDF2_ITERATIONS,          // number of iterations
        EVP_sha256(),               // hashing algorithm
        KEY_SIZE,                   // output key length (32 bytes)
        key.data()                  // where to store the key
    );

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        std::cout << "ERROR: Could not create cipher context!" << std::endl;
        return {};
    }

    EVP_EncryptInit_ex(
        ctx,                // our context
        EVP_aes_256_cbc(),  // algorithm: AES-256-CBC
        nullptr,            // engine (nullptr = use default)
        key.data(),         // our derived key
        iv.data()           // our random IV
    );

    // Output buffer needs extra space for padding
    std::vector<uint8_t> cipherText(plainData.size() + IV_SIZE);
    int bytesWritten = 0;
    int totalBytes   = 0;

    EVP_EncryptUpdate(
        ctx,
        cipherText.data(),          // output buffer
        &bytesWritten,              // how many bytes were written
        plainData.data(),           // input data
        plainData.size()            // input size
    );
    totalBytes += bytesWritten;

    EVP_EncryptFinal_ex(ctx, cipherText.data() + totalBytes, &bytesWritten);
    totalBytes += bytesWritten;
    cipherText.resize(totalBytes);

    EVP_CIPHER_CTX_free(ctx);

    // Final format: [salt (16 bytes)][iv (16 bytes)][encrypted data]
    // We store salt and IV at the front so decryption can find them
    std::vector<uint8_t> output;
    output.insert(output.end(), salt.begin(), salt.end());
    output.insert(output.end(), iv.begin(), iv.end());
    output.insert(output.end(), cipherText.begin(), cipherText.end());

    return output;
}
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& encryptedData, const std::string& password)
{
    // We need at least salt + IV bytes to even start
    if (encryptedData.size() < SALT_SIZE + IV_SIZE)
    {
        std::cout << "ERROR: Data too small to be valid!" << std::endl;
        return {};
    }

    // Remember our format: [salt (16)][iv (16)][encrypted data]
    std::vector<uint8_t> salt(encryptedData.begin(), encryptedData.begin() + SALT_SIZE);
    std::vector<uint8_t> iv(encryptedData.begin() + SALT_SIZE, encryptedData.begin() + SALT_SIZE + IV_SIZE);
    std::vector<uint8_t> cipherText(encryptedData.begin() + SALT_SIZE + IV_SIZE, encryptedData.end());

    std::vector<uint8_t> key(KEY_SIZE);

    PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.size(),
        salt.data(),
        SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        KEY_SIZE,
        key.data()
    );

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        std::cout << "ERROR: Could not create cipher context!" << std::endl;
        return {};
    }

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

    std::vector<uint8_t> plainText(cipherText.size());
    int bytesWritten = 0;
    int totalBytes   = 0;

    EVP_DecryptUpdate(
        ctx,
        plainText.data(),
        &bytesWritten,
        cipherText.data(),
        cipherText.size()
    );
    totalBytes += bytesWritten;

    int result = EVP_DecryptFinal_ex(ctx, plainText.data() + totalBytes, &bytesWritten);
    totalBytes += bytesWritten;
    plainText.resize(totalBytes);

    EVP_CIPHER_CTX_free(ctx);

    if (result != 1)
    {
        std::cout << "ERROR: Decryption failed! Wrong password?" << std::endl;
        return {};
    }

    return plainText;
}