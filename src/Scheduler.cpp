#include "../h/Scheduler.h"
#include "../h/print.h"

List<TCB> Scheduler::readyQueue;

TCB *Scheduler::get()
{
    return readyQueue.removeFirst();
}

void Scheduler::put(TCB *tcb)
{
    readyQueue.addLast(tcb);
}
