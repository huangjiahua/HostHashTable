//
// Created by jiahua on 2019/1/23.
//

#include "../include/HostHashTable.h"
#include <cstdlib>
#include <cstring>
#include "HashFunc.h"


HostHashTable::HostHashTable(
      uint32_t bkt_size,
      uint32_t bkt_cnt,
      uint32_t max_key_size,
      uint32_t max_val_size,
      uint32_t overflow_cnt):
      _bkt_cnt(bkt_cnt),
      _bkt_elem_cnt(bkt_size),
      _max_key_size(max_key_size),
      _max_elem_size(max_key_size + max_val_size),
      _overflow_cnt(overflow_cnt) {
    const size_type all_data_size = _max_elem_size * ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt );
    const size_type all_stat_size = ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt );
    const size_type all_size_size = 2 * ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt);
    const size_type all_bkt_info_size = ( _bkt_cnt + 1 );

    _data_ptr = new unsigned char[all_data_size];
    _data_info_ptr = new status_type[all_stat_size];
    _elem_info_ptr = new size_type[all_size_size];
    _bkt_info_ptr = new size_type[all_bkt_info_size];

    std::memset(_bkt_info_ptr, 0x00, all_bkt_info_size * sizeof(size_type));
    std::memset(_data_info_ptr, 0x00, all_stat_size * sizeof(status_type));
}

HostHashTable::~HostHashTable() {
    delete [] _data_ptr;
    delete [] _data_info_ptr;
    delete [] _elem_info_ptr;
    delete [] _bkt_info_ptr;
    delete [] _in_buf;
    delete [] _out_buf;
}

uint32_t HostHashTable::memorySize() const {
    const size_type all_data_size = _max_elem_size * ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt );
    const size_type all_stat_size = ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt );
    const size_type all_size_size = 2 * ( _bkt_cnt * _bkt_elem_cnt + _overflow_cnt);
    const size_type all_bkt_info_size = ( _bkt_cnt + 1 );
    return all_data_size + all_stat_size * sizeof(status_type) +
           (all_size_size + all_bkt_info_size) * sizeof(size_type) + sizeof(HostHashTable);
}

uint32_t HostHashTable::maxElementCount() const {
    return _bkt_elem_cnt * _bkt_cnt + _overflow_cnt;
}

uint32_t HostHashTable::maxKeySize() const {
    return _max_key_size;
}

uint32_t HostHashTable::maxValueSize() const {
    return _max_elem_size - _max_key_size;
}

uint32_t HostHashTable::bucketCount() const {
    return _bkt_cnt;
}

void *HostHashTable::bucketInfoAddress() const {
    return (void*)_bkt_info_ptr;
}

void *HostHashTable::elementInfoAddress() const {
    return (void*)_elem_info_ptr;
}

void *HostHashTable::dataAddress() const {
    return (void*)_data_ptr;
}

void HostHashTable::createInsertBatch(uint32_t size) {
    if (size == 0) return;
    delete [] _in_buf;
    _in_buf = new HostDataBlock[2 * size];
    _in_buf_cap = size;
    _in_buf_size = 0;
}

void HostHashTable::addToInsertBatch(const HostDataBlock &key, const HostDataBlock &value) {
    _in_buf[_in_buf_size * 2] = key;
    _in_buf[_in_buf_size * 2 + 1] = value;
    _in_buf_size++;
}

uint32_t HostHashTable::currInsertBatchSize() const {
    return _in_buf_size;
}

void HostHashTable::insertBatch(IstRet *ret_code) {
    for (size_type i = 0; i < _in_buf_size; i++) {
        IstRet ret = insert(_in_buf[2 * i], _in_buf[2 * i + 1]);
        if (ret_code != nullptr)
            ret_code[i] = ret;
    }
    delete [] _in_buf;
    _in_buf = nullptr;
    _in_buf_cap = _in_buf_size = 0;
}

