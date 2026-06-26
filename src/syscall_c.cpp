#include "../h/syscall_c.hpp"


void* mem_alloc(size_t size) {

    size_t newSize = ((size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE;


    register uint64 arg0 __asm__("a0") = 0x01;
    register uint64 arg1 __asm__("a1") = newSize;

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)                 // output (a0)
        : "r" (arg0), "r" (arg1)      // inputi (a0 i a1)
        : "memory"                    // govori kompajleru da ecall moze da menja ram
    );

    return (void*)arg0;

}

int mem_free(void* ptr) {

    register uint64 arg0 __asm__("a0") = 0x02;
    register uint64 arg1 __asm__("a1") = (uint64)ptr;

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1)
        : "memory"
    );

    return (int)arg0;
}

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) {

    void* sp = mem_alloc(DEFAULT_STACK_SIZE);

    //void* sp = MemoryAllocator::mem_alloc(DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE); ako je u blokovima

    if (!sp) return -1; 

    // racunamo vrh steka
   uint64 stack_top = (uint64)sp + DEFAULT_STACK_SIZE;


    register uint64 arg0 __asm__("a0") = 0x11;
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    register uint64 arg2 __asm__("a2") = (uint64)start_routine;
    register uint64 arg3 __asm__("a3") = (uint64)arg;
    register uint64 arg4 __asm__("a4") = stack_top;

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4)
        : "memory"
    );

    return (int)arg0;
}

void thread_dispatch() {
    register uint64 arg0 __asm__("a0") = 0x13;    // opCode thread_dispatch je 0x13
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0)
        : "memory"
    );
}

int thread_exit() {
    register uint64 arg0 __asm__("a0") = 0x12; // opCode za thread_exit
    __asm__ volatile (
        "ecall"
        : "=r"(arg0)
        : "r"(arg0)
        : "memory"
    );
    return (int)arg0;
}

int sem_open(sem_t *handle, unsigned init) {
    register uint64 arg0 __asm__("a0") = 0x21;                    // opCode za SEM_OPEN je 0x21
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    register uint64 arg2 __asm__("a2") = (uint64)init;

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1), "r" (arg2)
        : "memory"
    );

    return (int)arg0;
}

int sem_close(sem_t handle) {
    register uint64 arg0 __asm__("a0") = 0x22;                    // opCode za SEM_CLOSE je 0x22
    register uint64 arg1 __asm__("a1") = (uint64)handle;

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1)
        : "memory"
    );

    return (int)arg0;
}

int sem_wait(sem_t handle) {
    register uint64 arg0 __asm__("a0") = 0x23;              //opCode za SEM_WAIT je 0x23
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1)
        : "memory"
    );

    return (int)arg0;
}

int sem_signal(sem_t handle) {
    register uint64 arg0 __asm__("a0") = 0x24;              //opCode za SEM_SIGNAL je 0x24
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1)
        : "memory"
    );

    return (int)arg0;
}

int sem_wait_n(sem_t handle, unsigned n) {
    register uint64 arg0 __asm__("a0") = 0x25;              //opCode za SEM_WAIT_N je 0x25
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    register uint64 arg2 __asm__("a2") = (uint64)n;
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1), "r" (arg2)
        : "memory"
    );

    return (int)arg0;
}

int sem_signal_n(sem_t handle, unsigned n) {
    register uint64 arg0 __asm__("a0") = 0x26;              //opCode za SEM_SIGNAL_N je 0x26
    register uint64 arg1 __asm__("a1") = (uint64)handle;
    register uint64 arg2 __asm__("a2") = (uint64)n;
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)
        : "r" (arg0), "r" (arg1), "r" (arg2)
        : "memory"
    );

    return (int)arg0;
}


int time_sleep(time_t t) {
    register uint64 arg0 __asm__("a0") = 0x31;
    register uint64 arg1 __asm__("a1") = (uint64)t;
    __asm__ volatile ("ecall" : "=r"(arg0) : "r"(arg0), "r"(arg1) : "memory");
    return (int)arg0;
}

char getc() {
    register uint64 arg0 __asm__("a0") = 0x41;
    __asm__ volatile ("ecall" : "=r"(arg0) : "r"(arg0) : "memory");
    return (char)arg0;
}

void putc(char c) {
    register uint64 arg0 __asm__("a0") = 0x42;
    register uint64 arg1 __asm__("a1") = (uint64)c;
    __asm__ volatile ("ecall" : "=r"(arg0) : "r"(arg0), "r"(arg1) : "memory");
}
