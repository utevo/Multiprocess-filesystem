#include "sync.hpp"

#include <exception>
#include <iostream>
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

std::vector<u_int8_t> GenerateBuf(long mtype, std::vector<u_int8_t> data_) {
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
    buf_ = GenerateBuf(mtype_, data_);
    return buf_.data();
  }

  size_t getMsgsz() { return data_.size(); }

private:
  long mtype_;
  std::vector<u_int8_t> data_;
  std::vector<u_int8_t> buf_;
};

long CalcLockMType(int inode) { return inode * 3 + 0; }

long CalcStateMType(int inode) { return inode * 3 + 1; }

long CalcCondWriterMType(int inode) { return inode * 3 + 2; }

void SendMessage(int msqid, Message message) {
  int result = msgsnd(msqid, message.getMsgp(), message.getMsgsz(), IPC_NOWAIT);
  if (result == -1) {
    throw std::runtime_error("Couldn't send message");
  }
}

void SendEmptyMessage(long mtype) {}

void SendLock(long mtype) { SendEmptyMessage(mtype); }

void SendState(long mtype, bool reader, u_int8_t writers) {}

void SendCondWriter(long mtype) { SendEmptyMessage(mtype); }

void InitLock(long inode) {
  int mtype = CalcLockMType(inode);
  SendLock(mtype);
}

void InitState(long inode) { int mtype = CalcStateMType(inode); }

void InitCondWriter(long inode) { int mtype = CalcCondWriterMType(inode); }

void InitInodesConcurrencyControl(key_t msqid, int inodes) {
  for (int inode = 0; inode < inodes; ++inode) {
    InitLock(inode);
    InitState(inode);
    InitCondWriter(inode);
  }
}

extern void InitSync(const std::string path) {
  key_t msq_key = ftok(path.c_str(), kProjecId);
  if (msq_key == -1) {
    throw std::logic_error("Couldn't generate msq_key");
  }

  int msqid = msgget(msq_key, IPC_CREAT | 0600);
  if (msqid == -1) {
    throw std::logic_error("Couldn't create msqid");
  }
  std::cout << "msqid: " << msqid << std::endl;

  int inodes = 10;
  InitInodesConcurrencyControl(msq_key, inodes);
}

extern void RemoveSync(const std::string path) {
  key_t msq_key = ftok(path.c_str(), kProjecId);
  if (msq_key == -1) {
    throw std::runtime_error("Couldn't generate msq_key");
  }

  int msqid = msgget(msq_key, 0);
  if (msqid == -1) {
    throw std::runtime_error("Couldn't create msqid");
  }

  struct msqid_ds buf;
  int result = msgctl(msqid, IPC_RMID, &buf);
  if (result != 0) {
    throw std::runtime_error("Couldn't delete msq");
  }
}

void SyncClient::ReadLock(u_int32_t inode_idx){};
void SyncClient::ReadUnlock(u_int32_t inode_idx){};