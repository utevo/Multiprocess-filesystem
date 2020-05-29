#include "../libraries/core/sync.hpp"

#include <string>

const std::string path = "./mfs";

int main() {
  SyncClient sync_client;
  sync_client.Init(path);

  while (int c = getchar()) {
    getchar();
    if (c == 'l') {
      sync_client.AllocationBitmapLock();
      std::cout << "Lock" << std::endl;
    }
    if (c == 'u') {
      sync_client.AllocationBitmapUnlock();
      std::cout << "UnLock!" << std::endl;
    }
  }
}