//
// Created by os on 6/18/26.
//

#ifndef PROJECT_BASE_BUFFER_H
#define PROJECT_BASE_BUFFER_H
#include "../h/syscall_c.h"
#include "../h/print.h"

class Buffer {
private:
    int cap;
    int *buffer;
    int head, tail;

    sem_t spaceAvailable;
    sem_t itemAvailable;
    sem_t mutexHead;
    sem_t mutexTail;

public:
    Buffer(int _cap);
    ~Buffer();

    void put(int val);
    int get();

    int getCnt();

};

#endif //PROJECT_BASE_BUFFER_H
