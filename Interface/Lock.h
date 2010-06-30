// Lock class so two processes don't try to access Pixie at once
// Ingratiously borrowed from CV
// David Miller, March 2010

#ifndef __LOCK_H_
#define __LOCK_H_

#include <string>

#include <cstdio>

class Lock {
 public:
  Lock(std::string name);
  ~Lock(void);
  void Remove(void);
 private:
  static const std::string lockDirectory;
  FILE *lockFile;
  std::string fileName; 
};

#endif
