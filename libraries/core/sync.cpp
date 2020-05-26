#include "sync.hpp"

#include <exception>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

const int kProjecId = 2;

void SendMessage(int lock_id){

};

long CalcLockMType(int inode) { return inode * 3 + 0; }

long CalcStateMType(int inode) { return inode * 3 + 1; }

long CalcCondWriterMType(int inode) { return inode * 3 + 2; }

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

  int inodes = 10;
  InitInodesConcurrencyControl(msq_key, inodes);
}

extern void RemoveSync(std::string path) { // ToDO: make to work
  key_t msq_key = ftok(path.c_str(), kProjecId);
  if (msq_key == -1) {
    throw std::logic_error("Couldn't generate msq_key");
  }

  int msqid = msgget(msq_key, IPC_CREAT | 0600);
  if (msqid == -1) {
    throw std::logic_error("Couldn't create msqid");
  }

  struct msqid_ds buf;
  int result = msgctl(msqid, IPC_RMID, &buf);
  std::cout << "msgctl: " << result << std::endl;
  std::cout << "errno: " << errno << std::endl;
}

void SyncClient::ReadLock(u_int32_t inode_idx){};
void SyncClient::ReadUnlock(u_int32_t inode_idx){};