#include "sync.hpp"

#include <exception>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <vector>
#include <string>


const int kProjecId = 0x1a4;

std::vector<u_int8_t> SerializeLong(long value) {
  std::vector<u_int8_t> result;
  for (int i = 0; i < sizeof(long); i++) {
    result.push_back((value >> (8 * i)) & 0XFF);
  }

  return result;
}

std::vector<u_int8_t> GenerateMessageBuf(long mtype, std::vector<u_int8_t> data_) {
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

long CalcLockMType(int inode) { return (inode + 1) * 3 + 0; }

long CalcStateMType(int inode) { return (inode + 1) * 3 + 1; }

long CalcCondWriterMType(int inode) { return (inode + 1) * 3 + 2; }

void SendMessage(int msqid, Message message) {
  int result = msgsnd(msqid, message.getMsgp(), message.getMsgsz(), IPC_NOWAIT);
  if (result == -1) {
    throw std::runtime_error("Couldn't send message. Errno: " + std::to_string(errno));
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

void SendState(int msqid, long mtype, bool reader, u_int8_t writers) {
  Message message = Message(mtype, {(u_int8_t)reader, writers});
  SendMessage(msqid, message);
}

void SendCondWriter(int msqid, long mtype) { SendEmptyMessage(msqid, mtype); }

void RecieveLock(int msqid, long mtype) { RecieveEmptyMessage(msqid, mtype); }

struct State {
  bool reader;
  char writers;
};

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
    long lock_mtype = CalcLockMType(inode);
    std::cout << msqid << std::endl;
    SendLock(msqid, lock_mtype);
    
    long state_mtype = CalcStateMType(inode);
    SendState(msqid, state_mtype, 0, 0);

    long cond_writer_mtype = CalcCondWriterMType(inode);
    SendCondWriter(msqid, cond_writer_mtype);
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

  int inodes = 13;
  InitInodesSync(msqid, inodes);
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