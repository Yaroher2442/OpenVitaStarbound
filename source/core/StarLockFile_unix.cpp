#include "StarLockFile.hpp"
#include "StarThread.hpp"
#include "StarTime.hpp"

#include "psp2/io/fcntl.h"
#include <psp2/kernel/threadmgr.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

// #define LOCK_SH 0x01 /* shared file lock */
// #define LOCK_EX 0x02 /* exclusive file lock */
// #define LOCK_NB 0x04 /* don't block when locking */
// #define LOCK_UN 0x08 /* unlock file */


namespace Star {

// Mutex for file locking simulation
SceUID lockMutex = -1;

// Initialize the mutex
void initLockMutex() {
  lockMutex = sceKernelCreateMutex("LockMutex", SCE_KERNEL_MUTEX_ATTR_RECURSIVE, 0, NULL);
  if (lockMutex < 0) {
    throw StarException("Failed to create mutex: " + std::to_string(lockMutex));
  }
}

// Lock and unlock functions using mutex
void lockFileMutex() {
  int ret = sceKernelLockMutex(lockMutex, 1, NULL);
  if (ret < 0) {
    throw StarException("Failed to lock mutex: " + std::to_string(ret));
  }
}

void unlockFileMutex() {
  int ret = sceKernelUnlockMutex(lockMutex, 1);
  if (ret < 0) {
    throw StarException("Failed to unlock mutex: " + std::to_string(ret));
  }
}


int64_t const LockFile::MaximumSleepMillis;

Maybe<LockFile> LockFile::acquireLock(String const& filename, int64_t lockTimeout) {
  LockFile lock(std::move(filename));
  if (lock.lock(lockTimeout))
    return lock;
  return {};
}

LockFile::LockFile(String const& filename) : m_filename(std::move(filename)) {
  initLockMutex();  // Initialize mutex
}

LockFile::LockFile(LockFile&& lockFile) {
  operator=(std::move(lockFile));
}

LockFile::~LockFile() {
  unlock();
}

LockFile& LockFile::operator=(LockFile&& lockFile) {
  m_filename = std::move(lockFile.m_filename);
  m_handle = std::move(lockFile.m_handle);

  return *this;
}


bool LockFile::lock(int64_t timeout) {
  if (timeout < 0) {
    lockFileMutex();
    return true;
  } else if (timeout == 0) {
    // Non-blocking lock (simulated with mutex)
    int ret = sceKernelTryLockMutex(lockMutex, 1);
    if (ret == 0) {
      return true;
    }
    return false;
  } else {
    uint64_t startTime = sceKernelGetSystemTimeWide();  // Use sceKernelGetSystemTimeWide for timing
    while (true) {
      int ret = sceKernelTryLockMutex(lockMutex, 1);
      if (ret == 0) {
        return true;
      }

      uint64_t currentTime = sceKernelGetSystemTimeWide();
      if (currentTime - startTime > timeout) {
        return false;
      }

      sceKernelDelayThread(1000000);  // Sleep for 1 second
    }
  }

  // auto doFLock = [](String const& filename, bool block) -> shared_ptr<int> {
  //   int fd = open(filename.utf8Ptr(), O_RDONLY | O_CREAT, 0644);
  //   if (fd < 0)
  //     throw StarException(strf("Could not open lock file {}, {}\n", filename, strerror(errno)));
  //
  //   int ret;
  //   if (block)
  //     ret = flock(fd, LOCK_EX);
  //   else
  //     ret = flock(fd, LOCK_EX | LOCK_NB);
  //
  //   if (ret != 0) {
  //     close(fd);
  //     if (errno != EWOULDBLOCK)
  //       throw StarException(strf("Could not lock file {}, {}\n", filename, strerror(errno)));
  //     return {};
  //   }
  //
  //   return make_shared<int>(fd);
  // };
  //
  // if (timeout < 0) {
  //   m_handle = doFLock(m_filename, true);
  //   return true;
  // } else if (timeout == 0) {
  //   m_handle = doFLock(m_filename, false);
  //   return (bool)m_handle;
  // } else {
  //   int64_t startTime = Time::monotonicMilliseconds();
  //   while (true) {
  //     m_handle = doFLock(m_filename, false);
  //     if (m_handle)
  //       return true;
  //
  //     if (Time::monotonicMilliseconds() - startTime > timeout)
  //       return false;
  //
  //     Thread::sleep(min(timeout / 4, MaximumSleepMillis));
  //   }
  // }
}

void LockFile::unlock() {
  if (m_handle) {
    int fd = *(int*)m_handle.get();
    unlink(m_filename.utf8Ptr());
    close(fd);
    m_handle.reset();
  }
}

bool LockFile::isLocked() const {
  return (bool)m_handle;
}

}// namespace Star