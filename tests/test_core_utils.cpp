#include "../libraries/core/utils.hpp"

#include <cassert>


bool TestSizes() {
  if (sizeof(Superblock) != kBlockSize)
    return false;
  if (sizeof(Inode) != kInodeSize)
    return false;

  return true;
}

int main() { 
  assert(TestSizes());
}