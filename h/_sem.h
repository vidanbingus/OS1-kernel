#ifndef PROJECT_BASE__SEM_H
#define PROJECT_BASE__SEM_H
#include "list.h"
#include "TCB.h"


class _sem {

public:
    _sem(unsigned initVal);

    int open(_sem** semHandle, unsigned int val);
    int wait();
    int signal();
    int close();

private:
    int val;
    bool isOpen;

    List<TCB> blockedThreads;
};


#endif //PROJECT_BASE__SEM_H
