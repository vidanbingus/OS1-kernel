#ifndef syscall_c
#define syscall_c

#include "../lib/hw.h"

void* mem_alloc(size_t size);

int mem_free(void* ptr);



#endif //syscall_c
