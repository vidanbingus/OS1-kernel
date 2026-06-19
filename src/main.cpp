#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"
#include "../h/TCB.h"
#include "../h/workers.h"
#include "../h/_sem.h"
#include "../test/buffer.h"
#include "../h/syscall_cpp.h"

extern "C" void supervisorTrap(void);

static Semaphore* done;

class Worker : public Thread {
public:
    Worker(char c) : Thread(), ch(c) {}
protected:
    void run() override {
        for (int i = 0; i < 20; i++) {
            time_sleep(10);
            Console::putc(ch);
            for (volatile uint64 j = 0; j < 3000000; j++) { }
        }
        done->signal();
    }
private:
    char ch;
};

class Worker2 : public Thread {
public:
    Worker2(char c) : Thread(), ch(c) {}
protected:
    void run() override {
        for (int i = 0; i < 20; i++) {
            time_sleep(30);
            Console::putc(ch);
            for (volatile uint64 j = 0; j < 3000000; j++) { }
        }
        done->signal();
    }
private:
    char ch;
};

void userMain(void* arg) {
    print_string("\nC++ API test start\n");
    done = new Semaphore(0);          // globalni new -> syscall mem_alloc

    Worker w1('A');
    Worker2 w2('B');
    w1.start();
    w2.start();

    done->wait();
    done->wait();                     // cekaj da obe niti zavrse

    print_string("\nC++ API test done\n");
    delete done;                      // ~Semaphore -> sem_close; globalni delete -> syscall mem_free
}


int main() {

    print_string("-----------\n");

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);
    print_char('\n');


    // u IVTP stavi pocetnu adresu prekidnih rutina
    //__asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    RiscV::w_stvec((uint64)&supervisorTrap);

    char* buffer;
    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    char* buffer2;
    buffer2 = (char*)mem_alloc(30);
    print_ptr(buffer2);
    print_char('\n');
    char* buffer3;
    buffer3 = (char*)mem_alloc(30);
    print_ptr(buffer3);
    print_char('\n');

    mem_free(buffer);
    mem_free(buffer2);

    TCB* threads[2];

    thread_create(&threads[0], nullptr, nullptr);
    TCB::running = threads[0];

    thread_t t1;
    thread_create(&t1,&userMain, nullptr);
    print_string("Thread user1 created!\n");
    // thread_create(&t2,&userMain, nullptr);
    // print_string("Thread user2 created!\n");

    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    mem_free(buffer);

    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);


    while (!(t1->isFinished()))
    {
        thread_dispatch();
    }
    buffer = (char*)mem_alloc(30);
    print_ptr(buffer);
    print_char('\n');
    mem_free(buffer);

    print_string("Finished!\n");


    return 0;
};

