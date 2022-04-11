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

//FILE* log_file;

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
	cout << "reading address: " << address_regex << endl;
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
			cout << dev << " = " << buff << endl;
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

void list_dataset(string cmd)
{
	printf("Printing Dataset ");
	dataset->print();
}

void toggle_debug(string cmd)
{
	DEBUG = !DEBUG;

	for (Device &d : devices)
		d.debug = DEBUG;

	cout << "Debug set: " << (DEBUG ? "TRUE" : "FALSE") << endl;
}

void __init__debug__()
{
	for (Device &d : devices)
		d.debug = DEBUG;
}

void terminate(string cmd)
{
	printf("Exiting...\n");
	exit(0);
}

// help strings
string top_help = 
"Available commands:\n instructions\n settings\n reset\n prbs\n exit\n"
"Type help [command] to see more infomation about a command\n"
"Type help instructions to learn how to use command line";
string help_todo = "[TODO] Write help message";
string no_help = "Help is not available for this item";
string settings_help = "Writes settings into specified address";
string reset_help = "Resets the device";
string prbs_help = "Sets all TX and RX units to transmit/receive specified PRBS pattern (7,15,23,31)";
string instructions_help = "Start typing a command.\n"
"A hint for the next parameter is shown as you type (if available).\n"
"Press TAB at any time to cycle though possible completions.\n"
"Press Enter to execute the command";
string missing_args = "Command is missing agruments. Syntax:\n settings write [addr] [data]";
string prbs_read_help = "Reads and displays PRBS counters from all RX units";

string open_help = "Opens all devices that match regex pattern.\n"
				   "Overwrites any already openned devices.\n"
				   "Device regex does not need to include /dev/ path.";

string debug_help = "Toggle debug options";

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
		{0, "device", "open|read|write|list", NULL, &help_todo},
			{1, "list", "<Enter>", list_devices, &help_todo},
			{1, "open", "path/regex", NULL, &open_help},
				{2, "([0-9a-fx]+)", "<Enter>", open_devices, &no_help},

			{1, "read", "address name", NULL, &help_todo},
				{2, "([0-9a-fx]+)", "<Enter>", read_name, &help_todo},
			
			{1, "write", "data (dec or 0x)", NULL, &help_todo},
				{2, "([0-9a-fx]+)", "to address name", NULL, &help_todo},
					{3, "([0-9a-fx]+)", "<Enter>", write_name, &help_todo},

		{0, "dataset", "operation", NULL, &help_todo},
			{1, "list", "<Enter>", list_dataset, &help_todo},

		// {0, "settings",                 "operation",           NULL,    &settings_help},
		// {1, 	"write",                "address (dec or 0x)", NULL,    &missing_args},
		// {2, 		"([0-9a-fx]+)",     "data (dec or 0x)",    NULL,    &missing_args},
		// {3, 			"([0-9a-fx]+)", "<Enter>",             cb1,     &settings_help},
		// {1, 	"wrong",                "stuff",               NULL,    &no_help},
		// {2, 		"stuff",            "<Enter>",             NULL,    &no_help},
		// {0, "reset", "<Enter>", NULL, &reset_help},
		// {0, "instructions", "<Enter>", NULL, &instructions_help},
		{0, "debug", "<Enter>", toggle_debug, &debug_help},
		{0, "exit", "<Enter>", terminate, NULL},
		{-1, "help", "command", NULL, &top_help} // help command = end marker
};


int main(int argc, char **argv) 
{
	dataset = new Dataset("emtf_pcie_address_table.csv");
	__init__debug__();

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
