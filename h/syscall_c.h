#ifndef syscall_c
#define syscall_c

#include "_sem.h"
#include "../h/TCB.h"
#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"

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

#endif //syscall_c
