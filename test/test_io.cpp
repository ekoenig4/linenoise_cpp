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
    cout << "Testing IO" << endl;

    string device = "/dev/utca_sp120";
    int id = ::open(device.c_str(), O_RDWR);

    cout << "Openned " << device << " as " << id << endl;

    uint64_t buff = 0x0;
    // off_t off = 0x000B6088;
    // off_t off = 0x000B6058;
    off_t off = 0x000B6138;
    int count = 8;

    cout << "Testing Address IO: 0x" << hex << off << endl;

    int ret = pread(id, &buff, count, off);

    cout << "read  block(" << ret << "): ";
    print_binary(buff);

    buff = 0x0000000004000000ULL;
    ret = pwrite(id, &buff, count, off);

    cout << "write block(" << ret << "): ";
    print_binary(buff);

    buff = 0x0;
    ret = pread(id, &buff, count, off);

    cout << "read  block(" << ret << "): ";
    print_binary(buff);

    return 0;
}