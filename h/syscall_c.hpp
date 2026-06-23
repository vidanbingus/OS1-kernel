#ifndef syscall_c
#define syscall_c

#include "_sem.hpp"
#include "../h/TCB.hpp"
#include "../lib/hw.h"
#include "../h/MemoryAllocator.hpp"

typedef TCB* thread_t;
typedef _sem* sem_t;

void* mem_alloc(size_t size);

int mem_free(void* ptr);

int thread_exit();

int thread_create(thread_t* handle,
                        void (*start_routine)(void*),
                        void* arg
                        );

void thread_dispatch();

int sem_open(sem_t* handle, unsigned init);

int sem_close(sem_t handle);

int sem_wait(sem_t id);

int sem_signal(sem_t id);

int  time_sleep(time_t);

char getc();

void putc(char);

#endif //syscall_c
