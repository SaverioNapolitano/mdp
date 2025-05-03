#pragma once
#include "mat.h"
#include <string>
#include <cstdint>
bool load_pcx(const std::string& filename, mat<uint8_t>& img);
