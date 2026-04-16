#pragma once

#include <string>
#include <vector>
#include <cstdint>

// Payload layout embedded inside the image:
//   [magic "SISF" - 4 bytes]
//   [version      - 1 byte ]
//   [filename_len - 2 bytes]
//   [filename     - filename_len bytes]
//   [encrypted_blob - rest (output of encryptData)]
//
// The encrypted_blob contains the raw file bytes encrypted with AES-256-CBC.

struct VaultResult
{
    bool        success;
    std::string filename;       // original filename recovered from the vault
    std::vector<uint8_t> data;  // decrypted file bytes
    std::string error;          // human-readable error if success == false
};

// Reads secretFilePath, encrypts it with password, embeds into carrierImagePath.
// Writes the resulting stego PNG to outputImagePath.
// Returns true on success, false on failure (prints reason to stdout).
bool embedVault(const std::string& carrierImagePath,
                const std::string& secretFilePath,
                const std::string& password,
                const std::string& outputImagePath);

// Loads stegoImagePath, extracts the hidden vault, decrypts with password.
// Returns a VaultResult. On success, write result.data to output/<result.filename>.
VaultResult extractVault(const std::string& stegoImagePath, const std::string& password);
