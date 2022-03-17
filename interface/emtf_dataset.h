#ifndef EMTF_DATASET_H
#define EMTF_DATASET_H

#include <iostream>
#include <string>
#include <vector>

class Address
{
public:
    std::string base;
    std::string ch;
    std::string sel;
    std::string offset;
    std::string address;
    std::string hex;
    std::string length;
    std::string mask;
    std::string read;
    std::string write;
    std::string description;
    std::string station;
    std::string chid;
    std::string chamber;
    std::string suggested_parameter_name;
    std::string root_name;

    Address(std::vector<std::string> input);
private:
    std::vector<std::string> input;
};

class Dataset
{
public:
    std::string fname;

    Dataset() : Dataset("emtf_pcie_address_table.csv") {};
    Dataset(std::string fname);

private:
};

#endif // EMTF_DATASET_H