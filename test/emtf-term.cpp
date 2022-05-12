#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include "linenoise.h"
#include <boost/regex.hpp>
#include <bitset>


#include "config.h"
#include "emtf_dataset.h"
#include "device.h"
#include "data.h"
#include "terminal_output.h"

using namespace boost;

string dataset_path = "/home/koenig/Documents/linenoise_cpp/data/emtf_pcie_address_table.csv";
Dataset *dataset;
vector<Device> devices;

//FILE* log_file;
// help strings
string top_help = 
"Available commands:\n open\n read\n write\n list\n dataset\n info\n print\n verbose\n debug\n exit\n"
"Type help [command] to see more infomation about a command\n"
"Type help instructions to learn how to use command line\n"
"A file of commands can be passed to the terminal on startup: ./emtf-term /path/to/cmds.txt\n"
"If a command file is passed, specify the dataset path with the dataset command first";

string help_todo = "[TODO] Write help message";
string no_help = "Help is not available for this item";
string instructions_help = "Start typing a command.\n"
"A hint for the next parameter is shown as you type (if available).\n"
"Press TAB at any time to cycle though possible completions.\n"
"Press Enter to execute the command";

string missing_args = "Command is missing agruments. Syntax:\n settings write [addr] [data]";
string missing_adr_name = "Command is missing an address name/regex.";
string missing_value = "Command is missing a value.";

string device_list_help = "Lists all opened devices";
void list_devices(string cmd);

string device_open_help = "Opens all devices that match regex pattern.\n"
						  "Overwrites any already openned devices.\n"
						  "Device regex does not need to include /dev/ path.";
void open_devices(string cmd);

string device_read_help = "Reads value of given address name/regex for all openned devices.";
void read_name(string cmd);

string device_write_help = "Writes value to given address name/regex for all openned devices.";
void write_name(string cmd);

string info_help = "Print information about given address name/regex.\n"
				   "Optionally an address parameter can be given followed by a regex pattern to match."
				   "By default this address parameter is name and does not need to be given.";
void address_info(string cmd);

string dataset_help = "Specify path to dataset csv.\n"
					  "If a dataset is not passed in a command file, the default is used.\n"
					  "Default is set in source code as: " +
					  dataset_path;
void set_dataset(string cmd);

string print_help = "Set the format for values to printed as. Either decimal (dec, default), hex (hex), or 64bit binary (bin).";
void set_print(string cmd);

string debug_help = "Set debug options";
void toggle_debug(string cmd);

string warning_help = "Toggle warnings on and off";
void toggle_warning(string cmd);

string verbose_help = "Set verbose level. 0 - no output, 1 - standard output";
void set_verbose(string cmd);

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

		{0, "read", "address name/regex", NULL, &device_read_help},
			{1, "(.*)", "<Enter>", read_name, &missing_adr_name},
		
		{0, "write", "address name/regex", NULL, &device_write_help},
			{1, "(.*)", "data (dec or 0x)", NULL, &missing_adr_name},
				{2, "([0-9a-fx]+)", "<Enter>", write_name, &missing_value},

		{0, "dataset", "/path/to/dataset.csv", NULL, &dataset_help},
			{1, "(.*)", "<Enter>", set_dataset, &dataset_help},

		{0, "info", "address name/regex", NULL, &info_help},
			{1, "(.*)", "<Enter>", address_info, &missing_adr_name},
				{2, "(.*)", "<Enter>", address_info, &missing_adr_name},

		{0, "debug", "0/1", NULL, &debug_help},
			{1, "([0-1])", "<Enter>", toggle_debug, &debug_help},

		{0, "warning", "0/1", NULL, &warning_help},
			{1, "([0-1])", "<Enter>", toggle_warning, &warning_help},

		{0, "verbose", "level (0,1)", NULL, &verbose_help},
			{1, "([0-1])", "<Enter>", set_verbose, &verbose_help},

		{0, "print", "(dec|hex|bin)", set_print, &print_help},

		{0, "exit", "<Enter>", terminate, NULL},
		{-1, "help", "command", NULL, &top_help} // help command = end marker
};

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

	print::output.verbose(1) << "openning devices: " << device_regex << endl;

	devices = get_valid_devices(device_regex);

	print::output.verbose(0) << "openned " << devices.size() << " devices" << endl;
}

void list_devices(string cmd)
{
	print::output.verbose(1) << "listing devices" << endl;

	for (Device &d : devices)
	{
		print::output.verbose(0) << d.print();
	}
}

