#ifndef EMTF_DATASET_H
#define EMTF_DATASET_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

const std::vector<std::string> HEADER = {"base", "ch", "sel", "offset", "hex",
                                         "length", "mask", "read", "write",
                                         "description", "station", "chid", "chamber",
                                         "name", "root"};

class Address
{
public:

    Address(std::vector<std::string> input);
    std::string get(std::string param) { return data[param];  }
    uint64_t get_as_hex(std::string param) 
    {
        std::string value = get(param);
        try {
            return stoll(value, 0, 16);
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] - Unable to cast param to hex - " << param << ": " << value << std::endl;
            return 0x0;
        }
    }
    void print(std::vector<int> spacers={});

private:
    std::map<std::string, std::string> data;
};

class Dataset
{
public:
    std::string fname;

    Dataset() {};
    Dataset(std::string fname);
    Dataset subset(std::string param, std::string pattern);

    unsigned int size() { return addresses.size(); }
    void print(int n_adrs=5);
    std::vector<Address> data() { return addresses; }

private:
    std::vector<Address> addresses;
};

#endif // EMTF_DATASET_H