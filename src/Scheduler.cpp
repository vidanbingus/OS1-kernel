#include "../h/Scheduler.hpp"
#include "../h/print.hpp"

List<TCB> Scheduler::readyQueue;

TCB *Scheduler::get()
{
    return readyQueue.removeFirst();
}

void Scheduler::put(TCB *tcb)
{
    readyQueue.addLast(tcb);
}
