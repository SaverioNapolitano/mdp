#pragma once
#include "mat.h"
#include "types.h"
#include <string>
bool load_pcx(const std::string& filename, mat<vec3b>& img);