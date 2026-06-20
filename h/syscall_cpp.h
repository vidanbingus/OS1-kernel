#ifndef PROJECT_BASE_SYSCALL_CPP_H
#define PROJECT_BASE_SYSCALL_CPP_H


#include "../h/syscall_c.h"
#include "../lib/hw.h"

class Thread {
public:
    Thread(void (*body)(void*), void* arg);
    virtual ~Thread();
    int start();
    static void dispatch();
    static int sleep(time_t);
protected:
    Thread();
    virtual void run() {}
    thread_t myHandle;
private:
    void (*body)(void*);
    void* arg;
    static void wrapper(void*);          // pokrece body ili run
};

class Semaphore {
public:
    Semaphore(unsigned init = 1);
    virtual ~Semaphore();
    int wait();
    int signal();
private:
    sem_t myHandle;
};

class PeriodicThread : public Thread {
public:
    void terminate();
protected:
    PeriodicThread(time_t period);
    virtual void periodicActivation() {}
    void run() override;                 // override Thread::run (ne menja set virtuala)
private:
    time_t period;
};

class Console {
public:
    static char getc();
    static void putc(char);
};

#endif //PROJECT_BASE_SYSCALL_CPP_H