void read_name(string cmd)
{
	vector<string> cmds = split(cmd);
	string address_regex = cmds[cmds.size() - 1];
	print::output.verbose(1) << "reading address: " << address_regex << " from " << devices.size() << " devices" << endl;
	Dataset addresses = dataset->subset("name", address_regex);

	if (addresses.size() == 0)
	{
		print::warning << "[WARNING] - Unable to find any addresses that match \"" << address_regex << "\"" << endl;
	}

	for (Address &adr : addresses.data())
	{
		print::output.verbose(0) << adr << endl;
		print::debug << "[DEBUG] | " << adr.print();

		for (Device &dev : devices)
		{
			Data buff(0x0);
			dev.read(&buff, adr);
			print::output.verbose(0) << "    " << dev << ": " << buff << endl;
		}
	}
}

void write_name(string cmd)
{
	vector<string> cmds = split(cmd);
	string address_regex = cmds[cmds.size() - 2];
	Data data(cmds[cmds.size() - 1]);

	print::output.verbose(1) << "writing data:       " << data << endl;
	print::output.verbose(1) << "to address pattern: " << address_regex << endl;

	Dataset addresses = dataset->subset("name", address_regex);

	if (addresses.size() == 0)
	{
		print::warning << "[WARNING] - Unable to find any addresses that match \"" << address_regex << "\"" << endl;
	}

	// cout << "[Device] -> [Address]: [Value] " << adr.get("name") << endl;
	for (Address adr : addresses.data())
	{
		print::debug << "[DEBUG] | " << adr.print();

		for (Device dev : devices)
		{
			dev.write(&data, adr);
		}
	}
}

void set_dataset(string cmd)
{
	vector<string> cmds = split(cmd);
	string path = cmds[cmds.size() - 1];
	print::output.verbose(1) << "Loading dataset: " << path << endl;
	dataset = new Dataset(path);
}

void address_info(string cmd)
{
	vector<string> cmds = split(cmd);

	string param, regex;
	if (cmds.size() == 2)
	{
		param = "name";
		regex = cmds[cmds.size() - 1];
	}

	if (cmds.size() == 3)
	{
		param = cmds[cmds.size() - 2];
		regex = cmds[cmds.size() - 1];
	}

	print::output.verbose(1) << "reading address " << param << " info for " << regex << endl;

	Dataset addresses = dataset->subset(param, regex);

	if (addresses.size() == 0)
	{
		print::warning << "[WARNING] - Unable to find any addresses that match \"" << regex << "\"" << endl;
	}

	print::output.verbose(0) << addresses.print(-1);
}

void toggle_debug(string cmd)
{
	vector<string> cmds = split(cmd);
	config::debug = stoi(cmds[cmds.size() - 1]);
	print::output.verbose(1) << "Debug set: " << (config::debug ? "TRUE" : "FALSE") << endl;
}

void toggle_warning(string cmd)
{
	vector<string> cmds = split(cmd);
	config::warning = stoi(cmds[cmds.size() - 1]);
	print::output.verbose(1) << "Warning set: " << (config::warning ? "TRUE" : "FALSE") << endl;
}

void set_verbose(string cmd)
{
	vector<string> cmds = split(cmd);
	int verbose = stoi(cmds[cmds.size() - 1]);
	config::verbose = verbose ? verbose > 0 : 0;
	print::output.verbose(1) << "Verbose set: " << config::verbose << endl;
}

void set_print(string cmd)
{
	vector<string> cmds = split(cmd);
	string print = cmds[cmds.size() - 1];
	if (print == "dec")
	{
		Data::format = F_DEC;
		print::output.verbose(1) << "printing values in decimal" << endl;
	}
	else if (print == "hex")
	{
		Data::format = F_HEX;
		print::output.verbose(1) << "printing values in hex." << endl;
	}
	else if (print == "bin")
	{
		Data::format = F_BIN;
		print::output.verbose(1) << "printing values in 64bit binary." << endl;
	}
}

void terminate(string cmd)
{
	print::output.verbose(0) << "exiting..." << endl;
	exit(0);
}

void runcmds(std::string fname, linenoise& ln)
{
	std::ifstream file(fname);
	if (!file.is_open())
		throw std::runtime_error("Could not open file: " + fname);

	std::string line;
	std::string comment = "#";
	while (std::getline(file, line))
	{
		if (line.find(comment) == 0)
			continue;

		string hint = ln.menu_tree.find_hints(line);
		int ei = ln.get_enter_index();
		
		// call callback for this command if available
		if (ei >= 0 && nr[ei].cb != NULL) nr[ei].cb(line);

		// display help message if requested
		if (ei < 0)	printf("%s", ln.get_help_message().c_str());
	}
}	


int main(int argc, char **argv) 
{
	char* buf;
	string line;

	//	log_file = fopen ("linenoise_log.txt", "w");

	// construct linenoise
	linenoise ln(nr, "history.txt");

	if (argc > 1) 
	{
		runcmds(argv[1], ln);
	}

	if (dataset == NULL)
	{
		dataset = new Dataset(dataset_path);
	}

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
