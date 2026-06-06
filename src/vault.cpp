#include "vault.h"
#include "steganography.h"
#include "crypto.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static const uint8_t MAGIC[4] = {'S', 'I', 'S', 'F'};
static const uint8_t VERSION  = 2;

// Read a file into bytes directly using fs::path (handles UTF-8 paths)
static std::vector<uint8_t> readFile(const fs::path& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(f),
                                std::istreambuf_iterator<char>());
}

// Load image from path that may contain Unicode characters
static uint8_t* loadImage(const fs::path& path, int* width, int* height, int* channels)
{
    std::vector<uint8_t> fileBytes = readFile(path);
    if (fileBytes.empty()) return nullptr;
    return stbi_load_from_memory(
        fileBytes.data(),
        static_cast<int>(fileBytes.size()),
        width, height, channels, 3
    );
}

// Callback for stbi_write_png_to_func — appends bytes to a vector
static void pngWriteCallback(void* context, void* data, int size)
{
    auto* buffer = reinterpret_cast<std::vector<uint8_t>*>(context);
    auto* bytes  = reinterpret_cast<uint8_t*>(data);
    buffer->insert(buffer->end(), bytes, bytes + size);
}

// Save image to path that may contain Unicode characters
static bool saveImage(const fs::path& path, int width, int height, std::vector<uint8_t>& pixels)
{
    // Write PNG to memory buffer using callback
    std::vector<uint8_t> pngBuffer;
    int result = stbi_write_png_to_func(
        pngWriteCallback,
        &pngBuffer,
        width, height,
        3,
        pixels.data(),
        width * 3
    );

    if (!result || pngBuffer.empty()) return false;

    // Write buffer to file using fs::path (handles Unicode)
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    f.write(reinterpret_cast<const char*>(pngBuffer.data()), pngBuffer.size());
    return f.good();
}

bool embedVault(const std::string& carrierImagePath,
                const std::string& secretFilePath,
                const std::string& password,
                const std::string& outputImagePath)
{
    // Convert all paths to fs::path using u8path (handles Turkish/Unicode chars)
    fs::path carrierPath = fs::u8path(carrierImagePath);
    fs::path secretPath  = fs::u8path(secretFilePath);
    fs::path outputPath  = fs::u8path(outputImagePath);

    // Read secret file
    std::vector<uint8_t> secretBytes = readFile(secretPath);
    if (secretBytes.empty())
    {
        std::cout << "ERROR: Could not read secret file: " << secretFilePath << std::endl;
        return false;
    }

    // Load carrier image
    int width, height, channels;
    uint8_t* rawPixels = loadImage(carrierPath, &width, &height, &channels);
    if (!rawPixels)
    {
        std::cout << "ERROR: Could not load image: " << carrierImagePath << std::endl;
        return false;
    }

    std::vector<uint8_t> pixels(rawPixels, rawPixels + (static_cast<size_t>(width) * height * 3));
    stbi_image_free(rawPixels);

    // Get filename only (no full path)
    std::string filename    = secretPath.filename().u8string();
    uint16_t    filenameLen = static_cast<uint16_t>(filename.size());

    // Early capacity check before encrypting
    size_t estimatedPayload = 4 + 1 + 2 + filename.size() + secretBytes.size() + 64;
    if (estimatedPayload > maxEmbeddableBytes(pixels.size()))
    {
        std::cout << "ERROR: Image too small.\n"
                  << "  Capacity : " << maxEmbeddableBytes(pixels.size()) << " bytes\n"
                  << "  Needed   : " << estimatedPayload << " bytes" << std::endl;
        return false;
    }

    // Build plaintext blob: [filename_len(2)][filename][filedata]
    std::vector<uint8_t> plainBlob;
    plainBlob.push_back(static_cast<uint8_t>(filenameLen >> 8));
    plainBlob.push_back(static_cast<uint8_t>(filenameLen & 0xFF));
    plainBlob.insert(plainBlob.end(), filename.begin(), filename.end());
    plainBlob.insert(plainBlob.end(), secretBytes.begin(), secretBytes.end());

    // Encrypt the blob
    std::vector<uint8_t> encrypted = encryptData(plainBlob, password);
    if (encrypted.empty())
    {
        std::cout << "ERROR: Encryption failed." << std::endl;
        return false;
    }

    // Build final payload: [SISF][version][encrypted blob]
    std::vector<uint8_t> payload;
    payload.insert(payload.end(), MAGIC, MAGIC + 4);
    payload.push_back(VERSION);
    payload.insert(payload.end(), encrypted.begin(), encrypted.end());

    // Final capacity check with real payload size
    if (payload.size() * 8 + 32 > pixels.size())
    {
        std::cout << "ERROR: Payload too large.\n"
                  << "  Capacity : " << maxEmbeddableBytes(pixels.size()) << " bytes\n"
                  << "  Payload  : " << payload.size() << " bytes" << std::endl;
        return false;
    }

    // Embed payload into image pixels
    embedData(pixels, payload);

    // Create output directory if needed and save image
    if (outputPath.has_parent_path())
        fs::create_directories(outputPath.parent_path());

    if (!saveImage(outputPath, width, height, pixels))
    {
        std::cout << "ERROR: Could not write output image." << std::endl;
        return false;
    }

    std::cout << "Embedded: " << filename << " (" << secretBytes.size() << " bytes) -> "
              << outputImagePath << std::endl;
    return true;
}

