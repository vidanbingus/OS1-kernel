#include "../h/syscall_c.h"

//umesto da svaki put pripremas syscall pojedinacnim instrikcijama koje loaduju u a0,a1..
//napravi funkciju koja to radi
//takodje, za povratnu vrednost napravi funkciju koja to radi

void* mem_alloc(size_t size) {

    size_t newSize = ((size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE;

    //priprema argumenata za sistemski poziv

    register uint64 arg0 __asm__("a0") = 0x01;    // opCode za mem_alloc ide u a0
    register uint64 arg1 __asm__("a1") = newSize; // Velicina ide u a1

    // Izvršavamo ecall.
    // ":: "r"(arg0), "r"(arg1)" govori kompajleru da su ovi registri INPUTI.
    // "=r"(arg0) govori da će ecall vratiti rezultat nazad u a0 (OUTPUT).
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)                 // Output (a0)
        : "r" (arg0), "r" (arg1)      // Inputi (a0 i a1)
        : "memory"                    // Govori kompajleru da ecall može menjati RAM
    );

    return (void*)arg0;

}

int mem_free(void* ptr) {
    // Vezujemo C promenljive direktno za registre a0 i a1
    register uint64 arg0 __asm__("a0") = 0x02;           // Op-kod za mem_free ide u a0
    register uint64 arg1 __asm__("a1") = (uint64)ptr;  // Kastujemo pokazivač i stavljamo u a1

    __asm__ volatile (
        "ecall"
        : "=r" (arg0)             // Output: rezultat sistemskog poziva se vraća u a0
        : "r" (arg0), "r" (arg1)  // Inputi: kompajler garantuje da su a0 i a1 spremni pre ecall-a
        : "memory"                // Clobber: oslobađanje memorije menja stanje RAM-a
    );

    return (int)arg0;
}

int thread_create(thread_t *handle, void (*start_routine)(void *), void *arg) {
    // 1. Prvo radimo alokaciju steka u čistom C++ kodu.
    // Dok se ovo izvršava, kompajler drži handle, start_routine i arg bezbedno na steku ili u s-registrima.
    void* sp = MemoryAllocator::mem_alloc(DEFAULT_STACK_SIZE / MEM_BLOCK_SIZE);
    
    // Ako alokacija steka nije uspela, odmah vraćamo grešku (opciono, ali dobra praksa)
    if (!sp) return -1; 

    // Računamo vrh steka (stack pointer raste nadole, pa mu dodajemo veličinu)
   uint64 stack_top = (uint64)sp + DEFAULT_STACK_SIZE;

    // 2. Vezujemo argumente direktno za željene registre po tvom šablonu
    register uint64 arg0 __asm__("a0") = 0x03;                    // Pretpostavljam op-kod za thread_create (npr. 0x03)
    register uint64 arg1 __asm__("a1") = (uint64)handle;        // Prvi argument funkcije
    register uint64 arg2 __asm__("a2") = (uint64)start_routine; // Pokazivač na funkciju npr. u a2
    register uint64 arg3 __asm__("a3") = (uint64)arg;           // Argument za tu funkciju npr. u a3
    register uint64 arg4 __asm__("a4") = stack_top;               // Vrh alociranog steka šaljemo kernelu u a4

    // Napomena: U tvom kodu si koristio a6 i a7, ali ako ti ABI ili tvoj kernel traži a1, a2, a3, a4,
    // prilagodi imena u __asm__("...") navodnicima prema tvojoj specifikaciji sistemskog poziva!

    // 3. Ispaljujemo ecall u jednom jedinom bloku
    __asm__ volatile (
        "ecall"
        : "=r" (arg0)                                     // Izlaz: rezultat se vraća u a0
        : "r" (arg0), "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4) // Ulazi: svi ovi registri moraju biti napunjeni
        : "memory"                                        // Clobber: menjamo memoriju
    );

    return (int)arg0;
}