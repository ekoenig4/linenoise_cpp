#include "data.h"

#include <bitset>
#include <sstream>

using namespace std;

Format Data::format = F_DEC;

Data::Data(string data)
{
	if (data.find("0x") == 0)
		_data = stoll(data, 0, 16);
	else
        _data = stoll(data);
}

string Data::print() const
{
    stringstream ss;
	switch (format)
	{
	case F_DEC:
		ss << _data;
		break;
	case F_HEX:
		ss << "0x" << std::hex << _data;
		break;
	case F_BIN:
		ss << "0b" << std::bitset<64>(_data);
		break;
	// default:
	}
	return ss.str();
}

ostream& operator<<(ostream& os, const Data& data)
{
    os << data.print();
    return os;
}