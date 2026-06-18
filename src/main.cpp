#include "../lib/console.h"
#include "../h/riscv.h"
#include "../h/syscall_c.h"
#include "../h/MemoryAllocator.h"
#include "../h/print.h"
#include "../h/TCB.h"
#include "../h/workers.h"
#include "../h/_sem.h"
#include "../test/buffer.h"
extern "C" void supervisorTrap(void);


struct thread_data {
    int id;
    Buffer *buffer;
    sem_t wait;
};

static volatile int threadEnd = 0;

static void producerKeyboard(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;

    int key;
    int i = 0;
    while ((key = __getc()) != 0x1b) {
        data->buffer->put(key);
        i++;

        if (i % (10 * data->id) == 0) {
            thread_dispatch();
        }
    }

    threadEnd = 1;
    data->buffer->put('!');

    sem_signal(data->wait);
}

static void producer(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;

    int i = 0;
    while (!threadEnd) {
        data->buffer->put(data->id + '0');
        i++;

        if (i % (10 * data->id) == 0) {
            thread_dispatch();
        }
    }

    sem_signal(data->wait);
}

static void consumer(void *arg) {
    struct thread_data *data = (struct thread_data *) arg;

    int i = 0;
    while (!threadEnd) {
        int key = data->buffer->get();
        i++;

        __putc(key);

        if (i % (5 * data->id) == 0) {
            thread_dispatch();
        }

        if (i % 80 == 0) {
            __putc('\n');
        }
    }

    while (data->buffer->getCnt() > 0) {
        int key = data->buffer->get();
        __putc(key);
    }

    sem_signal(data->wait);
}


void userMain(void* arg) {
    int cap = 200;
    int numOfProducers = 6;

    static sem_t waitForAll;

    Buffer* buffer = new Buffer(cap);
    sem_open(&waitForAll,0);
    thread_t threads[numOfProducers];
    thread_t consumerThread;
    struct thread_data data[numOfProducers+1];

    data[numOfProducers].id = numOfProducers;
    data[numOfProducers].buffer = buffer;
    data[numOfProducers].wait = waitForAll;
    thread_create(&consumerThread, consumer, data + numOfProducers);


    for (int i = 0; i < numOfProducers; i++) {
        data[i].id = i;
        data[i].buffer = buffer;
        data[i].wait = waitForAll;

        thread_create(threads + i,
                      i > 0 ? producer : producerKeyboard,
                      data + i);
    }

    thread_dispatch();

    for (int i = 0; i <= numOfProducers; i++) {
        sem_wait(waitForAll);
    }

    sem_close(waitForAll);

    delete buffer;
}

int main() {

    print_string("-----------\n");

    MemoryAllocator::init();
    print_ptr(HEAP_START_ADDR);
    print_char('\n');



    // u IVTP stavi pocetnu adresu prekidnih rutina
    //__asm__ volatile ("csrw stvec, %0" : : "r" ((uint64)&supervisorTrap));
    RiscV::w_stvec((uint64)&supervisorTrap);


    TCB* threads[2];

    thread_create(&threads[0], nullptr, nullptr);
    TCB::running = threads[0];

    thread_t t1;
    thread_create(&t1,&userMain, nullptr);
    print_string("Thread user1 created!\n");
    // thread_create(&t2,&userMain, nullptr);
    // print_string("Thread user2 created!\n");


    // ukljuci prekide
    //__asm__ volatile ("csrs sstatus, 0x02");
    RiscV::ms_sstatus(RiscV::SSTATUS_SIE);


    while (!(t1->isFinished()))
    {
        thread_dispatch();
    }

    print_string("Finished!\n");


    return 0;
};

