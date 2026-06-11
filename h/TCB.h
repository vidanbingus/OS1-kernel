#ifndef PROJECT_BASE_TCB_H
#define PROJECT_BASE_TCB_H

#include "../lib/hw.h"
#include "../h/Scheduler.h"

class TCB {
public:

    ~TCB() { delete[] stack; }

    bool isFinished() const { return finished; }
    void setFinished(bool finished) { this->finished = finished; }

    using Body = void (*) (void);

    static TCB* createThread(Body body);

    static void yield();

public:
    static TCB* running;

private:
    TCB(Body body) :
        body(body),
        stack(body != nullptr ? new uint64[STACK_SIZE] : nullptr),
        context({ body != nullptr? (uint64)body : 0,
                stack != nullptr? (uint64) &stack[STACK_SIZE] : 0
                }),
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
    bool finished;

    static uint64 constexpr STACK_SIZE = 1024;
private:
    static void dispatch();
    static void contextSwitch(Context *oldContext, Context *newContext);

};




#endif //PROJECT_BASE_TCB_H
