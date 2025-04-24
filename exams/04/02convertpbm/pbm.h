#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
struct BinaryImage {
    int W = 0;
    int H = 0;
    std::vector<uint8_t> ImageData;

    bool ReadFromPBM(const std::string& filename);

    void read_comment(std::istream& is);

    void read_width(std::istream& is);

    void read_height(std::istream& is);

    void read_row(std::istream& is, int cols);
};

struct Image {
    int W = 0;
    int H = 0;
    std::vector<uint8_t> ImageData;
};

Image BinaryImageToImage(const BinaryImage& bimg);