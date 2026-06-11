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



    // u IVTP stavi pocetnu adresu prekidnih rutina
    __asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));

    size_t cnt = 13;
    int* bufferNumbers = (int*)mem_alloc(cnt * sizeof (int));
    mem_free(bufferNumbers);


    TCB* threads[3];
    threads[0] = TCB::createThread(nullptr);
    TCB::running = threads[0];

    threads[1] = TCB::createThread(workerBodyA);
    print_string("Thread A created!\n");
    threads[2] = TCB::createThread(workerBodyB);
    print_string("Thread B created!\n");

    while (!(threads[1]->isFinished() && threads[2]->isFinished()))
    {
        TCB::yield();
    }

    for (auto &thread : threads) {
        delete thread;
    }
    print_string("Finished!\n");


    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    //while (1);
    return 0;
};

