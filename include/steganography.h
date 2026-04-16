#pragma once

#include <vector>
#include <cstdint>

// embedData takes pixels and secret bytes. & pointer for editing the actual data not a copy.
void embedData(std::vector<uint8_t>& pixels, const std::vector<uint8_t>& secretData);

// extractData takes pixels, pulls out the hidden bytes and returns them.
std::vector<uint8_t> extractData(const std::vector<uint8_t>& pixels);

// Returns how many bytes can be hidden inside an image with the given pixel count.
inline size_t maxEmbeddableBytes(size_t pixelCount)
{
    if (pixelCount <= 32) return 0;
    return (pixelCount - 32) / 8;
}