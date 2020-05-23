#include "sync.hpp"

#include <iostream>

void SyncClient::ReadLock(u_int32_t inode_idx) {}

void SyncClient::ReadUnlock(u_int32_t inode_idx) {}

void SyncClient::WriteLock(u_int32_t inode_idx) {}

void SyncClient::WriteUnlock(u_int32_t inode_idx) {}

void SyncClient::AllocationBitmapLock() {}

void SyncClient::AllocationBitmapUnlock() {}

void SyncClient::InodeBitmapLock() {}

void SyncClient::InodeBitmapUnlock() {}



}
