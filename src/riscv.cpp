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

    void* ptr;

    uint64 opCode;
    __asm__ volatile ("mv %0, a0" : "=r" (opCode));
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
            thread_t** handle;
            TCB::Body body;
            void* arg;
            uint64* stack_top; // pazi: iz thread_create smo poslali VRH steka (stack_top)

            __asm__ volatile ("mv %0, a1" : "=r" (handle));
            __asm__ volatile ("mv %0, a2" : "=r" (body));
            __asm__ volatile ("mv %0, a3" : "=r" (arg));
            __asm__ volatile ("mv %0, a6" : "=r" (stack_top));

            // Mala korekcija: Pošto ti je u a4 stigao VRH steka (stack_top = sp + DEFAULT_STACK_SIZE),
            // a tvoj konstruktor želi pokazivač na POČETAK steka, moramo ga vratiti unazad:
            // (Pod uslovom da je veličina u bajtovima DEFAULT_STACK_SIZE)
            uint64* stack_start = (uint64*)((uint64)stack_top - DEFAULT_STACK_SIZE);


            TCB* newThread = TCB::createThread(body, arg, stack_start);

            int retValue = 0;

            if (newThread == nullptr) {
                retValue = -1;
            } else {
                *handle = (thread_t*)newThread;
            }

            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case THREAD_EXIT: {
            TCB::running->setFinished(true);
            TCB::toDelete = TCB::running;
            TCB::dispatch();
            break;
        }

        case THREAD_DISPATCH: {
            TCB::timeSliceCounter=0;
            TCB::dispatch();
            break;
        }
        case SEM_OPEN: {
            sem_t** handle;
            uint64 semValue;
            __asm__ volatile ("mv %0, a1" : "=r" (handle));
            __asm__ volatile ("mv %0, a2" : "=r" (semValue));
            _sem* semaphore = new _sem((unsigned)semValue);

            int retValue = 0;
            if (!semaphore) { retValue = -1;}
            else {
                *handle = (sem_t*)semaphore;
            }
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case SEM_CLOSE: {
            sem_t handle;
            __asm__ volatile ("mv %0, a1" : "=r" (handle));
            int retValue = handle->close();
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case SEM_WAIT: {
            sem_t handle;
            __asm__ volatile ("mv %0, a1" : "=r" (handle));
            int retValue = handle->wait();
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case SEM_SIGNAL: {
            sem_t handle;
            __asm__ volatile ("mv %0, a1" : "=r" (handle));
            int retValue = handle->signal();
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
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
        uint64 volatile sepc = r_sepc();
        uint64 volatile sstatus = r_sstatus();

        TCB::timeSliceCounter = 0;

        TCB::dispatch();

        w_sstatus(sstatus);
        w_sepc(sepc);
    }

    mc_sip(SIP_SSIP);
}

void RiscV::handleConsoleInterrupt() {

    console_handler();
}

