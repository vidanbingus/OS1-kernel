#include "../h/TCB.h"
#include "../h/riscv.h"


TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;

TCB* TCB::createThread(Body body) {
    return new TCB(body,TIME_SLICE);
}

void TCB::yield() {
    __asm__ volatile("ecall");
}

void TCB::dispatch() {
    TCB* old = running;
    if (!old->isFinished()) { Scheduler::put(old); }
    running = Scheduler::get();

    TCB::contextSwitch(&old->context, &running->context);
}

void TCB::threadWrapper() {
    RiscV::extractSppSpie();
    running->body();
    running->setFinished(true);
    TCB::yield();
}
