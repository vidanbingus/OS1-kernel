#include "../lib/mem.h"
#include "../h/MemoryAllocator.h"



void *operator new(size_t n)
{
    //return MemoryAllocator::mem_alloc(n);
    return __mem_alloc(n);
}

void *operator new[](size_t n)
{
    //return MemoryAllocator::mem_alloc(n);
    return __mem_alloc(n);
}

void operator delete(void *p) noexcept
{
    //MemoryAllocator::mem_free(p);
    __mem_free(p);
}

void operator delete[](void *p) noexcept
{
    //MemoryAllocator::mem_free(p);
    __mem_free(p);
}
