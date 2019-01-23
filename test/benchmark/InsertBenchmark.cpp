//
// Created by jiahua on 2019/1/23.
//

#include "../../include/HostHashTable.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <random>
const int TOTAL_NUM = 50000000;
using namespace std;

typedef std::chrono::time_point<std::chrono::steady_clock> tp;

using namespace std;

void generate_random_data(int *keys, int *values, int n) {
    default_random_engine en((unsigned int)chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<uint32_t> dis(0, n);
    for (uint32_t i = 0; i < n; i++) {
        keys[i] = dis(en);
        values[i] = 42;
    }
}

inline unsigned long long elapsed_time(tp from, tp to) {
    return static_cast<unsigned long long int>(chrono::duration_cast<chrono::milliseconds>(to - from).count());
}

int main() {
    HostHashTable hht(10, TOTAL_NUM, sizeof(int), sizeof(int));
    hht.createInsertBatch(TOTAL_NUM);
    hht.createFindBatch(TOTAL_NUM);
    int *key = new int[TOTAL_NUM];
    int *val = new int[TOTAL_NUM];
    auto *ret = new IstRet[TOTAL_NUM];
    auto *values = new HostDataBlock[TOTAL_NUM];
    generate_random_data(key, val, TOTAL_NUM);
    for (int i = 0; i < TOTAL_NUM; i++) {
        HostDataBlock key_blk{key + i, sizeof(int)};
        HostDataBlock val_blk{val + i, sizeof(int)};
        hht.addToInsertBatch(key_blk, val_blk);
        hht.addToFindBatch(key_blk);
    }

    auto bf_ins = chrono::steady_clock::now();
    hht.insertBatch(ret);
    auto af_ins = chrono::steady_clock::now();
    auto bf_fnd = chrono::steady_clock::now();
    hht.findBatch(values);
    auto af_fnd = chrono::steady_clock::now();

    cout << *(int*)values[TOTAL_NUM - 1].data << " " << values[TOTAL_NUM - 1].size
         << "  " << ret[TOTAL_NUM - 1] << endl;
    cout << "Total Inserting Time: " << elapsed_time(bf_ins, af_ins) << endl;
    cout << "Total Finding Time:   " << elapsed_time(bf_fnd, af_fnd) << endl;

    delete [] key;
    delete [] val;
    delete [] ret;
    delete [] values;
    return 0;
}