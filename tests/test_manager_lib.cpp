#include "../libraries/manager/lib.hpp"

#include <cassert>

bool TestCalcInodeBitmapBlocks() {
  if (CalcInodeBitmapBlocks(1) != 1)
    return false;
  if (CalcInodeBitmapBlocks(256) != 1)
    return false;
  if (CalcInodeBitmapBlocks(257) != 2)
    return false;
  if (CalcInodeBitmapBlocks(512) != 2)
    return false;
  if (CalcInodeBitmapBlocks(513) != 3)
    return false;

  return true;
}

bool TestCalcAllocationBitmapBlocks() {
  if (CalcAllocationBitmapBlocks(1) != 1)
    return false;
  if (CalcAllocationBitmapBlocks(32768) != 1)
    return false;
  if (CalcAllocationBitmapBlocks(32769) != 2)
    return false;
  if (CalcAllocationBitmapBlocks(65536) != 2)
    return false;
  if (CalcAllocationBitmapBlocks(65537) != 3)
    return false;

  return true;
}

int main() { 
  assert(TestCalcInodeBitmapBlocks());
  assert(TestCalcAllocationBitmapBlocks());
}