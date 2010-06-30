// Implementation of Lock class which creates a file in a directory
//   with a given name which prevents the process from continuing

#include <iostream>

#include <cstdlib>

#include <unistd.h>

#include "Display.h"
#include "Lock.h"

using namespace Display;

using std::cout;
using std::endl;
using std::string;

const string Lock::lockDirectory = "/var/lock";

Lock::Lock(string name)
{
  fileName = lockDirectory + '/' + name;
  LeaderPrint("Creating lock file");
  lockFile = fopen(fileName.c_str(), "r");
  if (lockFile == NULL) { // the file doesn't exist, the device is free
    lockFile = fopen(fileName.c_str(), "w");
    if (lockFile == NULL) {
      cout << ErrorStr();
      perror("fopen");
      exit(EXIT_FAILURE);
    }
    cout << InfoStr(fileName) << endl;
    // store the pid of the locking process in the file
    fprintf(lockFile, "%10d", getpid());
    fclose(lockFile);
  } else {
    pid_t pid;

    fscanf(lockFile, "%10d", &pid);
    cout << ErrorStr() << endl;
    cout << "  Lockfile " << InfoStr(fileName) << " already created by process " << pid << endl;
    fclose(lockFile);
    exit(EXIT_FAILURE);
  }
}

Lock::~Lock()
{
  Remove();
}

void Lock::Remove(void)
{
  LeaderPrint("Removing lock file");

  int errno = remove(fileName.c_str());
  if (errno < 0)
    cout << ErrorStr() << endl;
  else
    cout << OkayStr() << endl;
}
