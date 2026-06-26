#include "../h/TCB.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"


TCB* TCB::running = nullptr;
TCB* TCB::toDelete = nullptr;
uint64 TCB::timeSliceCounter = 0;
TCB* TCB::sleepHead = nullptr;
uint64 TCB::systemTime = 0;

TCB* TCB::createThread(Body body, void* arg, uint64* stack) {
    return new TCB(body,arg, stack);
}

void TCB::yield() {
    __asm__ volatile ("li a0, 0x13");   //0x13 je THREAD_DISPATCH
    __asm__ volatile("ecall");
}

void TCB::dispatch() {
    if (toDelete != nullptr && toDelete!=running) {
        delete toDelete;
        toDelete = nullptr;
    }
    TCB* old = running;
    if (!old->isFinished() && !old->isBlocked) { Scheduler::put(old); }
    running = Scheduler::get();
    TCB::contextSwitch(&old->context, &running->context);
}

void TCB::threadWrapper() {
    if (!running->isKernelThread)
        RiscV::popSppSpie();

    running->body(running->arg);
    thread_exit();
}

TCB* TCB::createKernelThread(Body body, void* arg) {
    uint64* stack = (uint64*)MemoryAllocator::mem_alloc(DEFAULT_STACK_SIZE);
    if (stack == nullptr) return nullptr;
    return new TCB(body, arg, stack, true);
}

void TCB::sleep(time_t ticks) {
    if (ticks == 0) return;

    running->wakeTime = systemTime + ticks;
    running->setBlocked(true);

    // sortirano umetanje u listu uspavanih (po vremenu budjenja)
    TCB* prev = nullptr;
    TCB* curr = sleepHead;
    while (curr != nullptr && curr->wakeTime <= running->wakeTime) {
        prev = curr;
        curr = curr->nextSleeper;
    }
    running->nextSleeper = curr;
    if (prev) prev->nextSleeper = running;
    else
        sleepHead = running;

    dispatch();
}

void TCB::tick() {
    systemTime++;
    while (sleepHead != nullptr && sleepHead->wakeTime <= systemTime) {
        TCB* t = sleepHead;
        sleepHead = sleepHead->nextSleeper;
        t->nextSleeper = nullptr;
        t->setBlocked(false);
        Scheduler::put(t);
    }
}