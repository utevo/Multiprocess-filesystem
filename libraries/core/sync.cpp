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

long CalcInodeLockMType(int inode_idx) { return (inode_idx + 2) * 3 + 0; }

long CalcInodeStateMType(int inode_idx) { return (inode_idx + 2) * 3 + 1; }

long CalcInodeCondWriterMType(int inode_idx) { return (inode_idx + 2) * 3 + 2; }

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

void SendInodeLock(int msqid, int inode_idx) {
  long mtype = CalcInodeLockMType(inode_idx);
  SendLock(msqid, mtype);
}

struct State {
  u_int8_t readers;
  bool writer;
};

void SendState(int msqid, long mtype, State state) {
  Message message = Message(mtype, {state.readers, (u_int8_t)state.writer});
  SendMessage(msqid, message);
}

void SendInodeState(int msqid, u_int32_t inode_idx, State state) {
  long mtype = CalcInodeStateMType(inode_idx);
  SendState(msqid, mtype, state);
}

void SendCondWriter(int msqid, long mtype) { SendEmptyMessage(msqid, mtype); }

void RecieveLock(int msqid, long mtype) { RecieveEmptyMessage(msqid, mtype); }

void RecieveInodeLock(int msqid, int inode_idx) {
  long mtype = CalcInodeLockMType(inode_idx);
  RecieveLock(msqid, mtype);
}

State RecieveState(int msqid, long mtype) {
  const size_t buf_size = sizeof(long) + 2;
  char buf[buf_size];

  int result = msgrcv(msqid, &buf, buf_size, mtype, 0);
  if (result == -1) {
    std::runtime_error("Couldn't recieve message");
  }
  int readers_idx = sizeof(long);
  u_int8_t readers = buf[readers_idx];
  int writer_idx = sizeof(long) + sizeof(bool);
  bool writer = buf[writer_idx];

  State state = {readers, writer};
  return state;
}

State RecieveInodeState(int msqid, u_int32_t inode_idx) {
  long mtype = CalcInodeStateMType(inode_idx);
  return RecieveState(msqid, mtype);
}

void RecieveCondWriter(int msqid, long mtype) {
  RecieveEmptyMessage(msqid, mtype);
}

void InitInodesSync(key_t msqid, int inodes) {
  for (int inode_idx = 0; inode_idx < inodes; ++inode_idx) {
    long lock_mtype = CalcInodeLockMType(inode_idx);
    SendLock(msqid, lock_mtype);

    long state_mtype = CalcInodeStateMType(inode_idx);
    State state = {0, 0};
    SendState(msqid, state_mtype, state);

    long cond_writer_mtype = CalcInodeCondWriterMType(inode_idx);
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

void AddReaderToInodeState(int msqid, u_int32_t inode_idx) {
  State state_before = RecieveInodeState(msqid, inode_idx);

  if (state_before.readers == 255) {
    SendInodeState(msqid, inode_idx, state_before);
    throw std::runtime_error("Too many readers in inode: " +
                             std::to_string(inode_idx));
  }

  State state_after = {state_before.readers + 1, state_before.writer};
  SendInodeState(msqid, inode_idx, state_after);
}
void RemoveReaderFromInodeState(int msqid, u_int32_t inode_idx) {
  State state_before = RecieveInodeState(msqid, inode_idx);

  if (state_before.readers == 0) {
    SendInodeState(msqid, inode_idx, state_before);
    throw std::runtime_error("Any readers in inode: " +
                             std::to_string(inode_idx));
  }

  State state_after = {state_before.readers - 1, state_before.writer};
  SendInodeState(msqid, inode_idx, state_after);
}

void SyncClient::ReadLock(u_int32_t inode_idx) {
  RecieveInodeLock(msqid_, inode_idx);

  AddReaderToInodeState(msqid_, inode_idx); // ToDo: Handle throw

  SendInodeLock(msqid_, inode_idx);
};
void SyncClient::ReadUnlock(u_int32_t inode_idx) {
  RemoveReaderFromInodeState(msqid_, inode_idx);

  // ToDo: Write more code
}

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
