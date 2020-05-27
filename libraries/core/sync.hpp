#ifndef SYNC_HPP
#define SYNC_HPP

#include <iostream>
#include <vector>

std::vector<u_int8_t> SerializeLong(long value);
std::vector<u_int8_t> GenerateMessageBuf(long mtype, std::vector<u_int8_t> data_);

extern void InitSync(const std::string path);
extern void RemoveSync(const std::string path);

class SyncClient {
public:
  void Init(std::string path);
  void ReadLock(u_int32_t inode_idx);
  void ReadUnlock(u_int32_t inode_idx);
  void WriteLock(u_int32_t inode_idx);
  void WriteUnlock(u_int32_t inode_idx);

  void AllocationBitmapLock();
  void AllocationBitmapUnlock();

  void InodeBitmapLock();
  void InodeBitmapUnlock();
};

#endif