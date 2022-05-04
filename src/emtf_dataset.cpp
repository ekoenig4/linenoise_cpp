#include "emtf_dataset.h"

#include <fstream>
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

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
    {
        cerr << "[ERROR] - Could not open file: " << fname << endl;
        return;
    }

    string line, word;
    string comma_delim = ",";
    string quote_delim = "\"";
    getline(file, line);
    while (getline(file, line))
    {
        size_t pos = 0;
        string token;
        vector<string> vec;
        while ((pos = line.find(comma_delim)) != string::npos)
        {
            token = line.substr(0, pos);

            if ( token.find(quote_delim) == 0 )
            { // Handle entries that have " " 
                pos = line.find(quote_delim, 1)+1;
                token = line.substr(0, pos);
            }

            vec.push_back(token);
            line.erase(0, pos + comma_delim.length());
        }
        vec.push_back(line);
        Address adrs(vec);
        out.push_back(adrs);
    }
}

Address::Address(vector<string> input)
{
    for ( unsigned int i = 0 ; i < HEADER.size(); i++)
    {
        if (input.size() > i)
            data[HEADER[i]] = input[i];
        else
            data[HEADER[i]] = "null";
    }
}

std::ostream &operator<<(std::ostream &os, const Address &adr)
{
    os << adr.get("name");
    return os;
}

string Address::print(vector<int> spacers)
{
    stringstream ss;
    for (unsigned int i = 0; i < HEADER.size(); i++)
    {
        string value = get(HEADER[i]);
        int n_spaces = value.size() + 1;

        if (spacers.size() > i)
        {
            if (spacers[i] > n_spaces)
            {
                n_spaces = spacers[i];
            }
        }

        string spaces(n_spaces - value.size(), ' ');
        ss << value << spaces << " | ";
        // cout << (string)value << endl;
    }
    ss << endl;
    return ss.str();
}

Dataset::Dataset(string fname)
{
    this->fname = fname;

    addresses.clear();
    loadtxt(fname, addresses);

}

string Dataset::print(int n_adrs)
{
    stringstream ss;
    ss << "N Addresses: " << size() << endl;
    vector<int> spacers;
    if ( n_adrs > size() || n_adrs == -1)
        n_adrs = size();

    int n_char = 0;
    for (string param : HEADER)
    {
        int space = param.size();
        for (unsigned int i = 0; i < n_adrs; i++)
        {
            string value = addresses[i].get(param);
            int size = value.size();
            if (size > space)
            {
                space = size;
            }
        }
        n_char += space + 1 + 3;
        spacers.push_back(space+1);
    }

    for (unsigned int i = 0; i < HEADER.size(); i++)
    {
        string spaces(spacers[i] - HEADER[i].size(), ' ');
        ss  << HEADER[i] << spaces << " | ";
    }
    ss << endl;

    string line(n_char, '-');
    ss << line << endl;

    for (unsigned int i = 0; i < n_adrs; i++)
    {
        ss << addresses[i].print(spacers);
    }
    return ss.str();
}

string matching_param(string param)
{
    regex param_regex(param);
    cmatch what;
    for (string param : HEADER)
    {
        if (regex_match(param.c_str(), what, param_regex))
            return param;
    }
    return "null";
}

Dataset Dataset::subset(string param, string pattern)
{
    Dataset _subset;

    param = matching_param(".*"+param+".*");
    regex expression(pattern);
    cmatch what;

    for (Address adr : addresses)
    {
        string value = adr.get(param);

        if (regex_match(value.c_str(), what, expression))
        {
            _subset.addresses.push_back(adr);
        }
    }

    return _subset;
}