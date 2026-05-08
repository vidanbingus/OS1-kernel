#ifndef riscv
#define riscv

#include "../lib/hw.h"

class RiscV {
public:

    static void supervisorTrap();

private:

    static void handleSynchronousSysCalls()     __asm__ ("handleSynchronousSysCalls") ;
    static void handleTimerInterrupt()          __asm__ ("handleTimerInterrupt") ;
    static void handleConsoleInterrupt()        __asm__ ("handleConsoleInterrupt");

public:

};

#endif //riscv
