#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"
#include "../h/TCB.h"
#include "../h/workers.h"
#include "../h/_sem.h"
#include "../test/buffer.h"
extern "C" void supervisorTrap(void);


static void busyWork(void* arg) {
    const char* tag = (const char*)arg;
    while (true) {
        print_string(tag);
        for (volatile uint64 j = 0; j < 5000000; j++) { /* busy wait */ }
    }
}

void userMain(void* arg) {
    thread_t a, b;
    thread_create(&a, busyWork, (void*)"A");
    thread_create(&b, busyWork, (void*)"B");
    while (true) {
        print_string("M");
        for (volatile uint64 j = 0; j < 5000000; j++) { /* busy wait */ }
    }
}

int main() {

    print_string("-----------\n");

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);
    print_char('\n');


    // u IVTP stavi pocetnu adresu prekidnih rutina
    //__asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    RiscV::w_stvec((uint64)&supervisorTrap);

    char* buffer;
    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    char* buffer2;
    buffer2 = (char*)mem_alloc(30);
    print_ptr(buffer2);
    print_char('\n');

    mem_free(buffer);
    mem_free(buffer2);

    TCB* threads[2];

    thread_create(&threads[0], nullptr, nullptr);
    TCB::running = threads[0];

    thread_t t1;
    thread_create(&t1,&workerBodyA, nullptr);
    print_string("Thread user1 created!\n");
    // thread_create(&t2,&userMain, nullptr);
    // print_string("Thread user2 created!\n");

    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    mem_free(buffer);

    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);


    while (!(t1->isFinished()))
    {
        thread_dispatch();
    }
    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    mem_free(buffer);

    print_string("Finished!\n");


    return 0;
};

