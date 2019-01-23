//
// Created by jiahua on 2019/1/23.
//

#ifndef HOSTHASHTABLE_HOSTHASHTABLE_H
#define HOSTHASHTABLE_HOSTHASHTABLE_H
#include <cstdint>


#define EMPTY          (0)
#define OCCUPIED       (1)
#define VALID          (2)
#define READING        (3)


enum IstRet {
    SUCCESSFUL = 0,
    UNKNOWN,
    OVERFLOWED,
    FULL,
    MODIFIED,
    DUPLICATE
};

struct HostDataBlock {
    void *data;
    uint32_t size;
};

class HostHashTable {
public:
    typedef uint32_t size_type;
    typedef uint32_t status_type;
private:
    unsigned char *_data_ptr = nullptr;
    status_type  *_data_info_ptr = nullptr;
    size_type *_elem_info_ptr = nullptr, *_bkt_info_ptr = nullptr;
    HostDataBlock *_in_buf = nullptr, *_out_buf = nullptr;

    size_type _bkt_cnt, _bkt_elem_cnt, _max_key_size, _max_elem_size;
    size_type _in_buf_cap, _out_buf_cap, _in_buf_size, _out_buf_size;
    size_type _overflow_cnt;

    size_type *getKeySzPtr(size_type bkt_no, size_type dst);
    unsigned char *getDataPtr(size_type bkt_no, size_type dst);
public:
    HostHashTable(uint32_t bkt_size,
                  uint32_t bkt_cnt,
                  uint32_t max_key_size,
                  uint32_t max_val_size,
                  uint32_t overflow_cnt = 1000);
    ~HostHashTable();

    uint32_t memorySize() const;
    uint32_t maxElementCount() const;
    uint32_t maxKeySize() const;
    uint32_t maxValueSize() const;
    uint32_t bucketCount() const;
    void *bucketInfoAddress() const;
    void *elementInfoAddress() const;
    void *dataAddress() const;

    void createInsertBatch(uint32_t size);
    void addToInsertBatch(const HostDataBlock &key, const HostDataBlock &value);
    uint32_t currInsertBatchSize() const;
    uint32_t currInsertBatchCapacity() const;
    void insertBatch(IstRet *ret_code = nullptr);

    void createFindBatch(uint32_t size);
    void addToFindBatch(const HostDataBlock &key);
    uint32_t currFindBatchSize() const;
    uint32_t currFindBatchCapacity() const;
    void findBatch(HostDataBlock *values);

    IstRet insert(const HostDataBlock &key, const HostDataBlock &value);
    void find(const HostDataBlock &key, HostDataBlock &value);
};


#endif //HOSTHASHTABLE_HOSTHASHTABLE_H
