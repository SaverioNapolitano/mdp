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

    std::vector<uint8_t> sym3{ 32, 101 };
    uint32_t code3_start = 0;

    for (const auto& s : sym3) {
        elem e{ s, 3, code3_start };
        code3_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym4{ 97, 105, 108, 110, 111, 114 };
    uint32_t code4_start = 4;

    for (const auto& s : sym4) {
        elem e{ s, 4, code4_start };
        code4_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym5{ 99, 100, 115, 116, 117 };
    uint32_t code5_start = 20;

    for (const auto& s : sym5) {
        elem e{ s, 5, code5_start };
        code5_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym6{ 39, 44, 103, 109, 112, 118 };
    uint32_t code6_start = 50;

    for (const auto& s : sym6) {
        elem e{ s, 6, code6_start };
        code6_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym7{ 9, 10, 13, 46, 91, 93, 98, 102, 104 };
    uint32_t code7_start = 112;

    for (const auto& s : sym7) {
        elem e{ s, 7, code7_start };
        code7_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym8{ 34, 49, 83, 113, 122 };
    uint32_t code8_start = 242;

    for (const auto& s : sym8) {
        elem e{ s, 8, code8_start };
        code8_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym9{ 50, 51, 58, 59, 65, 68, 69, 71, 73 };
    uint32_t code9_start = 494;

    for (const auto& s : sym9) {
        elem e{ s, 9, code9_start };
        code9_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym10{ 48, 52, 53, 54, 55, 56, 57, 63, 67, 76, 77, 78, 80 };
    uint32_t code10_start = 1006;

    for (const auto& s : sym10) {
        elem e{ s, 10, code10_start };
        code10_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym11{ 33, 66, 70, 79, 81, 82, 84 };
    uint32_t code11_start = 2038;

    for (const auto& s : sym11) {
        elem e{ s, 11, code11_start };
        code11_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym12{ 45, 86, 107 };
    uint32_t code12_start = 4090;

    for (const auto& s : sym12) {
        elem e{ s, 12, code12_start };
        code12_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym13{ 40, 41, 72, 85, 90 };
    uint32_t code13_start = 8186;

    for (const auto& s : sym13) {
        elem e{ s, 13, code13_start };
        code13_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym14{ 75 };
    uint32_t code14_start = 16382;

    for (const auto& s : sym14) {
        elem e{ s, 14, code14_start };
        code14_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym17{ 47 };
    uint32_t code17_start = 131064;

    for (const auto& s : sym17) {
        elem e{ s, 17, code17_start };
        code17_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym18{ 119 };
    uint32_t code18_start = 262130;

    for (const auto& s : sym18) {
        elem e{ s, 18, code18_start };
        code18_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym19{ 64, 74 };
    uint32_t code19_start = 524262;

    for (const auto& s : sym19) {
        elem e{ s, 19, code19_start };
        code19_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym20{ 88, 120 };
    uint32_t code20_start = 1048528;

    for (const auto& s : sym20) {
        elem e{ s, 20, code20_start };
        code20_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym21{ 87, 94, 106, 121, 125 };
    uint32_t code21_start = 2097060;

    for (const auto& s : sym21) {
        elem e{ s, 21, code21_start };
        code21_start++;
        _table.push_back(e);
    }

    std::vector<uint8_t> sym22{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 35, 36, 37, 38, 42, 43, 60, 61, 62, 89, 92, 95, 96, 123, 124 };
    for (uint32_t i = 126; i < 256; i++) {
        sym22.push_back(i);
    }
    uint32_t code22_start = 4194130;

    for (const auto& s : sym22) {
        elem e{ s, 22, code22_start };
        code22_start++;
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
    std::string s;
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
