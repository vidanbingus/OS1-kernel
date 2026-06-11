#ifndef PROJECT_BASE_TCB_H
#define PROJECT_BASE_TCB_H

#include "../lib/hw.h"
#include "../h/Scheduler.h"

class TCB {
public:

    ~TCB() { delete[] stack; }

    bool isFinished() const { return finished; }
    void setFinished(bool finished) { this->finished = finished; }
    uint64 getTimeSlice() const { return timeSlice; }

    using Body = void (*) (void);

    static TCB* createThread(Body body);

    static void yield();

public:
    static TCB* running;

private:
    TCB(Body body, uint64 timeSlice) :
        body(body),
        stack(body != nullptr ? new uint64[STACK_SIZE] : nullptr),
        context({ (uint64)&threadWrapper,
                stack != nullptr? (uint64) &stack[STACK_SIZE] : 0
                }),
        timeSlice(timeSlice),
        finished(false)
    {
        if (body!=nullptr) Scheduler::put(this);
    }
    struct Context {
        uint64 ra;
        uint64 sp;
    };
    Body body;
    uint64* stack;
    Context context;
    uint64 timeSlice;
    bool finished;

    static uint64 timeSliceCounter;

    static uint64 constexpr STACK_SIZE = 1024;
    static uint64 constexpr TIME_SLICE = 2;

    friend class RiscV;

    static void threadWrapper();
private:
    static void dispatch();
    static void contextSwitch(Context *oldContext, Context *newContext);

};




#endif //PROJECT_BASE_TCB_H
