#include "../libraries/core/sync.hpp"

#include <iostream>
#include <string>

const std::string path = "./mfs";

int main() {
  SyncClient sync_client;
  sync_client.Init(path);

  std::string command;

  std::cout << ">>> ";
  while (std::cin >> command) {
    if (command == "al") {
      sync_client.AllocationBitmapLock();
      std::cout << "AllocationBitmapLock" << std::endl;
    }
    if (command == "au") {
      sync_client.AllocationBitmapUnlock();
      std::cout << "AllocationBitmapUnlock" << std::endl;
    }

    if (command == "rl") {
      sync_client.ReadLock(1);
      std::cout << "ReadLock" << std::endl;
    }
    if (command == "ru") {
      sync_client.ReadUnlock(1);
      std::cout << "ReadUnlock" << std::endl;
    }

    if (command == "wl") {
      sync_client.WriteLock(1);
      std::cout << "WriteLock" << std::endl;
    }
    if (command == "wu") {
      sync_client.WriteUnlock(1);
      std::cout << "WriteUnlock" << std::endl;
    }
    std::cout << ">>> ";
  }
}