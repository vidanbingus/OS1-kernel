#include "../h/MemoryAllocator.h"

MemoryAllocator::BlockHeader* MemoryAllocator::freeMemHead = nullptr;

void MemoryAllocator::init() {
    freeMemHead = (BlockHeader*)HEAP_START_ADDR;

    size_t totalBytes = (char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR;
    size_t totalBlocks = totalBytes / MEM_BLOCK_SIZE;

    freeMemHead->size = totalBlocks;
    freeMemHead->next = nullptr;
}

void* MemoryAllocator::mem_alloc(size_t size) {
    if (!size) return nullptr;

    size_t blocksNeeded = (size + MEM_BLOCK_SIZE -1) / MEM_BLOCK_SIZE + 1; // jedan dodatan block potreban za BlockHeader
    BlockHeader* prev = nullptr;
    BlockHeader* curr = freeMemHead;

    // First fit
    while (curr != nullptr) {
        if (curr->size >= blocksNeeded) { // nasli smo block
            if (curr->size > blocksNeeded) {
                // cepanje slobodnog segmenta
                BlockHeader* newFreeBlock = (BlockHeader*)((char*)curr + blocksNeeded * MEM_BLOCK_SIZE);
                newFreeBlock->size = curr->size - blocksNeeded;
                newFreeBlock->next = curr->next;

                if (prev) {
                    prev->next = newFreeBlock;
                }
                else freeMemHead = newFreeBlock;
            }
            else {
                // uzima se ceo segment
                if (prev)
                    prev->next = curr->next;
                else
                    freeMemHead = curr->next;
            }

            curr->next = nullptr;                   // curr sad predstavlja slobodan segment
            return (char*)curr + MEM_BLOCK_SIZE;    // vraca se pokazivac na deo memorije odmah nakon zaglavlja
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

int MemoryAllocator::mem_free(void *ptr) {
    if (!ptr) return -1;

    BlockHeader* blockToFree = (BlockHeader*)((char*)ptr - MEM_BLOCK_SIZE); // pokazivac na zaglavlje je jedan block pre
    if ((char*)blockToFree < (char*)HEAP_START_ADDR || (char*)blockToFree >= (char*)HEAP_END_ADDR)
        return -1;

    BlockHeader* prev = nullptr;
    BlockHeader* curr = freeMemHead;

    while (curr != nullptr && curr < blockToFree) {
        prev = curr;
        curr = curr->next;
    }

    //umetanje slobodnog segmenta
    blockToFree->next = curr;
    if (prev)
        prev->next = blockToFree;
    else
        freeMemHead = blockToFree;

    //spajanje sa sledecim segmentom (ako su susedni)
    if (curr != nullptr && (char*)blockToFree + blockToFree->size * MEM_BLOCK_SIZE == (char*)curr) {
        blockToFree->size += curr->size;
        blockToFree->next = curr->next;
    }

    //spajanje sa prethodnim segmentom (ako su susedni)
    if (prev != nullptr && (char*)prev + prev->size * MEM_BLOCK_SIZE == (char*)blockToFree) {
        prev->size += blockToFree->size;
        prev->next = blockToFree->next;
    }

    return 0;
}