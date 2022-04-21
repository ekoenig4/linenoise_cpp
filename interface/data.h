#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>

enum Format
{
    F_DEC,
    F_HEX,
    F_BIN
};

struct Data
{
    static Format format;

    uint64_t _data;
    Data() { _data = 0x0;  };
    Data(uint64_t data) { _data = data;  };
    Data(std::string data);
    void operator=(const uint64_t &data) { _data = data; }
    std::string print() const;
    friend std::ostream& operator<<(std::ostream& os, const Data& data);
};

#endif // DATA_H