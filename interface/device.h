#ifndef DEVICE_H
#define DEVICE_H

#include <iostream>
#include "emtf_dataset.h"
#include "data.h"

class Device
{
public:
    std::string name;
    int id;
    std::string path;

    Device(std::string path);

    void print();
    ssize_t read(void *buf, size_t count, off_t offset);
    ssize_t write(const void *buf, size_t count, off_t offset);

    ssize_t read(void *buf, Address adr);
    ssize_t write(const void *buf, Address adr);

    ssize_t read(Data &data, Address adr) { return read(&data._data, adr); };
    ssize_t write(const Data &data, Address adr) { return write(&data._data, adr); };

    // friend std::ostream &operator<<(std::ostream &os, const Device &d);
};

std::ostream &operator<<(std::ostream &os, const Device &d);

#endif // DEVICE_H
