#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cmath>
#include "lib.hpp"

#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_client_lib(int x) { return 1 * x; }

extern const u_int32_t kBlockSize;
extern const u_int32_t kInodeSize;

const unsigned NORMAL_FILE = 0;
const unsigned DIRECTORY = 1;
const unsigned NAME_SIZE = 28;

MFSClient::MFSClient() {
    error = 0;
}

void MFSClient::mfs_mount(char *path) {
    disk_path = path;
    int fd = openAndSeek();
    if (read(fd, &blockSize, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read block size from super block");
    if (read(fd, &inodeSize, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read inode size from super block");
    if (read(fd, &inodes, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of inodes blocks from super block");
    inodes *= blockSize / inodeSize;
    inodesOffset = blockSize;
    if (read(fd, &inodesBitmap, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of inodes bitmap blocks from super block");
    inodesBitmapOffset = inodesOffset + inodes * blockSize;
    if (read(fd, &allocationBitmap, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read number of allocations bitmap from super block");
    allocationBitmapOffset = inodesBitmapOffset + inodesBitmap * blockSize;
    if (read(fd, &blocks, sizeof(u_int32_t)) <= 0)
        throw std::ios_base::failure("Cannot read block size from super block");
    blocksOffset = allocationBitmapOffset + allocationBitmap * blockSize;
    close(fd);

    //TODO create root directory as inode which index is 0
    //manually do mfs_mkdir("/")
}

int MFSClient::mfs_open(char *name, int mode) {
    try {
        u_int32_t inodeIndex = getInode(name);
        OpenFile openFile{Handler::getStatus(mode), 0, inodeIndex};
        int fd = getLowestDescriptor();
        open_files.insert({fd, openFile});
        return fd;
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
}

int MFSClient::mfs_creat(char *name, int mode) {
    int disk;
    try {
        u_int32_t inodeIndex = getAndTakeUpFirstFreeInode();
        disk = openAndSeek();
        std::string filename = Handler::getFileName(name);
        std::string directoryPath = Handler::getDirectory(name);
        u_int32_t directoryInodeIndex = getInode(directoryPath);

        addInodeToDirectory(directoryInodeIndex, inodeIndex, filename);

        Inode inode;
        inode.valid = 1;
        inode.type = NORMAL_FILE;
        inode.size = 0;

        sync_client.WriteLock(inodeIndex);
        LseekOnDisk(disk, inodesOffset + inodeIndex * inodeSize, SEEK_SET,
                    [&]() { sync_client.WriteUnlock(inodeIndex); });
        WriteToDisk(disk, &inode, sizeof(Inode),
                    [&]() { sync_client.WriteUnlock(inodeIndex); });

        close(disk);
        sync_client.WriteUnlock(inodeIndex);

        OpenFile openFile{WRONLY, 0, inodeIndex};
        int fd = getLowestDescriptor();
        open_files.insert({fd, openFile});

        return fd;
    }
    catch (std::exception &) {
        close(disk);
        return -1;
    }
}

int MFSClient::mfs_read(int fd, char *buf, int len) {
    OpenFile open_file = open_files.at(fd);
    u_int32_t inode_idx = open_file.inode_idx;

    sync_client.ReadLock(inode_idx);

    // TODO

    sync_client.ReadUnlock(inode_idx);
}

int MFSClient::mfs_write(int fd, char *buf, int len) {
    return 0;
}

int MFSClient::mfs_lseek(int fd, int whence, int offset) {
    return 0;
}

int MFSClient::mfs_unlink(char *name) {
    try {
        u_int32_t inodeToDelete = getInode(name);
        u_int32_t parentInode = getInode(Handler::getDirectory(name));

        removeInodeFromDirectory(parentInode, inodeToDelete);
        freeInode(inodeToDelete);

    } catch (std::exception &) {
        return -1;
    }
}

int MFSClient::mfs_mkdir(char *name) {
    //get both names 
    std::vector<std::string> directories = split(name, '/');
    std::string newName, parentName;
    if (directories.size() == 0) throw std::invalid_argument("Name cannot be empty");
    if (directories.size() == 1) {
        newName = directories[0];
        parentName = '/';
    } else {
        newName = directories[directories.size() - 1];
        parentName = directories[directories.size()];
    }

    //get parent inode:
    u_int32_t parent = getParentInode(name);
    //take up and get new dir inode:
    u_int32_t inodeIndex = getAndTakeUpFirstFreeInode();
    sync_client.WriteLock(inodeIndex);
    //set new inode object:
    int disk = openAndSeek(inodes + inodeIndex * inodeSize);
    Inode inode;
    inode.valid = 1;
    inode.type = DIRECTORY;
    inode.size = 0; //TODO determine size

    //get first free block:
    u_int32_t blockIndex = getAndTakeUpFirstFreeBlock();
    inode.direct_idxs[0] = blockIndex;

    //write ., .. references in directory memory block:
    disk = openAndSeek(blocks + blockIndex * blockSize);
    u_int32_t buf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    buf[0] = inodeIndex; //reference to .
    buf[1] = '.';
    buf[8] = inodeIndex; //TODO add reference to .., getInode(string path) needed
    buf[9] = '.' + 256 * '.';
    write(disk, &buf, sizeof(buf));

    //write the inode:
    int result = write(disk, &inode, sizeof(Inode));
    close(disk);
    if (result < 0)
        return -1;
    sync_client.WriteUnlock(inodeIndex);

    return 0;
}

int MFSClient::mfs_rmdir(char *name) {
    //free all blocks
    //free inode
    //remove from parent
    return 0;
}

int MFSClient::openAndSeek(const int &offset) const {
    if (offset < 0)
        throw std::invalid_argument("Offset cannot be less than zero");

    int fd = open(disk_path.c_str(), O_RDWR);
    if (fd == -1)
        throw std::ios_base::failure("Cannot open virtual disk");
    int seek = lseek(fd, offset, SEEK_CUR);
    if (seek == -1)
        throw std::ios_base::failure("Error during lseek on virtual disk");
    return fd;
}

int MFSClient::getLowestDescriptor() const {
    int lowestFd = 1;
    while (open_files.find(lowestFd) != open_files.end())
        ++lowestFd;
    return lowestFd;
}

u_int32_t MFSClient::getInode(std::string path) {
    int disk = openAndSeek(inodesOffset);
    std::vector<std::string> directories = split(path, '/');
    u_int32_t currentInode = 0;
    try {
        for (const auto &name : directories)
            currentInode = getInodeFromDirectoryByName(disk, name, currentInode);
    }
    catch (std::exception &e) {
        close(disk);
        throw e;
    }
    close(disk);
    return currentInode;
}

u_int32_t MFSClient::getParentInode(std::string path) {
    int disk = openAndSeek(inodesOffset);
    std::vector<std::string> directories = split(path, '/');
    u_int32_t currentInode = 0;
    for (unsigned int iter = 0; iter < directories.size() - 1; ++iter) {
        currentInode = getInodeFromDirectoryByName(disk, directories[iter], currentInode);
    }
    return currentInode;
}

u_int32_t MFSClient::getInodeFromDirectoryByName(const int &disk_fd,
                                                 const std::string &filename,
                                                 const u_int32_t &directoryInode) {
    sync_client.ReadLock(directoryInode);
    LseekOnDisk(disk_fd, inodesOffset + directoryInode * inodeSize, SEEK_SET,
                [&]() { sync_client.ReadUnlock(directoryInode); });
    Inode inode;
    ReadFromDisk(disk_fd, &inode, sizeof(Inode), [&]() { sync_client.ReadUnlock(directoryInode); });

    if (inode.type != DIRECTORY) {
        sync_client.ReadUnlock(directoryInode);
        throw std::invalid_argument("Inode is not directory");
    }
    //foreach data block
    for (const auto &block : inode.direct_idxs) {
        if (directoryInode > 0 && block == 0)
            break;
        u_int32_t index;
        char buffer[NAME_SIZE];
        LseekOnDisk(disk_fd, blocksOffset + block * blockSize, SEEK_SET,
                    [&]() { sync_client.ReadUnlock(directoryInode); });
        //foreach rows in data block, row is inode and name
        do {
            ReadFromDisk(disk_fd, &index, sizeof(u_int32_t), [&]() { sync_client.ReadUnlock(directoryInode); });
            ReadFromDisk(disk_fd, buffer, NAME_SIZE, [&]() { sync_client.ReadUnlock(directoryInode); });
            std::string name = trim(std::string(buffer));
            if (name == filename) {
                sync_client.ReadUnlock(directoryInode);
                return index;
            }
        } while (index != 0);
    }
    sync_client.ReadUnlock(directoryInode);
    throw std::invalid_argument("Directory inode is incorrect");
}

void MFSClient::addInodeToDirectory(const u_int32_t &directoryInodeIndex, const u_int32_t &inodeIndex,
                                    const std::string &name) {
    int disk_fd = openAndSeek(inodesOffset + directoryInodeIndex * inodeSize);
    try {
        directoryFill(disk_fd, directoryInodeIndex, 0, inodeIndex, name);
    }
    catch (std::exception &e) {
        close(disk_fd);
        throw e;
    }
}

void MFSClient::removeInodeFromDirectory(const u_int32_t &directoryInodeIndex, const u_int32_t &inodeToDelete) {
    int disk_fd = openAndSeek(inodesOffset + directoryInodeIndex * inodeSize);
    try {
        directoryFill(disk_fd, directoryInodeIndex, inodeToDelete, 0, "\0");
    }
    catch (std::exception &e) {
        close(disk_fd);
        throw e;
    }
}

void MFSClient::directoryFill(int disk_fd,
                              const u_int32_t &directoryInodeIndex,
                              const u_int32_t &inodeToChange,
                              const u_int32_t &newInode,
                              const std::string &name) {
    sync_client.WriteLock(directoryInodeIndex);
    Inode directoryInode;
    ReadFromDisk(disk_fd, &directoryInode, sizeof(Inode),
                 [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
    for (const auto &block : directoryInode.direct_idxs) {
        u_int32_t index;
        LseekOnDisk(disk_fd, blocksOffset + block * blockSize, SEEK_SET,
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
        //foreach rows in data block, row is inode and name
        int row = 0;
        do {
            ReadFromDisk(disk_fd, &index, sizeof(u_int32_t),
                         [&]() { sync_client.WriteUnlock(directoryInodeIndex); });

            if (index == inodeToChange) {
                LseekOnDisk(disk_fd, row * 32, SEEK_SET,
                            [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
                //TODO operation
                writeInodeAndName(disk_fd, directoryInodeIndex, newInode, name);
                sync_client.WriteUnlock(directoryInodeIndex);
                return;
            }
            row++;
        } while (index != 0);
    }
    sync_client.WriteUnlock(directoryInodeIndex);
    throw std::domain_error("Cannot change directory");
}

void MFSClient::writeInodeAndName(
        int disk_fd, const u_int32_t &directoryInodeIndex,
        const u_int32_t &newInodeIndex, const std::string &name) {
    if (newInodeIndex != 0) {
        WriteToDisk(disk_fd, &newInodeIndex, sizeof(u_int32_t),
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
        WriteToDisk(disk_fd, name.c_str(), sizeof(name.c_str()),
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
    } else {
        char buff[32];
        for (int i = 0; i < 32; ++i)
            buff[i] = '\0';
        WriteToDisk(disk_fd, &buff, sizeof(32),
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
    }
}

u_int32_t MFSClient::getAndTakeUpFirstFreeBlock() {
    sync_client.AllocationBitmapLock();
    int disk = openAndSeek(allocationBitmapOffset);
    int blockNumber;
    try {
        blockNumber = getFirstFreeBitmapIndex(disk, allocationBitmapOffset, allocationBitmap, blocks);
    }
    catch (std::out_of_range &e) {
        close(disk);
        sync_client.AllocationBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.AllocationBitmapUnlock();
    return blockNumber;
}

u_int32_t MFSClient::getAndTakeUpFirstFreeInode() {
    sync_client.InodeBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    int inodeNumber;
    try {
        inodeNumber = getFirstFreeBitmapIndex(disk, inodesBitmapOffset, inodesBitmap, inodes);
    }
    catch (std::exception &e) {
        close(disk);
        sync_client.InodeBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.InodeBitmapUnlock();
    return inodeNumber;
}

u_int32_t MFSClient::getFirstFreeBitmapIndex(
        int disk_fd, u_int32_t offset, u_int32_t sizeInBlocks, u_int32_t amount) const {
    u_int64_t maxValue = 0xFFFFFFFFFFFFFFFF;
    unsigned long indexNumber = 0;

    for (int i = 0; i < sizeInBlocks; ++i) {
        std::vector<u_int64_t> bitmap(blockSize / sizeof(u_int64_t));
        unsigned long unreadIndexes = amount - (i * blockSize * 8);
        bool stop = false;
        unsigned long uints64ToRead = unreadIndexes > blockSize * 8 ?
                                      bitmap.size() :
                                      myCeil(unreadIndexes, (sizeof(u_int64_t) * 8));

        unsigned long readCount = uints64ToRead * sizeof(u_int64_t);
        if (read(disk_fd, bitmap.data(), readCount) < 0)
            throw std::ios_base::failure("Cannot read bitmap block");

        unsigned long endLoop = myCeil(readCount, sizeof(u_int64_t));
        if ((unreadIndexes % 64 == 0) && (endLoop < bitmap.size()))
            endLoop++;

        for (int k = 0; k < endLoop; ++k) {
            if (bitmap[k] == maxValue) {
                indexNumber += 64;
                continue;
            }
            u_int64_t tmp = 0x0000000000000001;
            while (tmp <= maxValue) {
                if (indexNumber >= amount)
                    throw std::out_of_range("");
                if ((bitmap[k] & tmp) == 0) {
                    bitmap[k] |= tmp;
                    if (lseek(disk_fd, offset + i * blockSize, SEEK_SET) < 0)
                        throw std::ios_base::failure("Cannot seek on bitmap");
                    if (write(disk_fd, bitmap.data(), readCount) < 0)
                        throw std::ios_base::failure("Cannot seek on bitmap");
                    stop = true;
                    break;
                }
                tmp <<= 1;
                indexNumber++;
            }
            if (stop)
                break;
        }
        if (stop)
            break;
    }
    return indexNumber;
}

void MFSClient::freeInode(unsigned long index) {
    if (index > inodes || index < 0)
        throw std::invalid_argument("Invalid inode index");
    sync_client.InodeBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    try {
        freeBitmapIndex(disk, inodesBitmapOffset, index);
        clearInode(index);
    }
    catch (std::exception &e) {
        close(disk);
        sync_client.InodeBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.InodeBitmapUnlock();
}

void MFSClient::clearInode(u_int32_t index) {
    int disk = openAndSeek(inodesOffset + inodeSize * index);
    sync_client.WriteLock(index);
    Inode inode;
    ReadFromDisk(disk, &inode, sizeof(Inode), [&]() {
        sync_client.WriteUnlock(index);
        close(disk);
    });
    inode.valid = 0;
    inode.size = 0;
    //clear direct blocks
    for (auto &block : inode.direct_idxs) {
        if (block == 0)
            break;
        freeBlock(block);
        block = 0;
    }
    //clear indirect blocks
    if (inode.indirect_idx != 0) {
        clearIndirectBlocks(disk, inode.indirect_idx);
    }
    //clear doubly indirect blocks
    if (inode.double_indirect_idx != 0) {
        LseekOnDisk(disk, blocksOffset + inode.double_indirect_idx, SEEK_SET,
                    [&]() {
                        sync_client.WriteUnlock(index);
                        close(disk);
                    });
        std::vector<u_int32_t> indirectBlockIndexes(blockSize / sizeof(u_int32_t));
        ReadFromDisk(disk, indirectBlockIndexes.data(), blockSize, [&]() {
                         sync_client.WriteUnlock(index);
                         close(disk);
                     }
        );
        for (auto &block : indirectBlockIndexes) {
            if (block == 0)
                break;
            clearIndirectBlocks(disk ,index);
        }
        freeBlock(inode.double_indirect_idx);
        freeBlock(index);
    }
    //write cleared inode
    LseekOnDisk(disk, inodesOffset + inodeSize * index, SEEK_SET,
                [&]() {
                    sync_client.WriteUnlock(index);
                    close(disk);
                });
    WriteToDisk(disk, &inode, sizeof(Inode), [&]() {
        sync_client.WriteUnlock(index);
        close(disk);
    });
    sync_client.WriteUnlock(index);
    close(disk);
}

void MFSClient::clearIndirectBlocks(int disk, int index) {
    LseekOnDisk(disk, blocksOffset + index, SEEK_SET,
                [&]() {
                    sync_client.WriteUnlock(index);
                    close(disk);
                });
    std::vector<u_int32_t> blockIndexes(blockSize / sizeof(u_int32_t));
    ReadFromDisk(disk, blockIndexes.data(), blockSize, [&]() {
                     sync_client.WriteUnlock(index);
                     close(disk);
                 }
    );
    for (auto &block : blockIndexes) {
        if (block == 0)
            break;
        freeBlock(block);
    }
    freeBlock(index);
}


void MFSClient::freeBlock(unsigned long index) {
    if (index > blocks || index < 0)
        throw std::invalid_argument("Invalid block index");
    sync_client.AllocationBitmapLock();
    int disk = openAndSeek(allocationBitmapOffset);
    try {
        freeBitmapIndex(disk, allocationBitmapOffset, index);
        clearBlock(index);
    }
    catch (std::exception &e) {
        close(disk);
        sync_client.AllocationBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.AllocationBitmapUnlock();
}

void MFSClient::clearBlock(u_int32_t index) {
    int disk = openAndSeek(blocks + blockSize * index);
    char buff[blockSize];
    for (auto &byte : buff)
        byte = '\0';
    WriteToDisk(disk, buff, sizeof(blockSize), [&]() { close(disk); });
    close(disk);
}

void MFSClient::freeBitmapIndex(int disk_fd, u_int32_t offset, unsigned long index) const {
    unsigned int blockNumber = index / (blockSize * 8);
    unsigned int indexInBlock = index - (blockNumber * blockSize * 8);
    unsigned int offsetToReadUInt8 = indexInBlock / (sizeof(uint8_t) * 8);
    u_int8_t bitmap;

    if (lseek(disk_fd, offset + blockNumber * blockSize + offsetToReadUInt8, SEEK_SET) < 0)
        throw std::ios_base::failure("Cannot seek bitmap block");
    if (read(disk_fd, &bitmap, sizeof(u_int8_t)) < 0)
        throw std::ios_base::failure("Cannot read bitmap block");

    unsigned int positionInBitmap = index % 8;
    u_int8_t tmp = 0x01;
    tmp <<= positionInBitmap;
    tmp = ~tmp;

    bitmap &= tmp;

    if (lseek(disk_fd, offset + blockNumber * blockSize + offsetToReadUInt8, SEEK_SET) < 0)
        throw std::ios_base::failure("Cannot seek bitmap block");
    if (write(disk_fd, &bitmap, sizeof(u_int8_t)) < 0)
        throw std::ios_base::failure("Cannot write bitmap block");
}

void MFSClient::ReadFromDisk(int disk_fd, void *buf, size_t size, std::function<void()> functor) {
    if (read(disk_fd, &buf, size) < 0) {
        functor();
        throw std::ios_base::failure("Cannot read virtual disk");
    }
}

void MFSClient::WriteToDisk(int disk_fd, const void *buf, size_t size, std::function<void()> functor) {
    if (write(disk_fd, &buf, size) < 0) {
        functor();
        throw std::ios_base::failure("Cannot write to virtual disk");
    }
}

void MFSClient::LseekOnDisk(int disk_fd, off_t offset, int whence, std::function<void()> functor) {
    if (lseek(disk_fd, offset, whence) < 0) {
        functor();
        throw std::ios_base::failure("Cannot write to virtual disk");
    }
}










