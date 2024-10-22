#ifndef CLIENT_LIB_HPP
#define CLIENT_LIB_HPP

#include <map>
#include "../core/utils.hpp"
#include "../core/sync.hpp"
#include "Handler.h"

extern int test_client_lib(int x);

class MFSClient {
public:
  int error;

  MFSClient();
  void mfs_mount(const char *path);

  int mfs_open(const char *name, int mode);
  int mfs_creat(const char *name, int mode);
  int mfs_close(int fd);
  int mfs_read(int fd, char *buf, int len);
  int mfs_write(int fd, const char *buf, int len);
  int mfs_lseek(int fd, int whence, int offset);
  int mfs_unlink(const char *name);

  //TODO w ostatecznej wersji zwraca tylko vector<string>
  std::vector<std::pair<uint32_t, std::string>> mfs_ls(const char *name);
  int mfs_mkdir(const char *name);
  int mfs_rmdir(char *name);

//private:
  int openAndSeek(const int& offset = 0) const;
  int getLowestDescriptor() const;

  std::vector<u_int32_t> getAllTakenBlocksInInode(const Inode &inode);
  u_int32_t blocksToWrite(const u_int32_t& fileOffset, const u_int32_t& length) const;
  Inode getInodeByIndex(u_int32_t index);
  //returns the block number from the sequence of saved blocks of the file

  uint32_t getBlockInFileByNumber(u_int32_t inode_idx, const Inode& inode, u_int32_t blockNumberInFile);
  uint32_t getBlockInFileByNumberIndirect(int disk_fd, u_int32_t inode_idx, const Inode &inode, u_int32_t blockNumberInFile);

  bool CheckIfInodeExists(const std::string& name, const u_int32_t& directoryInode);
  u_int32_t getInode(std::string path);
  u_int32_t getInodeFromDirectoryByName(const int& disk_fd, const std::string& filename, const u_int32_t& directoryInode);

  void addInodeToDirectory(const u_int32_t& directoryInodeIndex, const u_int32_t& inodeIndex, const std::string& name);
  void removeInodeFromDirectory(const u_int32_t& directoryInodeIndex, const u_int32_t& inodeToDelete);

  void directoryFill(int disk_fd, const u_int32_t& directoryInodeIndex,
          const u_int32_t& inodeToChange, const u_int32_t &newInode, const std::string& name);
  void writeInodeAndName(int disk_fd, const u_int32_t& directoryInodeIndex,
          const u_int32_t& newInodeIndex, const std::string& name);

  void makeRoot(); //creates root directory

  u_int32_t getAndTakeUpFirstFreeInode(); //return inode number
    //TODO think about: get n blocks by one call and return vector?
  u_int32_t getAndTakeUpFirstFreeBlock(); //returns block number
  u_int32_t getFirstFreeBitmapIndex(int disk_fd, u_int32_t offset, u_int32_t sizeInBlocks, u_int32_t amount) const;

  void freeInode(unsigned long index);
  void freeBlock(unsigned long index);
  void freeBitmapIndex(int disk_fd, u_int32_t offset, unsigned long index) const;

  void clearBlock(u_int32_t index);
  void clearInode(u_int32_t index);
  void clearIndirectBlocks(int disk, int index);


  //functor when operation fails, e.g. unlock sync
  void readFromDisk(int disk_fd, void *buf, size_t size, std::function<void()> functor);
  void writeToDisk(int disk_fd, const void *buf, size_t size, std::function<void()> functor);
  void lseekOnDisk(int disk_fd, off_t offset, int whence, std::function<void()> functor);




  u_int32_t blockSize;
  u_int32_t inodeSize;

  //number of blocks for each objects
  u_int32_t inodes;
  u_int32_t inodesBitmap;
  u_int32_t blocks;
  u_int32_t allocationBitmap;

  //offsets in bytes
  u_int32_t inodesBitmapOffset;
  u_int32_t inodesOffset;
  u_int32_t allocationBitmapOffset;
  u_int32_t blocksOffset;

  std::string disk_path;
  //key - file descriptor, value - open file structure
  std::map<u_int32_t, OpenFile> open_files;
  MockSyncClient sync_client;
};

#endif
