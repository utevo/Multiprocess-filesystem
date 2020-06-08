#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <cmath>

#include "lib.hpp"

#include "../core/sync.hpp"
#include "../core/utils.hpp"

extern int test_client_lib(int x) { return 1 * x; }


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

    makeRoot();
}

void MFSClient::makeRoot() {
    getAndTakeUpFirstFreeInode(); //should take up inode 0
    getAndTakeUpFirstFreeBlock(); //should take up block 0

    u_int32_t rootInode = getAndTakeUpFirstFreeInode(); //should take up inode 1
    u_int32_t rootBlock = getAndTakeUpFirstFreeBlock(); //should take up inode 1

    Inode inode;
    inode.valid = 1;
    inode.type = DIRECTORY;
    inode.size = blockSize;
    inode.direct_idxs[0] = rootBlock;
    sync_client.WriteLock(rootInode);
    int disk = openAndSeek(inodesOffset + rootInode * inodeSize);

    writeToDisk(disk, &inode, sizeof(Inode),
                [&]() { sync_client.WriteUnlock(rootInode); });
    sync_client.WriteUnlock(rootInode);
    clearBlock(1);
    close(disk);

    addInodeToDirectory(rootInode, rootInode, ".");
    addInodeToDirectory(rootInode, rootInode, "..");
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

        if (CheckIfInodeExists(filename, directoryInodeIndex))
            return -1;

        addInodeToDirectory(directoryInodeIndex, inodeIndex, filename);

        Inode inode;
        inode.valid = 1;
        inode.type = NORMAL_FILE;
        inode.size = 0;

        sync_client.WriteLock(inodeIndex);
        lseekOnDisk(disk, inodesOffset + inodeIndex * inodeSize, SEEK_SET,
                    [&]() { sync_client.WriteUnlock(inodeIndex); });
        writeToDisk(disk, &inode, sizeof(Inode),
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
    try {
        OpenFile open_file = open_files.at(fd);
        u_int32_t inode_idx = open_file.inode_idx;
        sync_client.ReadLock(inode_idx);

        // TODO

        sync_client.ReadUnlock(inode_idx);
    }
    catch (std::exception &) {
        return -1;
    }
}

int MFSClient::mfs_write(int fd, char *buf, int len) {
    try {
        OpenFile open_file = open_files.at(fd);
        u_int32_t inode_idx = open_file.inode_idx;
        u_int32_t offset = open_file.offset;
        if (open_file.status == FileStatus::RDONLY)
            return -1;

        sync_client.WriteLock(inode_idx);
        Inode inode = getInodeByIndex(inode_idx);

        u_int32_t numberOfNewBlocks = myCeil((offset + len) - inode.size,blockSize);
        u_int32_t numberOfBlocksToWrite = blocksToWrite(offset, len);
        u_int32_t actualBlock = offset / blockSize;


        int newBlock = 0;
        if (offset + len > inode.size)
            newBlock = getAndTakeUpFirstFreeBlock();


        u_int32_t offsetInBlock = offset % blockSize;
        sync_client.WriteUnlock(inode_idx);
        return 0;
    } catch (std::exception &) {
        return -1;
    }
}

u_int32_t MFSClient::blocksToWrite(const u_int32_t& fileOffset, const u_int32_t& length) const {
    u_int32_t bytesToWriteInCurrentBlock = blockSize - (fileOffset % blockSize);
    if(length < bytesToWriteInCurrentBlock)
        return 1;
    int bytesToWriteInNextBlocks = length - bytesToWriteInCurrentBlock;
    return 1 + myCeil(bytesToWriteInNextBlocks, blockSize);
}

uint32_t MFSClient::getBlockInFileByNumber(u_int32_t inode_idx, const Inode &inode, u_int32_t blockNumberInFile) {
    int blockIndex;
    u_int32_t numberOfIndirect = blockSize / sizeof(u_int32_t);
    if (blockNumberInFile < 4) {
        blockIndex = inode.direct_idxs[blockNumberInFile];
        if (blockIndex == 0) {
            return -1;
        }
    } else if (blockNumberInFile < numberOfIndirect + 4) {
        int disk_fd = openAndSeek();
        u_int32_t blockNumberInIndirect = blockNumberInFile - 4;
        return getBlockInFileByNumberIndirect(disk_fd, inode_idx, inode, blockNumberInIndirect);
    } else {
        return -1;
    }
    return blockIndex;
}

uint32_t MFSClient::getBlockInFileByNumberIndirect(int disk_fd, u_int32_t inode_idx,
                                                   const Inode &inode,
                                                   u_int32_t blockNumberInFile) {
    lseekOnDisk(disk_fd, blocksOffset + inode.indirect_idx * blockSize + (blockNumberInFile - 1) * sizeof(u_int32_t),
                SEEK_SET, [&]() {});

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
    if (split(name, '/').size() == 0)
        throw std::invalid_argument("Cannot make directory with empty name");

    int disk;

    try {
        //get newName 
        std::string newName = Handler::getFileName(name);

        //get parent inode:
        u_int32_t parentInode = getInode(Handler::getDirectory(name));
        //take up and get new dir inode and first data block:
        u_int32_t inodeIndex = getAndTakeUpFirstFreeInode();
        u_int32_t blockIndex = getAndTakeUpFirstFreeBlock();
        //set new inode object:
        disk = openAndSeek(inodesOffset + inodeIndex * inodeSize);

        if (CheckIfInodeExists(newName, parentInode))
            return -1;

        Inode inode;
        inode.valid = 1;
        inode.type = DIRECTORY;
        inode.size = blockSize; //TODO determine size
        inode.direct_idxs[0] = blockIndex;

        //write the inode:
        sync_client.WriteLock(inodeIndex);

        writeToDisk(disk, &inode, sizeof(Inode),
                    [&]() { sync_client.WriteUnlock(inodeIndex); });

        sync_client.WriteUnlock(inodeIndex);

        //add parent references:
        addInodeToDirectory(parentInode, inodeIndex, newName);
        //add newdir references:
        addInodeToDirectory(inodeIndex, inodeIndex, ".");
        addInodeToDirectory(inodeIndex, parentInode, "..");
    }
    catch (const std::exception &) {
        close(disk);
        return -1;
    }

    return 0;
}

std::vector<std::pair<uint32_t, std::string>> MFSClient::mfs_ls(char *name) {
    std::vector<std::pair<uint32_t, std::string>> files;
    u_int32_t directoryIndex = getInode(name);
    int disk = openAndSeek(inodesOffset + directoryIndex * inodeSize);
    Inode directoryInode;
    sync_client.ReadLock(directoryIndex);
    readFromDisk(disk, &directoryInode, sizeof(Inode), [&]() { sync_client.ReadUnlock(directoryIndex); });
    for (auto blockIndex : directoryInode.direct_idxs) {
        lseekOnDisk(disk, blocksOffset + blockIndex * blockSize, SEEK_SET,
                    [&]() { sync_client.ReadUnlock(directoryIndex); });
        for (int i = 0; i < 128; ++i) {
            u_int32_t fileInodeIndex;
            readFromDisk(disk, &fileInodeIndex, sizeof(u_int32_t), [&]() { sync_client.ReadUnlock(directoryIndex); });
            char buff[28];
            readFromDisk(disk, buff, 28, [&]() { sync_client.ReadUnlock(directoryIndex); });
            if (fileInodeIndex != 0) {
                files.push_back(std::make_pair(fileInodeIndex, buff));
            }
        }
    }
    return files;
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


Inode &MFSClient::getInodeByIndex(u_int32_t index) {
    int disk = openAndSeek(inodesOffset + index * inodeSize);
    Inode inode;
    readFromDisk(disk, &inode, sizeof(Inode), [&]() { close(disk); });
    return inode;
}

bool MFSClient::CheckIfInodeExists(const std::string &filename, const u_int32_t &directoryInode) {
    int disk_fd = openAndSeek();
    try {
        u_int32_t index = getInodeFromDirectoryByName(disk_fd, filename, directoryInode);
    }
    catch (std::exception &) {
        close(disk_fd);
        return false;
    }
    close(disk_fd);
    return true;
}


u_int32_t MFSClient::getInode(std::string path) {
    int disk = openAndSeek(inodesOffset);
    std::vector<std::string> directories = split(path, '/');
    u_int32_t currentInode = 1;
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

u_int32_t MFSClient::getInodeFromDirectoryByName(const int &disk_fd,
                                                 const std::string &filename,
                                                 const u_int32_t &directoryInode) {
    sync_client.ReadLock(directoryInode);
    lseekOnDisk(disk_fd, inodesOffset + directoryInode * inodeSize, SEEK_SET,
                [&]() { sync_client.ReadUnlock(directoryInode); });
    Inode inode;
    readFromDisk(disk_fd, &inode, sizeof(Inode), [&]() { sync_client.ReadUnlock(directoryInode); });

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
        lseekOnDisk(disk_fd, blocksOffset + block * blockSize, SEEK_SET,
                    [&]() { sync_client.ReadUnlock(directoryInode); });
        //foreach rows in data block, row is inode and name
        do {
            readFromDisk(disk_fd, &index, sizeof(u_int32_t), [&]() { sync_client.ReadUnlock(directoryInode); });
            readFromDisk(disk_fd, buffer, NAME_SIZE, [&]() { sync_client.ReadUnlock(directoryInode); });
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
        close(disk_fd);
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
        close(disk_fd);
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
    readFromDisk(disk_fd, &directoryInode, sizeof(Inode),
                 [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
    for (const auto &block : directoryInode.direct_idxs) {
        lseekOnDisk(disk_fd, blocksOffset + block * blockSize, SEEK_SET,
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
        //foreach rows in data block, row is inode and name
        int row = 0;
        while (row < 128) {
            u_int32_t index = 0;
            readFromDisk(disk_fd, &index, sizeof(u_int32_t),
                         [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
            if (index == inodeToChange) {
                lseekOnDisk(disk_fd, blocksOffset + block * blockSize + row * 32, SEEK_SET,
                            [&]() { sync_client.WriteUnlock(directoryInodeIndex); });

                writeInodeAndName(disk_fd, directoryInodeIndex, newInode, name);
                sync_client.WriteUnlock(directoryInodeIndex);
                return;
            }
            lseekOnDisk(disk_fd, blocksOffset + block * blockSize + (row + 1) * 32, SEEK_SET,
                        [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
            row++;
        }
    }
    sync_client.WriteUnlock(directoryInodeIndex);
    throw std::domain_error("Cannot change directory");
}

void MFSClient::writeInodeAndName(
        int disk_fd, const u_int32_t &directoryInodeIndex,
        const u_int32_t &newInodeIndex, const std::string &name) {
    if (newInodeIndex != 0) {
        writeToDisk(disk_fd, &newInodeIndex, sizeof(u_int32_t),
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
        writeToDisk(disk_fd, name.c_str(), name.length(),
                    [&]() { sync_client.WriteUnlock(directoryInodeIndex); });
    } else {
        char buff[32];
        for (int i = 0; i < 32; ++i)
            buff[i] = '\0';
        writeToDisk(disk_fd, buff, 32,
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
    sync_client.InodesBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    int inodeNumber;
    try {
        inodeNumber = getFirstFreeBitmapIndex(disk, inodesBitmapOffset, inodesBitmap, inodes);
    }
    catch (std::exception &e) {
        close(disk);
        sync_client.InodesBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.InodesBitmapUnlock();
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
    sync_client.InodesBitmapLock();
    int disk = openAndSeek(inodesBitmapOffset);
    try {
        freeBitmapIndex(disk, inodesBitmapOffset, index);
        clearInode(index);
    }
    catch (std::exception &e) {
        close(disk);
        sync_client.InodesBitmapUnlock();
        throw e;
    }
    close(disk);
    sync_client.InodesBitmapUnlock();
}

void MFSClient::clearInode(u_int32_t index) {
    int disk = openAndSeek(inodesOffset + inodeSize * index);
    sync_client.WriteLock(index);
    Inode inode;
    readFromDisk(disk, &inode, sizeof(Inode), [&]() {
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
        lseekOnDisk(disk, blocksOffset + inode.double_indirect_idx, SEEK_SET,
                    [&]() {
                        sync_client.WriteUnlock(index);
                        close(disk);
                    });
        std::vector<u_int32_t> indirectBlockIndexes(blockSize / sizeof(u_int32_t));
        readFromDisk(disk, indirectBlockIndexes.data(), blockSize, [&]() {
                         sync_client.WriteUnlock(index);
                         close(disk);
                     }
        );
        for (auto &block : indirectBlockIndexes) {
            if (block == 0)
                break;
            clearIndirectBlocks(disk, index);
        }
        freeBlock(inode.double_indirect_idx);
        freeBlock(index);
    }
    //write cleared inode
    lseekOnDisk(disk, inodesOffset + inodeSize * index, SEEK_SET,
                [&]() {
                    sync_client.WriteUnlock(index);
                    close(disk);
                });
    writeToDisk(disk, &inode, sizeof(Inode), [&]() {
        sync_client.WriteUnlock(index);
        close(disk);
    });
    sync_client.WriteUnlock(index);
    close(disk);
}

void MFSClient::clearIndirectBlocks(int disk, int index) {
    lseekOnDisk(disk, blocksOffset + index, SEEK_SET,
                [&]() {
                    sync_client.WriteUnlock(index);
                    close(disk);
                });
    std::vector<u_int32_t> blockIndexes(blockSize / sizeof(u_int32_t));
    readFromDisk(disk, blockIndexes.data(), blockSize, [&]() {
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
    writeToDisk(disk, buff, sizeof(blockSize), [&]() { close(disk); });
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

void MFSClient::readFromDisk(int disk_fd, void *buf, size_t size, std::function<void()> functor) {
    if (read(disk_fd, buf, size) < 0) {
        functor();
        throw std::ios_base::failure("Cannot read virtual disk");
    }
}

void MFSClient::writeToDisk(int disk_fd, const void *buf, size_t size, std::function<void()> functor) {
    if (write(disk_fd, buf, size) < 0) {
        functor();
        throw std::ios_base::failure("Cannot write to virtual disk");
    }
}

void MFSClient::lseekOnDisk(int disk_fd, off_t offset, int whence, std::function<void()> functor) {
    if (lseek(disk_fd, offset, whence) < 0) {
        functor();
        throw std::ios_base::failure("Cannot write to virtual disk");
    }
}









