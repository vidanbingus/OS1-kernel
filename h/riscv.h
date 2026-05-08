#ifndef riscv
#define riscv

#include "../lib/hw.h"

class RiscV {
public:

    static void supervisorTrap();

private:

    static void handleSynchronousSysCalls()     __asm__ ("handleSynchronousSysCalls");
    static void handleTimerInterrupt()          __asm__ ("handleTimerInterrupt");
    static void handleConsoleInterrupt()        __asm__ ("handleConsoleInterrupt");

public:
    // pop sstatus.spp and sstatus.spie bits (has to be a non inline function)
    static void     popSppSpie();

    // funkcije koje su navedene ispod su u formatu r_sreg ili w_sreg sto oznacava citanje ili upis,
    // a sreg je sistemski registar na koji se to r ili w odnosi:

    static uint64   r_scause();

    static void     w_scause(uint64 scause);

    static uint64   r_sepc();

    static void     w_sepc(uint64 sepc);

    static uint64   r_stvec();

    static void     w_stvec(uint64 stvec);

    static uint64   r_stval();

    static void     w_stval(uint64 stval);

    static uint64   r_sip();

    static void     w_sip(uint64 sip);

    static uint64   r_sstatus();

    static void     w_sstatus(uint64 sstatus);

    // funkcije koje su navedene ispod maskiraju bitove sistemskih registara:
    // ms_ je mask-set, mc_ je mask-clear

    static void     ms_sstatus(uint64 mask);

    static void     mc_sstatus(uint64 mask);

    static void     ms_sip(uint64 mask);

    static void     mc_sip(uint64 mask);
};

#endif //riscv
