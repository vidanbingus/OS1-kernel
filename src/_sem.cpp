#include "../h/_sem.hpp"

#include "../h/syscall_c.hpp"

_sem::_sem(unsigned initVal) : val((int)initVal), isOpen(true) {}

int _sem::open(_sem** semHandle, unsigned int v) {
    *semHandle = new _sem(v);
    return 0;
}

int _sem::wait()   { return wait_n(1); }
int _sem::signal() { return signal_n(1); }

int _sem::wait_n(unsigned n) {
    if (!isOpen) { return -1; }
    if (n == 0)  { return 0; }

    if (val >= (int)n) {
        val -= (int)n;
        return 0;
    }

    // ako nema dovoljno jedinica blokiraj dok ne pristigne
    TCB::running->semNeed = n;
    TCB::running->setSemClosed(false);
    TCB::running->setBlocked(true);
    blockedThreads.addLast(TCB::running);
    TCB::dispatch();

    if (TCB::running->wasSemClosed()) {
        TCB::running->setSemClosed(false);
        return -1;
    }
    return 0;
}

int _sem::signal_n(unsigned n) {
    if (!isOpen) { return -1; }
    val += (int)n;
    wakeReady();
    return 0;
}

void _sem::wakeReady() {
    // budimo niti redom ali staje cim prva nit u redu trazi vise nego sto ima
    TCB* first;
    while ((first = blockedThreads.peekFirst()) != nullptr && (int)first->semNeed <= val) {
        TCB* tcb = blockedThreads.removeFirst();
        val -= (int)tcb->semNeed;
        tcb->semNeed = 0;
        tcb->setBlocked(false);
        Scheduler::put(tcb);
    }
}

int _sem::close() {
    if (!isOpen) { return -1; }

    isOpen = false;
    while (blockedThreads.peekFirst()) {
        TCB* tcb = blockedThreads.removeFirst();
        tcb->semNeed = 0;
        tcb->setBlocked(false);
        tcb->setSemClosed(true);
        Scheduler::put(tcb);
    }
    return 0;
}

