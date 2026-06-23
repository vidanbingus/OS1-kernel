#include "../h/KConsole.hpp"

char KConsole::inBuf[CAP];
char KConsole::outBuf[CAP];
int  KConsole::inHead = 0;
int  KConsole::inTail = 0;
int  KConsole::outHead = 0;
int  KConsole::outTail = 0;
_sem* KConsole::inItem   = nullptr;
_sem* KConsole::outItem  = nullptr;
_sem* KConsole::outSpace = nullptr;
TCB*  KConsole::outThread = nullptr;

void KConsole::init() {
    inHead = inTail = 0;
    outHead = outTail = 0;
    inItem   = new _sem(0);
    outItem  = new _sem(0);
    outSpace = new _sem(CAP - 1);
    outThread = TCB::createKernelThread(&KConsole::outputBody, nullptr);
}

void KConsole::putc(char c) {
    outSpace->wait();                  // blokira ako je izlazni bafer pun
    outBuf[outTail] = c;
    outTail = (outTail + 1) % CAP;
    outItem->signal();                 // budi internu nit za slanje
}

char KConsole::getc() {
    inItem->wait();                    // blokira ako nema znakova
    char c = inBuf[inHead];
    inHead = (inHead + 1) % CAP;
    return c;
}

void KConsole::handleInterrupt() {
    int irq = plic_claim();
    if (irq == (int)CONSOLE_IRQ) {
        // procitaj sve znakove pristigle sa tastature
        while (*(volatile uint8*)CONSOLE_STATUS & CONSOLE_RX_STATUS_BIT) {
            char c = *(volatile char*)CONSOLE_RX_DATA;
            int next = (inTail + 1) % CAP;
            if (next != inHead) {       // ima mesta u ulaznom baferu
                inBuf[inTail] = c;
                inTail = next;
                inItem->signal();
            }
            // ako je bafer pun -> znak se odbacuje
        }
    }
    plic_complete(irq);
}

void KConsole::outputBody(void*) {
    while (true) {
        outItem->wait();                // cekaj znak (kooperativno se odrice procesora)
        char c = outBuf[outHead];
        outHead = (outHead + 1) % CAP;
        outSpace->signal();
        // prozivanje: cekaj da kontroler bude spreman za slanje
        while (!(*(volatile uint8*)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT)) { }
        *(volatile char*)CONSOLE_TX_DATA = c;
    }
}