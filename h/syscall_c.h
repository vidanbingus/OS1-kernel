#ifndef syscall_c
#define syscall_c

#include "../h/TCB.h"
#include "../lib/hw.h"
#include "../h/MemoryAllocator.h"

typedef TCB* thread_t;

void* mem_alloc(size_t size);

int mem_free(void* ptr);

//int thread_exit();

int thread_create(thread_t* handle);

int thread_create_actual(thread_t* handle,
                        void (*start_routine)(void*),
                        void* arg
                        );

void thread_dispatch();


#endif //syscall_c
