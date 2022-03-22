#ifndef EMTF_DATASET_H
#define EMTF_DATASET_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

const std::vector<std::string> HEADER = {"base", "ch", "sel", "offset", "address hex",
                                         "length", "mask", "read", "write",
                                         "description", "station", "chid", "chamber",
                                         "suggested parameter name", "root name"};

class Address
{
public:

    Address(std::vector<std::string> input);
    std::string get(std::string param) { return data[param];  }
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

private:
    std::vector<Address> addresses;
};

#endif // EMTF_DATASET_H