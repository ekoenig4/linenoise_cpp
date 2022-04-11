#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include "linenoise.h"
#include <boost/regex.hpp>
#include <bitset>


#include "emtf_dataset.h"
#include "device.h"

using namespace boost;

Dataset* dataset;
vector<Device> devices;

bool DEBUG = false;

enum PRINT_VALUE
{
	DEC,
	HEX,
	BIN
};

PRINT_VALUE PRINT_TYPE = DEC;

//FILE* log_file;
// help strings
string top_help = 
"Available commands:\n open\n read\n write\n list\n info\n print\n debug\n exit\n"
"Type help [command] to see more infomation about a command\n"
"Type help instructions to learn how to use command line";

string help_todo = "[TODO] Write help message";
string no_help = "Help is not available for this item";
string instructions_help = "Start typing a command.\n"
"A hint for the next parameter is shown as you type (if available).\n"
"Press TAB at any time to cycle though possible completions.\n"
"Press Enter to execute the command";

string missing_args = "Command is missing agruments. Syntax:\n settings write [addr] [data]";
string missing_adr_name = "Command is missing an address name.";
string missing_value = "Command is missing a value.";

string device_list_help = "Lists all opened devices";
void list_devices(string cmd);

string device_open_help = "Opens all devices that match regex pattern.\n"
						  "Overwrites any already openned devices.\n"
						  "Device regex does not need to include /dev/ path.";
void open_devices(string cmd);

string device_read_help = "Reads value of given address name for all openned devices.";
void read_name(string cmd);

string device_write_help = "Writes value to given address name for all openned devices.";
void write_name(string cmd);

string info_help = "Print information about given address name.";
void address_info(string cmd);

string print_help = "Set the format for values to printed as. Either decimal (dec, default), hex (hex), or 64bit binary (bin).";
void set_print(string cmd);

string debug_help = "Set debug options";
void toggle_debug(string cmd);

void terminate(string cmd);

// menu structure record
// record structure:
// field 1: number of argument in command line that the user is typing
// field 2: string used for matching to user-typed input. Supports Regex
// field 3: hint for the NEXT argument (not the one currently being typed)
// field 4: pointer to callback function, or NULL if no callback
// field 5: pointer to string containing help message for this command. Last record contains pointer to top-level help
node_record nr[] =
	{
		/*
		device command (open/etc.) (device name/regex)
		device list - list all open devices

		register write register (name/regex) value (decimal/hex) (str_to_ll)
		register read register (name/regex) (print as 0X-hex)

		verylog format 16'habc5 (number of bits)'(hex)(value)


		read entire 64 bit register, edit masked part of register and write entire register back
		*/
		// lvl  command_text                hint for next cmd      callback	ptr to help string
		{0, "list", "<Enter>", list_devices, &device_list_help},
		{0, "open", "path/regex", NULL, &device_open_help},
			{1, "(.*)", "<Enter>", open_devices, &missing_adr_name},

		{0, "read", "address name", NULL, &device_read_help},
			{1, "(.*)", "<Enter>", read_name, &missing_adr_name},
		
		{0, "write", "data (dec or 0x)", NULL, &device_write_help},
			{1, "([0-9a-fx]+)", "to address name", NULL, &missing_value},
				{2, "(.*)", "<Enter>", write_name, &missing_adr_name},

		{0, "info", "address name", NULL, &info_help},
			{1, "(.*)", "<Enter>", address_info, &missing_adr_name},

		{0, "debug", "0/1", NULL, &debug_help},
			{1, "([0-1])", "<Enter>", toggle_debug, &debug_help},

		{0, "print", "(dec|hex|bin)", set_print, &print_help},

		{0, "exit", "<Enter>", terminate, NULL},
		{-1, "help", "command", NULL, &top_help} // help command = end marker
};



string print_value(uint64_t value)
{
	stringstream ss;
	switch (PRINT_TYPE)
	{
	case DEC:
		ss << value;
		break;
	case HEX:
		ss << "0x" << std::hex << value;
		break;
	case BIN:
		ss << "0b" << std::bitset<64>(value);
		break;
	// default:
	}
	return ss.str();
}

vector<string> split(string s, string delimiter=" ")
{
	vector<string> words;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		words.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	words.push_back(s);
	return words;
}

vector<Device> get_valid_devices(string pattern)
{
	vector<Device> devices;
	
    regex expression(pattern);
    cmatch what;

	string path = "/dev/";
	DIR *dirp = opendir(path.c_str());
	struct dirent *dp;
	while ( (dp = readdir(dirp)) != NULL )
	{
		string device_name = dp->d_name;

		if ( !regex_match(device_name.c_str(), what, expression) )
			continue;

		string device_path = "/dev/" + device_name;
		Device device(device_path);

		if ( device.id != -1 )
		{
			devices.push_back(device);
		}
	}
	return devices;
}