IstRet HostHashTable::insert(const HostDataBlock &key, const HostDataBlock &value) {
    size_type bkt_no = __hash_func1(key.data, key.size) % _bkt_cnt;
    size_type dst = _bkt_info_ptr[bkt_no];
    IstRet ret = IstRet::SUCCESSFUL;

    if (dst >= _bkt_elem_cnt) {
        ret = IstRet::OVERFLOWED;
        bkt_no = _bkt_cnt;
        dst = _bkt_info_ptr[bkt_no];
        if (dst >= _overflow_cnt) {
            return IstRet::FULL;
        }
    }

    size_type *size_p = getKeySzPtr(bkt_no, dst);
    unsigned char *data_p = getDataPtr(bkt_no, dst);

    size_p[0] = key.size;
    size_p[1] = value.size;
    std::memcpy(data_p, key.data, key.size);
    std::memcpy(data_p + _max_key_size, value.data, value.size);
    ++_bkt_info_ptr[bkt_no];
    return ret;
}

uint32_t HostHashTable::currInsertBatchCapacity() const {
    return _in_buf_cap;
}

void HostHashTable::createFindBatch(uint32_t size) {
    if (size == 0) return;
    delete [] _out_buf;
    _out_buf = new HostDataBlock[size];
    _out_buf_cap = size;
    _out_buf_size = 0;
}

uint32_t HostHashTable::currFindBatchSize() const {
    return _out_buf_size;
}

uint32_t HostHashTable::currFindBatchCapacity() const {
    return _out_buf_cap;
}

void HostHashTable::addToFindBatch(const HostDataBlock &key) {
    _out_buf[_out_buf_size++] = key;
}

void HostHashTable::findBatch(HostDataBlock *values) {
    for (size_type i = 0; i < _out_buf_size; i++) {
        find(_out_buf[i], values[i]);
    }
    _out_buf_size = _out_buf_cap = 0;
    delete [] _out_buf;
    _out_buf = nullptr;
}

void HostHashTable::find(const HostDataBlock &key, HostDataBlock &value) {
    size_type bkt_no = __hash_func1(key.data, key.size) % _bkt_cnt;
    size_type elem_cnt = _bkt_info_ptr[bkt_no];
    unsigned char *data_p = getDataPtr(bkt_no, 0);
    size_type *size_p = getKeySzPtr(bkt_no, 0);

    int i = 0;
    for (; i < elem_cnt && _bkt_elem_cnt;
           i++, data_p += _max_elem_size, size_p += 2) {
        if (key.size == size_p[0] && std::memcmp(data_p, key.data, key.size) == 0)
            break;
    }

    if (i >= elem_cnt || i >= _bkt_elem_cnt) {
        bkt_no = _bkt_cnt;
        elem_cnt = _bkt_info_ptr[bkt_no];
        data_p = getDataPtr(bkt_no, 0);
        size_p = getKeySzPtr(bkt_no, 0);
        for (; i < elem_cnt && i < _overflow_cnt;
               i++, data_p += _max_elem_size, size_p += 2) {
            if (key.size == size_p[0] && std::memcmp(data_p, key.data, key.size) == 0)
                break;
        }

        if (i >= elem_cnt || i >= _overflow_cnt) {
            value.size = 0;
            value.data = nullptr;
            return;
        }
    }

    value.size = size_p[1];
    value.data = data_p + _max_key_size;
}

HostHashTable::size_type
*HostHashTable::getKeySzPtr(HostHashTable::size_type bkt_no, HostHashTable::size_type dst) {
    return _elem_info_ptr + ( bkt_no * _bkt_elem_cnt + dst ) * 2;
}

unsigned char *
HostHashTable::getDataPtr(HostHashTable::size_type bkt_no, HostHashTable::size_type dst) {
    return _data_ptr + ( bkt_no * _bkt_elem_cnt + dst ) * _max_elem_size;
}







