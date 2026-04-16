#include "steganography.h"
#include <iostream>

void embedData(std::vector<uint8_t>& pixels, const std::vector<uint8_t>& secretData)
{
    // Each byte of hidden data needs 8 pixels (bits) to hide in.
    // Image file size should be at least 8 * pixels.size()

    // 32 pixel bytes for the size header + 8 pixel bytes per data byte
    if(secretData.size() * 8 + 32 > pixels.size())
    {
        std::cout << "ERROR: Image is too small to hide this data! "
                  << "Max: " << (pixels.size() - 32) / 8 << " bytes, "
                  << "Needed: " << secretData.size() << " bytes." << std::endl;
        return;
    }

    // Need to store length of the secret data first.
    // So, extractData function knows how many bytes to read later.
    // We use 4 bytes (32 bits) to store the length.

    uint32_t dataSize = secretData.size();
    int pixelByteIndex{0};

    // First embed the 4 bytes of dataSize into pixels
    // Loop through all 32 bits of dataSize (4 bytes * 8 bits)
    for(int bit = 31; bit >= 0; bit--)
    {
        // Extract one bit from dataSize
        uint8_t secretBit = (dataSize >> bit) & 1;

        // Clear the LSB of current pixel byte
        pixels[pixelByteIndex] &= 0xFE;

        // Set the LSB to our secret bit
        pixels[pixelByteIndex] |= secretBit;

        pixelByteIndex++;
    }

    // Embed the actual data bytes
    for(uint8_t byte: secretData)
    {
        // Loop through all 8 bits of this byte
        for(int bit = 7; bit >= 0; bit--)
        {
            uint8_t secretBit = (byte >> bit) & 1;
            
            pixels[pixelByteIndex] &= 0xFE;
            pixels[pixelByteIndex] |= secretBit;

            pixelByteIndex++;
        }
    }
}

std::vector<uint8_t> extractData(const std::vector<uint8_t>& pixels)
{
    int pixelByteIndex{0};
    uint32_t dataSize{0};

    // Read the first 32 bits to get dataSize
    for(int bit = 31; bit >=0; bit--)
    {
        uint8_t lsb = pixels[pixelByteIndex] & 1;
        dataSize |= (lsb << bit);
        pixelByteIndex++;
    }

    std::cout << "Extracting " << dataSize << " bytes..." << std::endl;
    std::vector<uint8_t> secretData;

    for(uint32_t i=0; i < dataSize; i++)
    {
        uint32_t byte{0};

        for(int bit=7; bit >= 0; bit--)
        {
            uint8_t lsb = pixels[pixelByteIndex] & 1;
            byte |= (lsb << bit);
            pixelByteIndex++;
        }
         secretData.push_back(byte);
    }

    return secretData;
}