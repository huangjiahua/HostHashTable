#include <iostream>
#include "include/HostHashTable.h"

using namespace std;

int main() {
    cout << "Hello, World!" << endl;
    HostHashTable hht(10, 50, 4, 4);
    cout << hht.memorySize() << endl;
    cout << (size_t)hht.dataAddress() << endl;
    return 0;
}