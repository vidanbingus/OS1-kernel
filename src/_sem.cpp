#include "../h/_sem.h"

#include "../h/syscall_c.h"

_sem::_sem(unsigned initVal) : val(initVal), isOpen(true) {}

int _sem::open(_sem** semHandle, unsigned int val) {
    *semHandle = new _sem(val);
    (*semHandle)->isOpen = true;
    return 0;
}
int _sem::wait() {
    if (!isOpen) {return -1;}
    if (--val < 0) {
        TCB::running->setBlocked(true);
        blockedThreads.addLast(TCB::running);
        TCB::dispatch();
    }
    return 0;
}

int _sem::signal() {
    if (!isOpen) {return -1;}
    if (++val <= 0) {
        TCB* tcb = blockedThreads.removeFirst();
        tcb->setBlocked(false);
        Scheduler::put(tcb);
    }
    return 0;
}

int _sem::close() {
    if (!isOpen) {return -1;}

    isOpen = false;
    while (blockedThreads.peekFirst()) {
        TCB* tcb = blockedThreads.removeFirst();
        tcb->setBlocked(false);
        Scheduler::put(tcb);
    }
    return 0;
}
