/// @brief Implementation of Lock class which creates a file in a directory with a given name which prevents the
/// process from continuing
///@authors D. Miller, C. R. Thornsberry, K. Smith

#include <algorithm>
#include <iostream>

#include <cstdlib>

#include <unistd.h>

#include "Display.h"
#include "Lock.h"

#ifndef LOCK_DIRECTORY
#define LOCK_DIRECTORY "/tmp"
#endif

using namespace Display;

using std::cout;
using std::endl;
using std::find;
using std::list;
using std::string;

list<string> Lock::lockList;

Lock::Lock(string name) {
    const string lockDirectory = LOCK_DIRECTORY;
    fileName = lockDirectory + "/" + name;
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
        if (Lock::lockList.empty()) {
            atexit(RemoveLocks);
        }
        Lock::lockList.push_back(fileName);
    } else {
        pid_t pid;

        fscanf(lockFile, "%10d", &pid);
        cout << ErrorStr() << endl;
        cout << "  Lockfile " << InfoStr(fileName)
             << " already created by process " << pid << endl;
        fclose(lockFile);
        exit(EXIT_FAILURE);
    }
}

Lock::~Lock() {
    Remove();
}

void Lock::Remove(void) {
    Remove(fileName);
}

// static method
void Lock::Remove(std::string &name) {
    LeaderPrint("Removing lock file " + name);

    Display::StatusPrint(remove(name.c_str()) < 0);

    Lock::lockList.remove(name);
}

void RemoveLocks(void) {
    while (!Lock::lockList.empty()) {
        Lock::Remove(Lock::lockList.front());
    }
}

