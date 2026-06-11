#include "../h/riscv.h"
#include "../h/opCodes.h"
#include "../h/MemoryAllocator.h"

void RiscV::extractSppSpie() {
    __asm__ volatile ("csrw sepc, ra");
    __asm__ volatile ("sret");
}

void RiscV::handleSynchronousSysCalls() {

    uint64 volatile sepc = r_sepc() + 4;
    uint64 volatile sstatus = r_sstatus();

    uint64 opCode;
    __asm__ volatile ("mv %0, a0" : "=r" (opCode));

    void* ptr;

    switch (opCode) {

        case MEM_ALLOC: {
            size_t size;
            __asm__ volatile ("mv %0, a1" : "=r" (size));
            ptr = MemoryAllocator::mem_alloc(size);
            __asm__ volatile ("mv a0, %0" : : "r" (ptr));
            break;
        }
        case MEM_FREE: {
            __asm__ volatile ("mv %0, a1" : "=r" (ptr));
            int retValue = MemoryAllocator::mem_free(ptr);
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case THREAD_CREATE: {
            break;
        }
        case THREAD_DISPATCH: {
            TCB::timeSliceCounter=0;
            TCB::dispatch();
            break;
        }
        default:
            break;
    }
    //vracamo staru statusnu rec i inkrementirani sepc
    w_sstatus(sstatus);
    w_sepc(sepc);

}

void RiscV::handleTimerInterrupt() {
    //verovatno ce trebati ovde da se desava promena konteksta

    TCB::timeSliceCounter++;
    if (TCB::timeSliceCounter >= TCB::running->getTimeSlice()) {
        uint64 sepc = r_sepc();
        uint64 sstatus = r_sstatus();

        TCB::timeSliceCounter = 0;

        TCB::dispatch();

        w_sepc(sepc);
        w_sstatus(sstatus);
    }
    mc_sip(SIP_SSIP);

}

void RiscV::handleConsoleInterrupt() {

    console_handler();
}