VaultResult extractVault(const std::string& stegoImagePath,
                          const std::string& password)
{
    VaultResult result{false, "", {}, ""};

    fs::path stegoPath = fs::u8path(stegoImagePath);

    // Load stego image
    int width, height, channels;
    uint8_t* rawPixels = loadImage(stegoPath, &width, &height, &channels);
    if (!rawPixels)
    {
        result.error = "Could not load image: " + stegoImagePath;
        return result;
    }

    std::vector<uint8_t> pixels(rawPixels, rawPixels + (static_cast<size_t>(width) * height * 3));
    stbi_image_free(rawPixels);

    // Extract raw payload from pixel LSBs
    std::vector<uint8_t> payload = extractData(pixels);
    if (payload.size() < 6)
    {
        result.error = "No vault found in this image.";
        return result;
    }

    // Check magic bytes
    if (payload[0] != MAGIC[0] || payload[1] != MAGIC[1] ||
        payload[2] != MAGIC[2] || payload[3] != MAGIC[3])
    {
        result.error = "No SISFA vault found in this image.";
        return result;
    }

    // Parse header: [magic(4)][version(1)][encrypted blob]
    size_t  offset  = 4;
    uint8_t version = payload[offset++];

    if (version != VERSION)
    {
        result.error = "Unsupported vault version: " + std::to_string(version);
        return result;
    }

    // Decrypt the blob
    std::vector<uint8_t> encryptedBlob(payload.begin() + offset, payload.end());
    std::vector<uint8_t> decrypted = decryptData(encryptedBlob, password);

    if (decrypted.empty())
    {
        result.error = "Wrong password or corrupted vault.";
        return result;
    }

    // Parse decrypted blob: [filename_len(2)][filename][filedata]
    if (decrypted.size() < 2)
    {
        result.error = "Corrupted vault.";
        return result;
    }

    uint16_t filenameLen = (static_cast<uint16_t>(decrypted[0]) << 8) | decrypted[1];
    offset = 2;

    if (offset + filenameLen > decrypted.size())
    {
        result.error = "Corrupted filename in vault.";
        return result;
    }

    std::string filename(decrypted.begin() + offset, decrypted.begin() + offset + filenameLen);
    offset += filenameLen;

    result.success  = true;
    result.filename = filename;
    result.data     = std::vector<uint8_t>(decrypted.begin() + offset, decrypted.end());
    return result;
}