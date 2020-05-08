#ifndef SYNC_HPP
#define SYNC_HPP

#include <iostream>

void InitSync(std::string path);

class SyncClient {
public:
  void Init(std::string path);
  void ReadLock(u_int32_t inode_idx);
  void ReadUnlock(u_int32_t inode_idx);
  void WriteLock(u_int32_t inode_idx);
  void WriteUnlock(u_int32_t inode_idx);

  void AllocationBitmapLock();
  void AllocationBitmapUnlock();
};

#endif