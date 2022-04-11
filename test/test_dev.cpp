#include "device.h"
#include "emtf_dataset.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include <bitset>

using namespace std;

template <typename T, size_t N>
void print_buffer(T (&buf)[N])
{
    size_t nbytes = sizeof(buf[0]);

    for (int i = 0; i < N; i++)
    {
        cout << hex << setw(2*nbytes) << setfill('0') << buf[i];
        if (i + 1 != N)
            cout << " ";
    }
    cout << endl;
}

template <typename T, size_t N>
void print_binary(T (&buf)[N])
{
    size_t nbytes = sizeof(buf[0]);

    for (int i = 0; i < N; i++)
    {
        cout << bitset<64>(buf[i]) << endl;
    }
    cout << endl;
}

void print_binary(uint64_t &buf)
{
    cout << bitset<64>(buf) << endl;
}

int main()
{
    cout << "Testing Device..." << endl;

    string dev_path = "/dev/utca_sp120";

    Device dev(dev_path);

    cout << "Openning ";
    dev.print();

    cout << "Testing Device.read..." << endl;

    uint64_t buff;
    size_t bytes_read;
    off_t off = 0x000B6138;

    uint64_t w_buff = 0x0000000004000000ULL;
    cout << "test data:" << endl;
    print_binary(w_buff);

    dev.write(&w_buff, 8, off);

    bytes_read = dev.read(&buff, 8, off);
    // bytes_read = pread(id, buff, 8, off);
    // buff[bytes_read] = 0; // terminate string
    cout << "read data:" << endl;
    print_binary(buff);

    cout << "Loading Dataset..." << endl;
    Dataset dataset("emtf_pcie_address_table.csv");

    Dataset addresses = dataset.subset("name", "ptlut_rd_latency");
    // addresses.print();
    for (Address adr : addresses.data())
    {
        buff = 0x0;
        dev.read(&buff, adr);
        cout << "read data:" << endl;
        print_binary(buff);
    }

    w_buff = 0x8ULL;
    cout << "test data:" << endl;
    print_binary(w_buff);

    for (Address adr : addresses.data())
    {
        dev.write(&w_buff, adr);

        buff = 0x0;
        dev.read(&buff, adr);
        cout << "read data:" << endl;
        print_binary(buff);
    }
}