#ifndef memoryallocator
#define memoryallocator

#include "../lib/hw.h"

class MemoryAllocator {
public:
    static  void     init();
    static  void*    mem_alloc  (size_t size); // u bajtovima poravnato na velicinu bloka
    static  int      mem_free   (void* ptr);

public:
    // zaglavlje koje se smesta na pocetak svakog slobodnog segmenta
    struct BlockHeader {
        size_t          size; // velicina segmenta u blokovima
        BlockHeader*    next;
    };
    static BlockHeader* freeMemHead;
};

#endif //memoryallocator
