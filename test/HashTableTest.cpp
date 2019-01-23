//
// Created by jiahua on 2019/1/23.
//

#include "../include/HostHashTable.h"
#include <iostream>
#include <iomanip>
const int TOTAL_NUM = 500;
using namespace std;

int main() {
    HostHashTable hht(10, TOTAL_NUM, sizeof(int), sizeof(int));
    hht.createInsertBatch(TOTAL_NUM);
    hht.createFindBatch(TOTAL_NUM);
    int key[TOTAL_NUM];
    int val[TOTAL_NUM];
    IstRet ret[TOTAL_NUM];
    HostDataBlock values[TOTAL_NUM];
    for (int i = 0; i < TOTAL_NUM; i++) {
        key[i] = i;
        val[i] = i + 1;
        HostDataBlock key_blk{key + i, sizeof(int)};
        HostDataBlock val_blk{val + i, sizeof(int)};
        hht.addToInsertBatch(key_blk, val_blk);
        hht.addToFindBatch(key_blk);
    }

    hht.insertBatch(ret);

    hht.findBatch(values);

    for (int i = 0; i < TOTAL_NUM; i++) {
        cout << setw(3) << i << " --- "
             << setw(3) << *(int*)values[i].data << " --- " << values[i].size << endl;
    }


    return 0;
}