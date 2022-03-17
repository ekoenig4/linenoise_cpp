#include "emtf_dataset.h"

#include <fstream>

using namespace std;

template <typename T>
void print_vector(const vector<T> vec, string name = "array")
{
    int m = vec.size();
    cout << name << "(" << m << "): {" << endl;
    for (int j = 0; j < m; j++)
    {
        cout << vec[j] << ",";
    }
    cout << endl
         << "}" << endl;
}

void loadtxt(string fname, vector<Address> &out)
    {
        ifstream file(fname);
        if (!file.is_open())
            throw runtime_error("Could not open file: " + fname);

        string line, word;
        string delim = ",";
        getline(file, line);
        while (getline(file, line))
        {
            size_t pos = 0;
            string token;
            vector<string> vec;
            while ((pos = line.find(delim)) != string::npos)
            {
                token = line.substr(0, pos);
                vec.push_back(token);
                line.erase(0, pos + delim.length());
            }
            vec.push_back(line);
            Address adrs(vec);
            out.push_back(adrs);
        }
    }

Address::Address(vector<string> input)
{
    // if (input.size() != 16)
    //     cout << "Expected 16 values but got " << input.size() << endl;

    base = input[0];
    ch = input[1];
    sel = input[2];
    offset = input[3];
    address = input[4];
    hex = input[5];
    length = input[6];
    mask = input[7];
    read = input[8];
    write = input[9];
    description = input[10];
    station = input[11];
    chid = input[12];
    chamber = input[13];
    suggested_parameter_name = input[14];

    if (input.size() == 16)
        root_name = input[15];

    this->input = input;
}

Dataset::Dataset(string fname)
{
    this->fname = fname;

    vector<Address> out;
    loadtxt(fname, out);
}