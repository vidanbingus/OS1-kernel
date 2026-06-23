#ifndef PROJECT_BASE_SCHEDULER_H
#define PROJECT_BASE_SCHEDULER_H

#include "list.hpp"

class TCB;

class Scheduler
{
private:
    static List<TCB> readyQueue;

public:
    static TCB *get();

    static void put(TCB *tcb);

};


#endif //PROJECT_BASE_SCHEDULER_H
