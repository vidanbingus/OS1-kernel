#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"

extern "C" void supervisorTrap(void);

void main() {
    __putc('f');
    __putc('\n');

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);
    char* buffer = (char*)MemoryAllocator::mem_alloc(1);
    print_ptr((void*)buffer);
    char* buffer2 = (char*)MemoryAllocator::mem_alloc(2);
    print_ptr((void*)buffer2);
    char* buffer3 = (char*)MemoryAllocator::mem_alloc(3);
    print_ptr((void*)buffer3);
    if (MemoryAllocator::mem_free((void*)buffer2) != 0) {
        print_string("error");
    }
    char* buffer4 = (char*)MemoryAllocator::mem_alloc(1);
    print_ptr((void*)buffer4);
    char* buffer5 = (char*)MemoryAllocator::mem_alloc(0);
    print_ptr((void*)buffer5);

    // u IVTP stavi pocetnu adresu prekidnih rutina
    __asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));

    if (MemoryAllocator::mem_free((void*)buffer) != 0) {
        print_string("error");
    }

    if (MemoryAllocator::mem_free((void*)buffer3) != 0) {
        print_string("error\n");
    }
    if (MemoryAllocator::mem_free((void*)buffer4) != 0) {
        print_string("error\n");
    }
    if (MemoryAllocator::mem_free((void*)buffer5) != 0) {
        print_string("error\n");
    }
    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    //while (1);

};

