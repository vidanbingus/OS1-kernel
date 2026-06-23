#include "../h/syscall_cpp.hpp"

Thread::Thread(void (*body)(void*), void* arg)
        : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread()
        : myHandle(nullptr), body(nullptr), arg(nullptr) {
}

Thread::~Thread() {}

int Thread::start() {
    return thread_create(&myHandle, &wrapper, this);
}
void Thread::dispatch()    { thread_dispatch(); }
int Thread::sleep(time_t t){ return time_sleep(t); }

void Thread::wrapper(void* arg) {
    Thread* self = (Thread*) arg;
    if (self->body != nullptr) self->body(self->arg);
    else self->run();
}

Semaphore::Semaphore(unsigned init) : myHandle(nullptr) {
    sem_open(&myHandle, init);
}
Semaphore::~Semaphore()  { sem_close(myHandle); }
int Semaphore::wait() {
    return sem_wait(myHandle);
}
int Semaphore::signal()  { return sem_signal(myHandle); }

PeriodicThread::PeriodicThread(time_t period)
        : Thread(), period(period) {}

void PeriodicThread::terminate() { myHandle->requestTerminate(); }

void PeriodicThread::run() {
    while (!myHandle->isTerminateRequested()) {
        periodicActivation();
        if (!myHandle->isTerminateRequested()) sleep(period);
    }
}

char Console::getc()       { return ::getc(); }
void Console::putc(char c) { ::putc(c); }
