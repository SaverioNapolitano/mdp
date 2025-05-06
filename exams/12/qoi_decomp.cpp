#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cassert>
#include <bit>

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows*cols) {}

    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

enum chunk {
    QOI_OP_RGB = 0,
    QOI_OP_RGBA = 1,
    QOI_OP_INDEX = 2,
    QOI_OP_DIFF = 3,
    QOI_OP_LUMA = 4,
    QOI_OP_RUN = 5,
};
chunk chunk_type(uint8_t tag) {
    if (tag == 254) {
        return QOI_OP_RGB;
    }
    if (tag == 255) {
        return QOI_OP_RGBA;
    }
    if (((tag >> 6) & 3) == 0) {
        return QOI_OP_INDEX;
    }
    if (((tag >> 6) & 3) == 1) {
        return QOI_OP_DIFF;
    }
    if (((tag >> 6) & 3) == 2) {
        return QOI_OP_LUMA;
    }
    return QOI_OP_RUN;

}

void chunk_decode(std::istream& is, chunk c, std::array<std::array<uint8_t, 4>, 64>& seen_pixels, mat<std::array<uint8_t, 4>>& img, uint8_t& last_pos, size_t& i, uint8_t tag) {
    if (c == QOI_OP_RGB) {
        uint8_t red = is.get();
        uint8_t green = is.get();
        uint8_t blue = is.get();
        uint8_t alpha = seen_pixels[last_pos][3];
        uint8_t pos = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64;
        seen_pixels[pos] = { red, green, blue, alpha };
        last_pos = pos;
        img[i++] = seen_pixels[last_pos];
    }
    if (c == QOI_OP_RGBA) {
        uint8_t red = is.get();
        uint8_t green = is.get();
        uint8_t blue = is.get();
        uint8_t alpha = is.get();
        uint8_t pos = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64;
        seen_pixels[pos] = { red, green, blue, alpha };
        last_pos = pos;
        img[i++] = seen_pixels[last_pos];
    }
    if (c == QOI_OP_INDEX) {
        uint8_t ix = tag & 63;
        img[i++] = seen_pixels[ix];
        last_pos = ix;
    }
    if (c == QOI_OP_DIFF) {
        uint8_t dr = (tag >> 4) & 3;
        uint8_t dg = (tag >> 2) & 3;
        uint8_t db = tag & 3;
        uint8_t red = seen_pixels[last_pos][0] + (dr - 2);
        uint8_t green = seen_pixels[last_pos][1] + (dg - 2);
        uint8_t blue = seen_pixels[last_pos][2] + (db - 2);
        uint8_t alpha = seen_pixels[last_pos][3];
        uint8_t pos = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64;
        seen_pixels[pos] = { red, green, blue, alpha };
        last_pos = pos;
        img[i++] = seen_pixels[last_pos];
    }
    if (c == QOI_OP_LUMA) {
        uint8_t dg = tag & 63;
        uint8_t byte = is.get();
        uint8_t dr_dg = (byte >> 4) & 15;
        uint8_t db_dg = byte & 15;
        uint8_t dr = dr_dg + dg - 32;
        uint8_t db = db_dg + dg - 32;
        uint8_t green = seen_pixels[last_pos][1] + (dg - 32);
        uint8_t red = seen_pixels[last_pos][0] + (dr - 8);
        uint8_t blue = seen_pixels[last_pos][2] + (db - 8);
        uint8_t alpha = seen_pixels[last_pos][3];
        uint8_t pos = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64;
        seen_pixels[pos] = { red, green, blue, alpha };
        last_pos = pos;
        img[i++] = seen_pixels[last_pos];
    }
    if (c == QOI_OP_RUN) {
        uint8_t run = (tag & 63) + 1;
        uint8_t ix = last_pos;
        for (uint8_t j = 0; j < run; j++) {
            img[i++] = seen_pixels[ix];
        }
    }
}
int main(int argc, char *argv[])
{
    using namespace std::string_literals;
    // TODO: Manage the command line  
    if (argc != 3) {
        return EXIT_FAILURE;
    }
    std::ifstream is(argv[1], std::ios::binary);
    if (!is) {
        return EXIT_FAILURE;
    }
    // TODO: Lettura dell'header e delle dimensioni dell'immagine 
    std::string magic{};
    is.read(magic.data(), 4);
    if (magic.data() != "qoif"s) {
        return EXIT_FAILURE;
    }
    uint32_t width, height;
    is.read(reinterpret_cast<char*>(&width), 4);
    is.read(reinterpret_cast<char*>(&height), 4);
    width = std::byteswap(width);
    height = std::byteswap(height);
    is.get(); // channels 
    is.get(); // colorspace
    using rgba = std::array<uint8_t, 4>; // Potete modificare questa definizione!
    mat<rgba> img(height, width); // TODO: Dovete mettere le dimensioni corrette!
    // TODO: decodificare il file QOI in input e inserire i dati nell'immagine di output
    std::array<rgba, 64> seen_pixels{};
    uint8_t red = 0, blue = 0, green = 0, alpha = 255;
    uint8_t pos = (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64;
    seen_pixels[pos] = { 0, 0, 0, 255 };
    uint8_t last_pos = pos;
    size_t covered_pixels = 0;
    while (covered_pixels < width * height) {
        uint8_t tag = is.get();
        chunk c = chunk_type(tag);
        chunk_decode(is, c, seen_pixels, img, last_pos, covered_pixels, tag);
    }
    // Questo è il formato di output PAM. È già pronto così, ma potete modificarlo se volete
    std::ofstream os(argv[2], std::ios::binary); // Questo utilizza il secondo parametro della linea di comando!
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}