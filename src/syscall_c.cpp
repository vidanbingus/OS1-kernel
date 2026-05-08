#include "../h/syscall_c.h"

//umesto da svaki putpripremas syscall pojedinacnim instrikcijama koje loaduju u a0,a1..
//napravi funkciju koja to radi
//takodje, za povratnu vrednost napravi funkciju koja to radi

void* mem_alloc(size_t size) {

    size = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;

    //priprema argumenata za sistemski poziv
    __asm__ volatile ("li a0, 0x01");               //0x01 je opCode za mem_alloc
    __asm__ volatile ("mv a1, %0" : : "r" (size));
    __asm__ volatile ("ecall");

    //kroz a0 vracamo povratnu vrednost ovog sistemskog poziva
    uint64 retValue;
    __asm__ volatile ("mv %0, a0" : "=r" (retValue));
    return (void*)retValue;

}

int mem_free(void* ptr) {

    __asm__ volatile ("li a0, 0x02");               //0x02 je opCode za mem_free
    __asm__ volatile ("mv a1, %0" : : "r" (ptr));   //potencilano mora da se kastuje ptr u uint64
    __asm__ volatile ("ecall");

    uint64 retValue;
    __asm__ volatile ("mv %0, a0" : "=r" (retValue));
    return (int)retValue;
}