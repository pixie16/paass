// Implementation of Lock class which creates a file in a directory
//   with a given name which prevents the process from continuing
#include <algorithm>
#include <iostream>

#include <cstdlib>

#include <unistd.h>

#include "Display.h"
#include "Lock.h"

using namespace Display;

using std::cout;
using std::endl;
using std::find;
using std::list;
using std::string;


list<string> Lock::lockList;
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
    if (lockList.empty()) {
	atexit(RemoveLocks);
    }
    lockList.push_back(fileName);
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
    Remove(fileName);
}

// static method
void Lock::Remove(std::string &name)
{
  LeaderPrint("Removing lock file " + name);

  int errno = remove(name.c_str());
  Display::StatusPrint(errno < 0);
  
  lockList.remove(name);
}

void RemoveLocks(void)
{
    while (!Lock::lockList.empty()) {
	Lock::Remove(Lock::lockList.front());
    }
}

