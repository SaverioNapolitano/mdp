#include "hufstr.h"
#include <fstream>
#include <iterator>
#include <algorithm>


class bitreader {
    std::istream& is_;
    size_t n_ = 0;
    uint8_t buffer_ = 0;

    uint32_t readbit() {
        if (n_ == 0) {
            is_.read(reinterpret_cast<char*>(&buffer_), sizeof(uint8_t));
            n_ = 8;
        }
        n_--;
        return (buffer_ >> n_) & 1;
    }
public:
    bitreader(std::istream& is) :is_(is) {}
    ~bitreader() {}
    std::istream& operator()(uint32_t& u, size_t n) {
        u = 0;
        while (n-- > 0) {
            u = (u << 1) | readbit();
        }
        return is_;
    }
};

hufstr::hufstr() {
    std::ifstream is("table.bin", std::ios::binary);
    uint32_t sym, len, code; // sym, len uint8_t
    bitreader br(is);
    while (br(sym, 8)) {
        br(len, 8);
        br(code, len);
        elem e{ static_cast<uint8_t>(sym), static_cast<uint8_t>(len), code };
        _table.push_back(e);
    }

}
std::vector<uint8_t> hufstr::compress(const std::string& s) const {
    std::vector<uint8_t> v{};
    uint8_t b = 0;
    uint8_t nbits = 0;
    for (const char& c : s) {
        uint8_t u = c;
        for (const elem& e : _table) {
            if (e._sym == u) {
                if (nbits > 0) {
                    if ((nbits + e._len) <= 8) {
                        b = (b << e._len) | e._code;
                        nbits += e._len;
                        if (nbits == 8) {
                            v.push_back(b);
                            b = 0;
                            nbits = 0;
                        }
                    }
                    else {
                        uint8_t diff = nbits + e._len - 8;
                        b = (b << (8 - nbits)) | (e._code >> diff);
                        v.push_back(b);
                        b = 0;
                        nbits = 0;
                        while (diff-- > 0) {
                            b = (b << 1) | ((e._code >> diff) & 1);
                            nbits++;
                            if (nbits == 8) {
                                v.push_back(b);
                                b = 0;
                                nbits = 0;
                            }
                        }
                    }
                }
                else {
                    if (e._len > 8) {
                        uint8_t diff = e._len - 8;
                        b = (e._code >> diff);
                        v.push_back(b);
                        b = 0;
                        while (diff-- > 0) {
                            b = (b << 1) | ((e._code >> diff) & 1);
                            nbits++;
                            if (nbits == 8) {
                                v.push_back(b);
                                b = 0;
                                nbits = 0;
                            }
                        }
                    }
                    else {
                        b = e._code;
                        nbits += e._len;
                    }

                }
                break;
            }
        }
    }
    if (nbits > 0) {
        while (nbits++ < 8) {
            b = (b << 1) | 1;
        }
        v.push_back(b);
    }
    return v;
}
std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
    uint32_t code = 0;
    uint8_t length = 0;
    std::string s("");
    for (const auto& b : v) {
        for (size_t i = 8; i-- > 0;) {
            code = (code << 1) | ((b >> i) & 1);
            length++;
            for (const auto& e : _table) {
                if (e._len == length and e._code == code) {
                    char c = e._sym;
                    s.append(1, c);
                    code = 0;
                    length = 0;
                    break;
                }
            }
        }
    }
    return s;
}