#include "lib.hpp"

#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_client_lib(int x) { return 1 * x; }

int MFSClient::read(int fd, char *buf, int len) {
    u_int32_t open_file_idx = file_descriptions.at(fd);
    OpenFile open_file = open_files.at(open_file_idx);
    u_int32_t inode_idx = open_file.inode_idx;

    sync_client.ReadLock(inode_idx);

    // ToDo

    sync_client.ReadUnlock(inode_idx);
}