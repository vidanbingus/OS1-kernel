#include "../h/TCB.h"
#include "../h/riscv.h"


TCB* TCB::running = nullptr;
TCB* TCB::createThread(Body body) {
    return new TCB(body);
}

void TCB::yield() {
    RiscV::pushRegisters();

    TCB::dispatch();

    RiscV::popRegisters();
}

void TCB::dispatch() {
    TCB* old = running;
    if (!old->isFinished()) { Scheduler::put(old); }
    running = Scheduler::get();

    TCB::contextSwitch(&old->context, &running->context);
}
