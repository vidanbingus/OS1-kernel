#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"
#include "../h/TCB.h"
#include "../h/workers.h"
#include "../h/_sem.h"
#include "../test/buffer.h"
#include "../h/syscall_cpp.h"
#include "../h/KConsole.h"
#include "../test/TestMain.h"

extern "C" void supervisorTrap(void);

extern void userMain(void*);


int main() {

    MemoryAllocator::init();

    // u IVTP stavi pocetnu adresu prekidnih rutina
    //__asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    RiscV::w_stvec((uint64)&supervisorTrap);
    KConsole::init();

    TCB* threads[2];

    thread_create(&threads[0], nullptr, nullptr);
    TCB::running = threads[0];

    thread_t t1;
    thread_create(&t1,&userMain, nullptr);

    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);


    while (!(t1->isFinished()))
    {
        thread_dispatch();
    }

    return 0;
};

