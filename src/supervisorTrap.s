# 1 "src/supervisorTrap.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/riscv64-linux-gnu/include/stdc-predef.h" 1 3
# 32 "<command-line>" 2
# 1 "src/supervisorTrap.S"
.extern handleSynchronousSysCalls
.extern handleTimerInterrupt
.extern handleConsoleInterrupt

.align 4
.global supervisorTrap
.type supervisorTrap, @function

supervisorTrap:
    addi sp, sp, -256
    .irp index, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\index, \index * 8(sp)
    .endr

    # ovde treba provera koji je razlog skoka u prekidnu rutinu
    # 1) ili je sistemski poziv, sto je sinhrono -> scause == 8 ili 9
    # 2) ili je tajmer koji je softverski prekid -> scause == 0x8000000000000001
    # 3) ili je konzola koja je asinhroni prekid spolja -> scause == 0x8000000000000009


    csrr t0, scause
    li t1, 8
    beq t0, t1, handleSynchronousSysCalls__LABEL
    li t1, 9
    beq t0, t1, handleSynchronousSysCalls__LABEL
    li t1, 0x8000000000000001
    beq t0, t1, handleTimerInterrupt__LABEL
    li t1, 0x8000000000000009
    beq t0, t1, handleConsoleInterrupt__LABEL

    j panic__LABEL

    handleSynchronousSysCalls__LABEL:
    call handleSynchronousSysCalls
    jal popAllRegistersExceptA0__LABEL # a0 je x10

    handleTimerInterrupt__LABEL:
    call handleTimerInterrupt
    jal popAllRegisters__LABEL

    handleConsoleInterrupt__LABEL:
    call handleConsoleInterrupt
    jal popAllRegisters__LABEL

    popAllRegisters__LABEL: # ako je syscall, tj. ako ima povratnu vrednost onda ne treba
    ld a0, 80(sp) # da se pop-uje a0 (mada ako syscall nema povratnu onda videcemo)
    popAllRegistersExceptA0__LABEL:
    .irp index, 0,1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 #primetiti da fali index 10
    ld x\index, \index * 8(sp)
    .endr
    addi sp, sp, 256

    sret

    panic__LABEL:
    call handleUnknownTrap # registri su vec sacuvani na vrhu rutine; ova funkcija ne vraca kontrolu
    repeat:
    j repeat # za svaki slucaj zavrti u mestu
