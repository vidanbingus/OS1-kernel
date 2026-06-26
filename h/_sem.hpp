#ifndef PROJECT_BASE__SEM_H
#define PROJECT_BASE__SEM_H
#include "KernelObject.hpp"
#include "list.hpp"
#include "TCB.hpp"


class _sem : public KernelObject {

public:
    _sem(unsigned initVal);

    int open(_sem** semHandle, unsigned int val);
    int wait();
    int signal();
    int close();

    int wait_n(unsigned n);
    int signal_n(unsigned n);


private:
    void wakeReady();

    int val;
    bool isOpen;

    List<TCB> blockedThreads;
};


#endif //PROJECT_BASE__SEM_H
