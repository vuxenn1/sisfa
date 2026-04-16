#pragma once
#include <string>
#include <vector>
#include <cstdint>

std::vector<uint8_t> encryptData(const std::vector<uint8_t>& plainData, const std::string& password);
std::vector<uint8_t> decryptData(const std::vector<uint8_t>& encryptedData, const std::string& password);