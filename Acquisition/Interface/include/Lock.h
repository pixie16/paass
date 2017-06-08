// Lock class so two processes don't try to access Pixie at once
// Ingratiously borrowed from CV
// David Miller, March 2010

#ifndef __LOCK_H_
#define __LOCK_H_

#include <list>
#include <string>

#include <cstdio>

class Lock {
public:
    Lock(std::string name);

    ~Lock(void);

    void Remove(void);

    static void Remove(std::string &name);

    static std::list<std::string> lockList;
private:
    static const std::string lockDirectory;
    FILE *lockFile;
    std::string fileName;
};

void RemoveLocks(void);

#endif
