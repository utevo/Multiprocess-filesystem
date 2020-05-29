#include "sync.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <vector>

const int kProjecId = 0x1a4;

std::vector<u_int8_t> SerializeLong(long value) {
  std::vector<u_int8_t> result;
  for (int i = 0; i < sizeof(long); i++) {
    result.push_back((value >> (8 * i)) & 0XFF);
  }

  return result;
}

std::vector<u_int8_t> GenerateMessageBuf(long mtype,
                                         std::vector<u_int8_t> data_) {
  std::vector<u_int8_t> result;

  std::vector<u_int8_t> serialised_mtype = SerializeLong(mtype);
  result.insert(result.end(), serialised_mtype.begin(), serialised_mtype.end());

  result.insert(result.end(), data_.begin(), data_.end());

  return result;
}

class Message {
public:
  Message(long mtype, std::vector<u_int8_t> data)
      : mtype_(mtype), data_(data) {}

  const void *getMsgp() {
    buf_ = GenerateMessageBuf(mtype_, data_);
    return buf_.data();
  }

  size_t getMsgsz() { return data_.size(); }

private:
  long mtype_;
  std::vector<u_int8_t> data_;
  std::vector<u_int8_t> buf_;
};

long CalcInodeLockMType(int inode) { return (inode + 2) * 3 + 0; }

long CalcInodeStateMType(int inode) { return (inode + 2) * 3 + 1; }

long CalcInodeCondWriterMType(int inode) { return (inode + 2) * 3 + 2; }

long CalcAllocationBitmapLockMType() { return 1; }

long CalcInodesBitmapLockMType() { return 2; }

void SendMessage(int msqid, Message message) {
  int result = msgsnd(msqid, message.getMsgp(), message.getMsgsz(), IPC_NOWAIT);
  if (result == -1) {
    throw std::runtime_error("Couldn't send message. Errno: " +
                             std::to_string(errno));
  }
}

void SendEmptyMessage(int msqid, long mtype) {
  Message message = Message(mtype, {});
  SendMessage(msqid, message);
}

void RecieveEmptyMessage(int msqid, long mtype) {
  const size_t buf_size = sizeof(long) + 0;
  char buf[buf_size];

  int result = msgrcv(msqid, &buf, buf_size, mtype, 0);
  if (result == -1) {
    std::runtime_error("Couldn't recieve message");
  }
}

void SendLock(int msqid, long mtype) { SendEmptyMessage(msqid, mtype); }

struct State {
  bool reader;
  u_int8_t writers;
};

void SendState(int msqid, long mtype, State state) {
  Message message = Message(mtype, {(u_int8_t)state.reader, state.writers});
  SendMessage(msqid, message);
}

void SendCondWriter(int msqid, long mtype) { SendEmptyMessage(msqid, mtype); }

void RecieveLock(int msqid, long mtype) { RecieveEmptyMessage(msqid, mtype); }

State RecieveState(int msqid, long mtype) {
  const size_t buf_size = sizeof(long) + 2;
  char buf[buf_size];

  int result = msgrcv(msqid, &buf, buf_size, mtype, 0);
  if (result == -1) {
    std::runtime_error("Couldn't recieve message");
  }
  int reader_idx = sizeof(long);
  bool reader = buf[reader_idx];
  int writers_idx = sizeof(long) + sizeof(bool);
  char writers = buf[writers_idx];

  State state = {reader, writers};
  return state;
}

void RecieveCondWriter(int msqid, long mtype) {
  RecieveEmptyMessage(msqid, mtype);
}

void InitInodesSync(key_t msqid, int inodes) {
  for (int inode = 0; inode < inodes; ++inode) {
    long lock_mtype = CalcInodeLockMType(inode);
    SendLock(msqid, lock_mtype);

    long state_mtype = CalcInodeStateMType(inode);
    State state = {0, 0};
    SendState(msqid, state_mtype, state);

    long cond_writer_mtype = CalcInodeCondWriterMType(inode);
    SendCondWriter(msqid, cond_writer_mtype);
  }
}

void InitAllocationBitmapSync(key_t msqid) {
  long allocation_bitmap_mtype = CalcAllocationBitmapLockMType();
  SendLock(msqid, allocation_bitmap_mtype);
}

void InitInodesBitmapSync(key_t msqid) {
  long inodes_bitmap_mtype = CalcInodesBitmapLockMType();
  SendLock(msqid, inodes_bitmap_mtype);
}

int CreateMsq(const std::string path) {
  key_t msq_key = ftok(path.c_str(), kProjecId);
  if (msq_key == -1) {
    throw std::logic_error("Couldn't generate msq_key");
  }

  int msqid = msgget(msq_key, IPC_CREAT | 0600);
  if (msqid == -1) {
    throw std::logic_error("Couldn't create msqid");
  }

  return msqid;
}

int ReadMsq(const std::string path) {
  key_t msq_key = ftok(path.c_str(), kProjecId);
  if (msq_key == -1) {
    throw std::runtime_error("Couldn't generate msq_key");
  }

  int msqid = msgget(msq_key, 0);
  if (msqid == -1) {
    throw std::runtime_error("Couldn't read msqid");
  }

  return msqid;
}

extern void InitSync(const std::string path) {
  int msqid = CreateMsq(path);

  InitAllocationBitmapSync(msqid);
  InitInodesBitmapSync(msqid);

  int inodes = 13; // ToDo: need read number of inodes from fs
  InitInodesSync(msqid, inodes);

}

extern void RemoveSync(const std::string path) {
  int msqid = ReadMsq(path);

  struct msqid_ds buf;
  int result = msgctl(msqid, IPC_RMID, &buf);
  if (result != 0) {
    throw std::runtime_error("Couldn't delete msq");
  }
}

void SyncClient::Init(const std::string path) { msqid_ = ReadMsq(path); }

void SyncClient::ReadLock(u_int32_t inode_idx){};
void SyncClient::ReadUnlock(u_int32_t inode_idx){};

void SyncClient::AllocationBitmapLock() {
  long mtype = CalcAllocationBitmapLockMType();
  RecieveLock(msqid_, mtype);
}
void SyncClient::AllocationBitmapUnlock() {
  long mtype = CalcAllocationBitmapLockMType();
  SendLock(msqid_, mtype);
}

void SyncClient::InodesBitmapLock() {
  long mtype = CalcInodesBitmapLockMType();
  RecieveLock(msqid_, mtype);
}
void SyncClient::InodesBitmapUnlock() {
  long mtype = CalcInodesBitmapLockMType();
  SendLock(msqid_, mtype);
}
