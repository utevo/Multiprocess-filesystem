#include "sync.hpp"
#include "utils.hpp"


#include <iostream>

extern const u_int32_t kBlockSize = 4096;
extern const u_int32_t kInodeSize = 32;

unsigned long myCeil(unsigned long first, unsigned long second) {
    return first / second + ((first % second != 0) ? 1 : 0);
}
