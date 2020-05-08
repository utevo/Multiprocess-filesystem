#ifndef CLIENT_SYNC_HPP
#define CLIENT_SYNC_HPP

#include <iostream>

class SyncClient {
public:
  void init(std::string path);
  void readLock(u_int index);
  void readUnlock(u_int index);
  void writeLock(u_int index);
  void writeUnlock(u_int index);
};

#endif