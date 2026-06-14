#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"
#include "../h/TCB.h"
#include "../h/workers.h"

extern "C" void supervisorTrap(void);

int main() {

    print_string("-----------\n");

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);
    print_char('\n');



    // u IVTP stavi pocetnu adresu prekidnih rutina
    //__asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    RiscV::w_stvec((uint64)&supervisorTrap);

    size_t cnt = 19;
    int* bufferNumbers = (int*)mem_alloc(cnt * sizeof (int));
    print_ptr(bufferNumbers);
    print_char('\n');

    mem_free(bufferNumbers);

    size_t cnt2 = 13;
    int* bufferNumbers2 = (int*)mem_alloc(cnt2 * sizeof (int));
    print_ptr(bufferNumbers2);
    print_char('\n');


    mem_free(bufferNumbers2);

    TCB* threads[5];

    thread_create(&threads[0], nullptr, nullptr);
    TCB::running = threads[0];

    thread_t t1, t2, t3, t4;
    thread_create(&t1,workerBodyA, nullptr);
    print_string("Thread A1 created!\n");
    thread_create(&t2,workerBodyB, nullptr);
    print_string("Thread B1 created!\n");
    thread_create(&t3,workerBodyA, nullptr);
    print_string("Thread A2 created!\n");
    thread_create(&t4,workerBodyB, nullptr);
    print_string("Thread B2 created!\n");

    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);

    while (!(t1->isFinished() &&
            t2->isFinished() &&
            t3->isFinished() &&
            t4->isFinished()))
    {
        thread_dispatch();
    }
    
    print_string("Finished!\n");



    return 0;
};

