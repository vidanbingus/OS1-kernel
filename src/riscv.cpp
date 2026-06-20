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
            delete handle;
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
        case TIME_SLEEP: {
            uint64 t;
            __asm__ volatile ("mv %0, a1" : "=r" (t));
            TCB::sleep(t);
            uint64 retValue = 0;
            __asm__ volatile ("mv a0, %0" : : "r" (retValue));
            break;
        }
        case PUTC: {
            uint64 ch;
            __asm__ volatile ("mv %0, a1" : "=r" (ch));
            KConsole::putc((char)ch);
            break;
        }
        case GETC: {
            char ch = KConsole::getc();
            __asm__ volatile ("mv a0, %0" : : "r" ((uint64)ch));
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

    mc_sip(SIP_SSIP);
}

void RiscV::handleConsoleInterrupt() {

    KConsole::handleInterrupt();
}

