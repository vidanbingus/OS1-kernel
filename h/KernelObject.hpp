#ifndef PROJECT_BASE_KERNELOBJECT_H
#define PROJECT_BASE_KERNELOBJECT_H

#include "../h/MemoryAllocator.hpp"

class KernelObject {
public:
    void* operator new(size_t sz)    { return MemoryAllocator::mem_alloc(sz); }
    void* operator new[](size_t sz)  { return MemoryAllocator::mem_alloc(sz); }
    void  operator delete(void* p)   { MemoryAllocator::mem_free(p); }
    void  operator delete[](void* p) { MemoryAllocator::mem_free(p); }
};


#endif //PROJECT_BASE_KERNELOBJECT_H
