//
// Created by jiahua on 2019/1/23.
//

#include "HashFunc.h"

#define	DCHARHASH(h, c)	((h) = 0x63c63cd9*(h) + 0x9c39c33d + (c))


uint32_t
__hash_func1(const void *key, uint32_t len) {
    const uint8_t *e, *k;
    uint32_t h;
    uint8_t c;

    k = reinterpret_cast<const uint8_t *>(key);
    e = k + len;
    for (h = 0; k != e;) {
        c = *k++;
        if (!c && k > e)
            break;
        DCHARHASH(h, c);
    }
    return (h);
}
