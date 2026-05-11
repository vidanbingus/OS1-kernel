#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"

extern "C" void supervisorTrap(void);

void fibonacci(int* buffer, size_t count) {
    buffer[0] = 1;
    buffer[1] = 1;
    for (size_t i = 2; i < count; ++i) {
        buffer[i] = buffer[i-1] + buffer[i-2];
    }
}
void main() {
    __putc('f');
    __putc('\n');

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);


    // u IVTP stavi pocetnu adresu prekidnih rutina
    __asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));

    char* buffer = (char*)mem_alloc(17);
    print_ptr(buffer);

    if (mem_free(buffer)!=0){
        print_string("greska pri dealociranju\n");
    };


    size_t cnt = 13;
    int* bufferNumbers = (int*)mem_alloc(cnt * sizeof (int));
    fibonacci(bufferNumbers, cnt);
    for (size_t i = 0; i < cnt; ++i) {
        print_int(bufferNumbers[i]);
    }
    if (mem_free(bufferNumbers)!=0){
        print_string("greska pri dealociranju\n");
    };

    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    //while (1);

};