void open_devices(string cmd)
{
	devices.clear();
	vector<string> cmds = split(cmd);
	string path_regex = cmds[cmds.size() - 1];

	vector<string> paths = split(path_regex, "/");
	string device_regex = paths[paths.size() - 1];

	cout << "openning devices: " << device_regex << endl;

	devices = get_valid_devices(device_regex);

	cout << "openned " << devices.size() << " devices" << endl;
}

void list_devices(string cmd)
{
	cout << "listing devices" << endl;
	for (Device &d : devices)
	{
		d.print();
	}
}

void read_name(string cmd)
{
	vector<string> cmds = split(cmd);
	string address_regex = cmds[cmds.size() - 1];
	cout << "reading address: " << address_regex << " from " << devices.size() << " devices" << endl;
	Dataset addresses = dataset->subset("name", address_regex);

	if (addresses.size() == 0)
	{
		cout << "[WARNING] - Unable to find any addresses that match \"" << address_regex << "\"" << endl;
	}

	for (Address &adr : addresses.data())
	{
		if (DEBUG)
		{
			cout << "[DEBUG] | ";
			adr.print();
		}
		for (Device &dev : devices)
		{
			uint64_t buff = 0x0;
			dev.read(&buff, adr);
			cout << dev << ": " << print_value(buff) << endl;
		}
	}
}

uint64_t convert_to_data(string cmd)
{
	if (cmd.find("0x") == 0)
		return stoll(cmd, 0, 16);
	return stoll(cmd);
}

void write_name(string cmd)
{
	vector<string> cmds = split(cmd);
	uint64_t data = convert_to_data(cmds[cmds.size() - 2]);
	string address_regex = cmds[cmds.size() - 1];
	cout << "writing data: " << data << endl;
	cout << "to address:   " << address_regex << endl;
	Dataset addresses = dataset->subset("name", address_regex);

	if (addresses.size() == 0)
	{
		cout << "[WARNING] - Unable to find any addresses that match \"" << address_regex << "\"" << endl;
	}

	for (Address adr : addresses.data())
	{
		if (DEBUG)
		{
			cout << "[DEBUG] | ";
			adr.print();
		}
		for (Device dev : devices)
		{
			dev.write(&data, adr);
		}
	}
}

void address_info(string cmd)
{
	vector<string> cmds = split(cmd);
	string address_regex = cmds[cmds.size() - 1];
	cout << "reading address info for " << address_regex << endl;
	Dataset addresses = dataset->subset("name", address_regex);

	if (addresses.size() == 0)
	{
		cout << "[WARNING] - Unable to find any addresses that match \"" << address_regex << "\"" << endl;
	}

	addresses.print();
}

void toggle_debug(string cmd)
{
	vector<string> cmds = split(cmd);
	DEBUG = stoi(cmds[cmds.size() - 1]);
	for (Device &d : devices)
		d.debug = DEBUG;

	cout << "Debug set: " << (DEBUG ? "TRUE" : "FALSE") << endl;
}

void set_print(string cmd)
{
	vector<string> cmds = split(cmd);
	string print = cmds[cmds.size() - 1];
	if (print == "dec")
	{
		PRINT_TYPE = DEC;
		cout << "printing values in decimal" << endl;
	}
	else if (print == "hex")
	{
		PRINT_TYPE = HEX;
		cout << "printing values in hex." << endl;
	}
	else if (print == "bin")
	{
		PRINT_TYPE = BIN;
		cout << "printing values in 64bit binary." << endl;
	}
}

void terminate(string cmd)
{
	printf("Exiting...\n");
	exit(0);
}


int main(int argc, char **argv) 
{
	dataset = new Dataset("emtf_pcie_address_table.csv");

	char* buf;
	string line;

	//	log_file = fopen ("linenoise_log.txt", "w");

	// construct linenoise
	linenoise ln(nr, "history.txt");

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to ln.prompt() will block as long as the user types something
     * and presses enter. */
    
    while((buf = ln.prompt("emtf-term> ")) != NULL) 
	{
		line = (string) buf;
		// index of the record when user pressed Enter
		int ei = ln.get_enter_index();

//		printf("cmd: '%s' enter index: %d\n", line.c_str(), ei);

		// call callback for this command if available
		if (ei >= 0 && nr[ei].cb != NULL) nr[ei].cb(line);

		// display help message if requested
		if (ei < 0)	printf("%s", ln.get_help_message().c_str());

		free (buf);
	}
    return 0;
}
