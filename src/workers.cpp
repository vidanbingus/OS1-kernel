#include "../h/workers.h"
#include "../lib/hw.h"
#include "../h/TCB.h"
#include "../h/print.h"

void workerBodyA()
{
    for (uint8 i = 0; i < 10; i++) {
        print_string("A: i=");
        print_int(i);
        print_string("\n");

        for (uint64 j = 0; j < 10000; j++) {
            for (uint64 k = 0; k < 30000; k++) {
                //busy wait
            }
        }
    }
}

void workerBodyB()
{
    for (uint8 i = 0; i < 16; i++) {
        print_string("B: i=");
        print_int(i);
        print_string("\n");

        for (uint64 j = 0; j < 10000; j++) {
            for (uint64 k = 0; k < 30000; k++) {
                //busy wait
            }
        }
    }
}

static uint64 fibonacci(uint64 n)
{
    if (n == 0 || n == 1) { return n; }
    if (n % 4 == 0) {
        TCB::yield();
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void workerBodyC()
{
    uint8 i = 0;
    for (; i < 3; i++)
    {
        print_string("C: i=");
        print_int(i);
        print_string("\n");
    }

    print_string("C: yield\n");
    __asm__ ("li t1, 7");
    TCB::yield();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    print_string("C: t1=");
    print_int(t1);
    print_string("\n");

    uint64 result = fibonacci(20);

    print_string("C: fibonaci=");
    print_int(result);
    print_string("\n");

    for (; i < 6; i++)
    {
        print_string("C: i=");
        print_int(i);
        print_string("\n");
    }
    TCB::running->setFinished(true);
    TCB::yield();
}

void workerBodyD()
{
    uint8 i = 10;
    // thread_join((thread_t) arg);
    for (; i < 13; i++)
    {
        print_string("D: i=");
        print_int(i);
        print_string("\n");
    }

    print_string("D: yield\n");
    __asm__ ("li t1, 5");
    TCB::yield();


    uint64 result = fibonacci(23);
    print_string("D: fibonaci=");
    print_int(result);
    print_string("\n");

    for (; i < 16; i++)
    {
        print_string("D: i=");
        print_int(i);
        print_string("\n");
    }

    TCB::running->setFinished(true);
    TCB::yield();
}

