#include "emtf_dataset.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
    cout << "Testing EMTF Dataset..." << endl;

    Dataset dataset("emtf_pcie_address_table.csv");

    cout << "Dataset fname:    " << dataset.fname << endl;
    cout << "--- Dataset Size: " << dataset.size() << endl;

    cout << "Printing Dataset" << endl;
    dataset.print();

    cout << "Filter only ME1" << endl;
    Dataset subset = dataset.subset("chamber", "ME1.*");

    subset.print();
}