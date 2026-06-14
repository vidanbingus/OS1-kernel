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
    __asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));


    // size_t cnt = 19;
    // int* bufferNumbers = (int*)mem_alloc(cnt * sizeof (int));
    // print_ptr(bufferNumbers);
    // print_char('\n');
    //
    // mem_free(bufferNumbers);
    //
    // size_t cnt2 = 13;
    // int* bufferNumbers2 = (int*)mem_alloc(cnt2 * sizeof (int));
    // print_ptr(bufferNumbers2);
    // print_char('\n');
    //
    //
    // mem_free(bufferNumbers2);

    TCB* threads[5];
    threads[0] = TCB::createThread(nullptr, nullptr, nullptr);
    TCB::running = threads[0];

    // threads[1] = TCB::createThread(workerBodyA,nullptr);
    // print_string("Thread A created!\n");
    // threads[2] = TCB::createThread(workerBodyB, nullptr);
    // print_string("Thread B created!\n");
    // threads[3] = TCB::createThread(workerBodyC, nullptr);
    // print_string("Thread C created!\n");
    // threads[4] = TCB::createThread(workerBodyD, nullptr);
    // print_string("Thread D created!\n");
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
    __asm__ volatile ("csrs sstatus, 0x02");

    while (!(t1->isFinished() &&
            t2->isFinished() &&
            t3->isFinished() &&
            t4->isFinished()))
    {
        TCB::yield();
    }

    for (auto &thread : threads) {
        delete thread;
    }
    print_string("Finished!\n");



    return 0;
};

