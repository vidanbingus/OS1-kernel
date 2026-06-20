#include "../h/riscv.h"
#include "../h/opCodes.h"
#include "../h/MemoryAllocator.h"
#include "../h/KConsole.h"

// --- panic ispis: direktno na konzolni kontroler, bez sistemskih poziva/semafora ---
static void panicPutc(char c) {
    while (!(*(volatile uint8*)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT)) { }
    *(volatile char*)CONSOLE_TX_DATA = c;
}

static void panicPrint(const char* s) {
    while (*s) panicPutc(*s++);
}

static void panicPrintHex(uint64 v) {
    panicPrint("0x");
    const char* d = "0123456789ABCDEF";
    bool started = false;
    for (int i = 60; i >= 0; i -= 4) {
        uint8 nib = (v >> i) & 0xF;
        if (nib) started = true;
        if (started || i == 0) panicPutc(d[nib]);
    }
}

void RiscV::handleUnknownTrap() {
    uint64 scause  = r_scause();
    uint64 sepc    = r_sepc();
    uint64 stval   = r_stval();
    uint64 sstatus = r_sstatus();

    bool fromUser = (sstatus & SSTATUS_SPP) == 0;   // SPP=0 -> trap je dosao iz korisnickog rezima

    panicPrint("\n*** TRAP: neocekivan uzrok ***\n");
    panicPrint("  scause = "); panicPrintHex(scause); panicPrint("\n");
    panicPrint("  sepc   = "); panicPrintHex(sepc);   panicPrint("\n");
    panicPrint("  stval  = "); panicPrintHex(stval);  panicPrint("\n");

    if (!fromUser) {
        // Greska u sistemskom rezimu (jezgro, idle/main, konzolna nit) -> nema oporavka.
        panicPrint("  -> greska u jezgru, zaustavljam sistem\n");
        *((volatile uint32*)0x100000) = 0x5555;
        while (true) { }
    }

    // SOFT: pukla je korisnicka nit -> ugasi samo nju i nastavi (isto kao thread_exit).
    panicPrint("  -> gasim nit koja je pukla i nastavljam\n");
    TCB::running->setFinished(true);
    TCB::toDelete = TCB::running;
    TCB::dispatch();
}

void RiscV::extractSppSpie() {

    __asm__ volatile ("csrw sepc, ra");
    __asm__ volatile("csrc sstatus, %0" : : "r"(SSTATUS_SPP));  // SPP = 0  -> posle sret-a smo u KORISNICKOM rezimu
    __asm__ volatile("csrs sstatus, %0" : : "r"(SSTATUS_SPIE)); // SPIE = 1 -> posle sret-a su prekidi ukljuceni (SIE <- SPIE)
    __asm__ volatile ("sret");
}

uint64 RiscV::handleSynchronousSysCalls(uint64 a0, uint64 a1, uint64 a2, uint64 a3,
            uint64 a4, uint64 a5, uint64 a6, uint64 a7) {

    uint64 volatile sepc = r_sepc() + 4;
    uint64 volatile sstatus = r_sstatus();

    void* ptr;
    uint64 retValue = 0;

    uint64 opCode = (uint64)a0;
    switch (opCode) {

        case MEM_ALLOC: {
            size_t size = (size_t)a1;
            ptr = MemoryAllocator::mem_alloc(size);
            retValue = (uint64)ptr;
            break;
        }
        case MEM_FREE: {
            ptr = (void*)a1;
            retValue = MemoryAllocator::mem_free(ptr);
            break;
        }
        case THREAD_CREATE: {
            thread_t** handle = (thread_t**)a1;
            TCB::Body body = (TCB::Body)a2;
            void* arg = (void*)a3;
            uint64* stack_top = (uint64*)a4; // pazi: iz thread_create smo poslali VRH steka (stack_top)


            uint64* stack_start = (uint64*)((uint64)stack_top - DEFAULT_STACK_SIZE);


            TCB* newThread = TCB::createThread(body, arg, stack_start);

            retValue = 0;

            if (newThread == nullptr) {
                retValue = -1;
            } else {
                *handle = (thread_t*)newThread;
            }
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
            sem_t** handle = (sem_t**)a1;
            uint64 semValue = (uint64)a2;
            _sem* semaphore = new _sem((unsigned)semValue);

            retValue = 0;
            if (!semaphore) { retValue = -1;}
            else {
                *handle = (sem_t*)semaphore;
            }
            break;
        }
        case SEM_CLOSE: {
            sem_t handle = (sem_t)a1;
            retValue = handle->close();
            delete handle;
            break;
        }
        case SEM_WAIT: {
            sem_t handle = (sem_t)a1;
            retValue = handle->wait();
            break;
        }
        case SEM_SIGNAL: {
            sem_t handle = (sem_t)a1;
            retValue = handle->signal();
            break;
        }
        case TIME_SLEEP: {
            uint64 t = (uint64)a1;
            TCB::sleep(t);
            retValue = 0;
            break;
        }
        case PUTC: {
            KConsole::putc((char)a1);
            break;
        }
        case GETC: {
            char ch = KConsole::getc();
            retValue = (uint64)ch;
            break;
        }
        default:
            break;
    }
    //vracamo staru statusnu rec i inkrementirani sepc
    w_sstatus(sstatus);
    w_sepc(sepc);

    return retValue;
}

void RiscV::handleTimerInterrupt() {
    mc_sip(SIP_SSIP);

    TCB::tick();

    TCB::timeSliceCounter++;

    if (TCB::timeSliceCounter >= TCB::running->getTimeSlice()) {
        uint64 volatile sepc = r_sepc();
        uint64 volatile sstatus = r_sstatus();

        TCB::timeSliceCounter = 0;

        TCB::dispatch();

        w_sstatus(sstatus);
        w_sepc(sepc);
    }


}

void RiscV::handleConsoleInterrupt() {

    KConsole::handleInterrupt();
}

