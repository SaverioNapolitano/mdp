#pragma once
#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <memory> 
class hufstr {
    struct elem {
        uint8_t _sym;
        uint8_t _len = 0;
        uint32_t _code = 0;
        elem(uint8_t sym, uint8_t len, uint32_t code) :_sym(sym), _len(len), _code(code) {}
        elem(const uint8_t& sym) : _sym(sym) {}
        bool operator<(const elem& rhs) const {
            if (_len < rhs._len)
                return true;
            else if (_len > rhs._len)
                return false;
            else
                return _sym < rhs._sym;
        }
    };
    std::vector<elem> _table;
public:
    hufstr();
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
};