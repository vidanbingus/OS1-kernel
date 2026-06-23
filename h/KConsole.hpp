#ifndef PROJECT_BASE_KCONSOLE_H
#define PROJECT_BASE_KCONSOLE_H
#include "../lib/hw.h"
#include "../h/_sem.hpp"
#include "../h/TCB.hpp"

class KConsole {
public:
    static void init();
    static void putc(char c);          // iz obrade sistemskog poziva PUTC
    static char getc();                // iz obrade sistemskog poziva GETC
    static void handleInterrupt();     // iz prekidne rutine za konzolu

private:
    static void outputBody(void*);     // telo interne (sistemske) niti za slanje
    enum { CAP = 256 };
    static char inBuf[CAP];
    static char outBuf[CAP];
    static int  inHead, inTail;
    static int  outHead, outTail;
    static _sem* inItem;               // znakovi spremni za citanje
    static _sem* outItem;              // znakovi spremni za slanje
    static _sem* outSpace;             // slobodna mesta u izlaznom baferu
    static TCB*  outThread;
};


#endif //PROJECT_BASE_KCONSOLE_H
