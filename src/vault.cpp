#include "vault.h"
#include "steganography.h"
#include "crypto.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

// Magic bytes that identify a SISFA vault payload
static const uint8_t MAGIC[4] = {'S', 'I', 'S', 'F'};
static const uint8_t VERSION   = 2;

// helpers

static std::vector<uint8_t> readFile(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
        return {};
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),
                                std::istreambuf_iterator<char>());
}

bool embedVault(const std::string& carrierImagePath,
                const std::string& secretFilePath,
                const std::string& password,
                const std::string& outputImagePath)
{
    // 1. Read secret file
    std::vector<uint8_t> secretBytes = readFile(secretFilePath);
    if (secretBytes.empty())
    {
        std::cout << "ERROR: Could not read secret file: " << secretFilePath << std::endl;
        return false;
    }

    // 2. Load carrier image
    int width, height, channels;
    uint8_t* rawPixels = stbi_load(carrierImagePath.c_str(), &width, &height, &channels, 3);
    if (!rawPixels)
    {
        std::cout << "ERROR: Could not load image: " << carrierImagePath << std::endl;
        return false;
    }

    std::vector<uint8_t> pixels(rawPixels, rawPixels + (static_cast<size_t>(width) * height * 3));
    stbi_image_free(rawPixels);

    // Get just the filename without the path
    std::string filename = fs::path(secretFilePath).filename().string();

    // 3. Check capacity early (before encrypting)
    // Rough estimate: payload = header overhead + encrypted size (~secretBytes + 48 for salt/iv/padding)
    size_t estimatedPayload = 4 + 1 + 2 + secretFilePath.size() + secretBytes.size() + 64;
    if (estimatedPayload > maxEmbeddableBytes(pixels.size()))
    {
        std::cout << "ERROR: Image too small for this file.\n"
                  << "  Image capacity : " << maxEmbeddableBytes(pixels.size()) << " bytes\n"
                  << "  Estimated need : " << estimatedPayload << " bytes\n"
                  << "  Use a larger carrier image." << std::endl;
        return false;
    }

    // 4. Build plaintext blob: [filename_len(2)][filename][filedata]
    uint16_t filenameLen = static_cast<uint16_t>(filename.size());

    std::vector<uint8_t> plainBlob;
    // filename length (2 bytes, big-endian)
    plainBlob.push_back(static_cast<uint8_t>(filenameLen >> 8));
    plainBlob.push_back(static_cast<uint8_t>(filenameLen & 0xFF));
    // filename
    plainBlob.insert(plainBlob.end(), filename.begin(), filename.end());
    // actual file bytes
    plainBlob.insert(plainBlob.end(), secretBytes.begin(), secretBytes.end());

    // 5. Encrypt the entire blob (filename + filedata together)
    std::vector<uint8_t> encrypted = encryptData(plainBlob, password);
    if (encrypted.empty())
    {
        std::cout << "ERROR: Encryption failed." << std::endl;
        return false;
    }

    // 6. Build final payload: [magic][version][encrypted blob]
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), MAGIC, MAGIC + 4);
    payload.push_back(VERSION);
    payload.insert(payload.end(), encrypted.begin(), encrypted.end());

    if (payload.size() * 8 + 32 > pixels.size())
    {
        std::cout << "ERROR: Payload is too large for this image.\n"
                  << "  Image capacity : " << maxEmbeddableBytes(pixels.size()) << " bytes\n"
                  << "  Payload size   : " << payload.size() << " bytes" << std::endl;
        return false;
    }

    // 7. Embed into pixels
    embedData(pixels, payload);

    // 8. Write output image
    // Make sure the output directory exists
    fs::path outPath(outputImagePath);
    if (outPath.has_parent_path())
        fs::create_directories(outPath.parent_path());

    if (!stbi_write_png(outputImagePath.c_str(), width, height, 3, pixels.data(), width * 3))
    {
        std::cout << "ERROR: Could not write output image: " << outputImagePath << std::endl;
        return false;
    }

    std::cout << "Vault embedded successfully.\n"
              << "  File    : " << filename << " (" << secretBytes.size() << " bytes)\n"
              << "  Output  : " << outputImagePath << std::endl;
    return true;
}

// extractVault

VaultResult extractVault(const std::string& stegoImagePath,
                          const std::string& password)
{
    VaultResult result{false, "", {}, ""};

    // 1. Load stego image
    int width, height, channels;
    uint8_t* rawPixels = stbi_load(stegoImagePath.c_str(), &width, &height, &channels, 3);
    if (!rawPixels)
    {
        result.error = "Could not load image: " + stegoImagePath;
        return result;
    }

    std::vector<uint8_t> pixels(rawPixels, rawPixels + (static_cast<size_t>(width) * height * 3));
    stbi_image_free(rawPixels);

    // 2. Extract raw payload
    std::vector<uint8_t> payload = extractData(pixels);
    if (payload.size() < 7) // minimum: 4 magic + 1 version + 2 filename_len
    {
        result.error = "No vault found in this image (payload too small).";
        return result;
    }

    // 3. Check magic bytes
    if (payload[0] != MAGIC[0] || payload[1] != MAGIC[1] ||
        payload[2] != MAGIC[2] || payload[3] != MAGIC[3])
    {
        result.error = "No SISFA vault found in this image.";
        return result;
    }

    // 4. Parse header — new format: [magic(4)][version(1)][encrypted blob]
    size_t offset = 4;
    uint8_t version = payload[offset++];

    if (version != VERSION)
    {
        result.error = "Unsupported vault version: " + std::to_string(version);
        return result;
    }

    // 5. Decrypt the entire blob
    std::vector<uint8_t> encryptedBlob(payload.begin() + offset, payload.end());
    std::vector<uint8_t> decrypted = decryptData(encryptedBlob, password);

    if (decrypted.empty())
    {
        result.error = "Wrong password or corrupted vault.";
        return result;
    }

    // 6. Parse decrypted blob: [filename_len(2)][filename][filedata]
    if (decrypted.size() < 2)
    {
        result.error = "Decrypted blob too small - corrupted vault.";
        return result;
    }

    uint16_t filenameLen = (static_cast<uint16_t>(decrypted[0]) << 8) | decrypted[1];
    offset = 2;

    if (offset + filenameLen > decrypted.size())
    {
        result.error = "Filename length corrupted.";
        return result;
    }

    std::string filename(decrypted.begin() + offset, decrypted.begin() + offset + filenameLen);
    offset += filenameLen;

    result.success  = true;
    result.filename = filename;
    result.data     = std::vector<uint8_t>(decrypted.begin() + offset, decrypted.end());
    return result;
}