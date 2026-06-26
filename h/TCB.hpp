#ifndef PROJECT_BASE_TCB_H
#define PROJECT_BASE_TCB_H

#include "../lib/hw.h"
#include "../h/Scheduler.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/print.hpp"
#include "../h/KernelObject.hpp"

class TCB : public KernelObject {
public:

    ~TCB() {
        if (stack != nullptr) {
            MemoryAllocator::mem_free(stack);
        }
    }

    bool isFinished() const { return finished; }
    void setFinished(bool finished) { this->finished = finished; }
    void setBlocked(bool blocked) { this->isBlocked = blocked; }
    uint64 getTimeSlice() const { return timeSlice; }
    bool isMain() const { return isMainThread; }
    bool wasSemClosed() const { return semClosedWhileWaiting; }
    void setSemClosed(bool b) { semClosedWhileWaiting = b; }
    void requestTerminate() { terminateRequested = true; }
    bool isTerminateRequested() const { return terminateRequested; }

    using Body = void (*) (void*);

    static TCB* createThread(Body body, void* arg, uint64* stack);

    static void yield();

    static void sleep(time_t ticks);
    static void tick();
    static TCB* createKernelThread(Body body, void* arg);

public:
    static TCB* running;
    static TCB* toDelete;

private:
    // TCB(Body body, uint64 timeSlice) :
    //     body(body),
    //     stack(body != nullptr ? new uint64[STACK_SIZE] : nullptr),
    //     context({ (uint64)&threadWrapper,
    //             stack != nullptr? (uint64) &stack[STACK_SIZE] : 0
    //             }),
    //     timeSlice(timeSlice),
    //     finished(false)
    // {
    //     if (body!=nullptr) Scheduler::put(this);
    // }

    TCB(Body body, void* arg, uint64* stack, bool kernel = false) :
            body(body),
            arg(arg),
            stack(body != nullptr ? stack : nullptr),
            context({(uint64) &threadWrapper,
                     stack != nullptr ? (uint64) ((char*)stack + DEFAULT_STACK_SIZE) : 0
                    }),
            timeSlice(DEFAULT_TIME_SLICE),
            finished(false),
            isMainThread(false),
            isBlocked(false),
            isKernelThread(kernel),
            wakeTime(0),
            nextSleeper(nullptr)
    {
        if (body!=nullptr) {
            isMainThread = true;
            Scheduler::put(this);
        }
    }

    struct Context {
        uint64 ra;
        uint64 sp;
    };
    Body body;
    void* arg;
    uint64* stack;
    Context context;
    uint64 timeSlice;
    bool finished;
    bool isMainThread;
    bool isBlocked;
    bool semClosedWhileWaiting = false;
    bool terminateRequested = false;
    bool isKernelThread;     // za konzolu
    uint64 wakeTime;         // tik na koji nit treba probuditi
    TCB* nextSleeper;        // ulancavanje u listi uspavanih
    unsigned semNeed;        // koliko jedinica nit ceka u sem_wait_n

    static uint64 timeSliceCounter;
    static TCB* sleepHead;
    static uint64 systemTime;

    // static uint64 constexpr STACK_SIZE = 1024;
    // static uint64 constexpr TIME_SLICE = 2;

    friend class RiscV;
    friend class _sem;

    static void threadWrapper();
private:
    static void dispatch();
    static void contextSwitch(Context *oldContext, Context *newContext);

};




#endif //PROJECT_BASE_TCB_H
