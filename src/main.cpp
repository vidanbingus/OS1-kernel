#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/print.hpp"
#include "../h/TCB.hpp"
#include "../h/workers.hpp"
#include "../h/_sem.hpp"
#include "../h/syscall_cpp.hpp"
#include "../h/KConsole.hpp"
#include "../test/userMain.hpp"

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

    //RiscV::ms_sstatus(RiscV::SSTATUS_SUM);   // jezgro sme da dira korisnicki stek i handle-ove
    // ukljuci prekide
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);


    while (!(t1->isFinished()))
    {
        thread_dispatch();
    }

    //halt
    *((volatile uint32*)0x100000) = 0x5555;

    return 0;
};

