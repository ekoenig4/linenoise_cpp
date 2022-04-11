#include "device.h"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <bitset>
#include <cstring>
using namespace std;

std::ostream &operator<<(std::ostream &os, const Device &d)
{
    os << d.path;
    return os;
}

Device::Device(string path)
{
    this->path = path;
    id = ::open(path.c_str(), O_RDWR);
    name = path + ":" + to_string(id);

    if (id == -1)
    {
        cout << "[WARNING] - Unable to open device: " << path << endl;
    }
}

ssize_t Device::read(void *buf, size_t count, off_t offset)
{
    if (debug)
    {
        cout << "[DEBUG] - reading " << count << " bytes from " << hex << offset << endl;
    }

    int ret = pread(id, buf, count, offset);

    if (ret == -1)
    {
        cerr << "[ERROR] - Unable to read from device: " << path << endl;
    }

    return ret;
}

ssize_t Device::read(void *buf, Address adr)
{
    // Get address, word and position of least significant bit
    off_t offset = adr.get_as_hex("hex");
    uint64_t mask = adr.get_as_hex("mask");
    uint64_t pos = ffs(mask) - 1;

    if (debug)
    {
        cout << "[DEBUG] - reading" << endl;
        cout << "          "
             << "address: " << hex << offset << endl;
        cout << "          "
             << "mask:    " << bitset<64>(mask) << endl;
    }

    // Get entire 64bit block and mask the word out 
    int ret = read(buf, 8, offset);
    uint64_t block = *static_cast<uint64_t *>(buf);
    uint64_t data = (block & mask) >> pos;

    // Write offset word back to buffer
    memcpy(buf, &data, sizeof(data));

    return ret;
}

ssize_t Device::write(const void *buf, size_t count, off_t offset)
{
    if (debug)
    {
        cout << "[DEBUG] - writing " << count << " bytes to " << hex << offset << endl;
    }

    int ret = pwrite(id, buf, count, offset);

    if (ret == -1)
    {
        cerr << "[ERROR] - Unable to write to device: " << path << endl;
    }

    return ret;
}

ssize_t Device::write(const void *buf, Address adr)
{
    // Get address, word and position of least significant bit
    off_t offset = adr.get_as_hex("hex");
    uint64_t mask = adr.get_as_hex("mask");
    uint64_t pos = ffs(mask) - 1;

    // Get entire 64bit block
    uint64_t block = 0x0;
    read(&block, 8, offset);

    // Get data to write, and move it to the word position
    uint64_t data;
    memcpy(&data, buf, sizeof(buf));
    data = data << pos;


    if (debug)
    {
        cout << "[DEBUG] - writing" << endl;
        cout << "          "
             << "address: " << hex << offset << endl;
        cout << "          "
             << "mask:    " << bitset<64>(mask) << endl;
        cout << "          "
             << "block:   " << bitset<64>(block) << endl;
        cout << "          "
             << "data:    " << bitset<64>(data) << endl;
    }

    // Zero out the word position
    block = block & ~mask;

    // Write word into block
    block = block | data;
    write(&block, 8, offset);
}

void Device::print()
{
    cout << "Device: " << name << endl;
}