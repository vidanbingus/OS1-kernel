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

inline uint64 Riscv::r_scause() {
    uint64 volatile scause;
    __asm__ volatile ("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

inline void Riscv::w_scause(uint64 scause) {
    __asm__ volatile ("csrw scause, %[scause]" : : [scause] "r"(scause));
}

inline uint64 Riscv::r_sepc() {
    uint64 volatile sepc;
    __asm__ volatile ("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

inline void Riscv::w_sepc(uint64 sepc) {
    __asm__ volatile ("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

inline uint64 Riscv::r_stvec() {
    uint64 volatile stvec;
    __asm__ volatile ("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

inline void Riscv::w_stvec(uint64 stvec) {
    __asm__ volatile ("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

inline uint64 Riscv::r_stval() {
    uint64 volatile stval;
    __asm__ volatile ("csrr %[stval], stval" : [stval] "=r"(stval));
    return stval;
}

inline void Riscv::w_stval(uint64 stval) {
    __asm__ volatile ("csrw stval, %[stval]" : : [stval] "r"(stval));
}

inline uint64 Riscv::r_sip() {
    uint64 volatile sip;
    __asm__ volatile ("csrr %[sip], sip" : [sip] "=r"(sip));
    return sip;
}

inline void Riscv::w_sip(uint64 sip) {
    __asm__ volatile ("csrw sip, %[sip]" : : [sip] "r"(sip));
}

inline uint64 Riscv::r_sstatus() {
    uint64 volatile sstatus;
    __asm__ volatile ("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

inline void Riscv::w_sstatus(uint64 sstatus) {
    __asm__ volatile ("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));
}

inline void Riscv::ms_sstatus(uint64 mask) {
    __asm__ volatile ("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sstatus(uint64 mask) {
    __asm__ volatile ("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::ms_sip(uint64 mask) {
    __asm__ volatile ("csrs sip, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sip(uint64 mask) {
    __asm__ volatile ("csrc sip, %[mask]" : : [mask] "r"(mask));
}

#endif //riscv
