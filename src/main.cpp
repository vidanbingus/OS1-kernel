#include "../lib/console.h"
#include "../h/riscv.h"

extern "C" void supervisorTrap(void);

void main() {
    __putc('f');
    __putc('\n');

    // u IVTP stavi pocetnu adresu prekidnih rutina
    __asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    // ukljuci prekide
    __asm__ volatile ("csrs sstatus, 0x02");
    while (1);

};

