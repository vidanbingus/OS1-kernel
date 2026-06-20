// =====================================================================
//  testMain.cpp  -  sveobuhvatni test jezgra
//
//  Pokriva:
//    1) alokaciju/dealokaciju memorije (mem_alloc / mem_free)
//    2) niti preko C++ API-a (Thread + run override) i C API-a (thread_create)
//    3) asinhronu promenu konteksta / deljenje vremena (niti koje NE yield-uju)
//    4) semafore: medjusobno iskljucenje + proizvodjac/potrosac (Buffer)
//    5) time_sleep (sortirano budjenje uspavanih niti)
//    6) PeriodicThread
//    7) thread_exit u pola izvrsavanja
//    8) konzolu: izlaz (baferisana nit jezgra) + ulaz (getc, preko prekida)
//
//  INTEGRACIJA:
//    Opcija A) U main.cpp obrisi postojecu definiciju userMain i dodaj
//              na vrh:   extern void userMain(void*);
//              pa ovaj fajl ubaci u src/ (ili test/) da se prevodi.
//    Opcija B) Prekopiraj sadrzaj ispod preko svog userMain-a u main.cpp.
//
//  Tajmer radi na 10 Hz (10 prekida/s), pa je 1 "tik" ~ 100 ms.
// =====================================================================

#include "../h/syscall_cpp.h"
#include "../h/syscall_c.h"
#include "../h/print.h"
#include "../test/buffer.h"
#include "../test/TestMain.h"

// ---------------------------------------------------------------------
//  Pomocne stvari
// ---------------------------------------------------------------------

static void busyWait(uint64 loops) {
    for (volatile uint64 i = 0; i < loops; i++) { }
}

static void line(const char* s) {            // kratak sinhroni ispis sa nl
    print_string(s);
    print_string("\n");
}

static void banner(const char* s) {
    print_string("\n========== ");
    print_string(s);
    print_string(" ==========\n");
}

static Semaphore* gDone;                     // brojac zavrsenih niti (init 0)

// =====================================================================
//  TEST 1 — asinhrona promena konteksta (deljenje vremena)
//  Dve niti samo busy-petljaju i ispisuju svoj znak. Posto NE yield-uju,
//  isprepleten izlaz (AABBABAB...) dokazuje da preotimanje na tajmer radi.
//  Izlaz ide kroz Console::putc => testira i baferisanu izlaznu nit.
// =====================================================================
class TimeShareWorker : public Thread {
public:
    TimeShareWorker(char c, int n) : Thread(), ch(c), count(n) {}
protected:
    void run() override {
        for (int i = 0; i < count; i++) {
            Console::putc(ch);
            busyWait(30000000);               // dovoljno dugo da tajmer preotme
        }
        gDone->signal();
    }
private:
    char ch;
    int  count;
};
class TimeShareWorker2 : public Thread {
public:
    TimeShareWorker2(char c, int n) : Thread(), ch(c), count(n) {}
protected:
    void run() override {
        for (int i = 0; i < count; i++) {
            Console::putc(ch);
            busyWait(30000000);               // dovoljno dugo da tajmer preotme
            if (i == 7) {
                __asm__("csrw sscratch, 0x5");
            }
        }
        gDone->signal();
    }
private:
    char ch;
    int  count;
};

static void test_timeSharing() {

    banner("TEST 1: deljenje vremena (asinhrona promena konteksta)");
    line("Ocekivano: isprepletani znakovi 'a' i 'b' (ne svi 'a' pa svi 'b').");

    TimeShareWorker w1('a', 12);
    TimeShareWorker2 w2('b', 12);
    w1.start();
    w2.start();

    gDone->wait();
    print_string("\n[TEST 1 OK]\n");
    Thread::sleep(10);                        // settle: pusti niti da dosegnu thread_exit
}

// =====================================================================
//  TEST 2 — semafor kao mutex (medjusobno iskljucenje)
//  Vise niti uvecava deljeni brojac uz prosirenu kriticnu sekciju.
//  Ako mutex radi, konacna vrednost == broj_niti * po_niti.
//  Koristi se C API (thread_create) da se pokrije i taj sloj.
// =====================================================================
static Semaphore* gMutex;
static volatile int gCounter;

static void counterBody(void* arg) {
    long perThread = (long)arg;
    for (long i = 0; i < perThread; i++) {
        gMutex->wait();
        int tmp = gCounter;
        busyWait(20000);                     // siri kriticnu sekciju da iznese trku na videlo
        gCounter = tmp + 1;
        gMutex->signal();
    }
    gDone->signal();
}

static void test_mutex() {
    banner("TEST 2: semafor kao mutex (C API niti)");
    const long NTH = 4, PER = 50;
    gCounter = 0;

    thread_t th[NTH];
    for (long i = 0; i < NTH; i++) {
        thread_create(&th[i], &counterBody, (void*)PER);
    }
    for (long i = 0; i < NTH; i++) {
        gDone->wait();
    }

    print_string("brojac = ");
    print_int(gCounter);
    print_string("  (ocekivano ");
    print_int((int)(NTH * PER));
    print_string(")\n");
    line(gCounter == (int)(NTH * PER) ? "[TEST 2 OK]" : "[TEST 2 FAIL!]");
    Thread::sleep(10);
}

// =====================================================================
//  TEST 3 — proizvodjac/potrosac preko tvoje Buffer klase
//  Mali kapacitet (4) namerno tera blokiranje na 'pun' i na 'prazan',
//  pa proverava blokiranje/deblokiranje semafora u oba smera.
// =====================================================================
static Buffer* gBuf;

static void producerBody(void* arg) {
    char base = (char)(long)arg;
    for (int i = 0; i < 10; i++) {
        gBuf->put(base + i);                 // 'A'..'J'
        busyWait(500000);
    }
    gDone->signal();
}

static void consumerBody(void* arg) {
    for (int i = 0; i < 10; i++) {
        int v = gBuf->get();
        Console::putc((char)v);              // izlaz kroz baferisanu nit
    }
    gDone->signal();
}

static void test_producerConsumer() {

    banner("TEST 3: proizvodjac/potrosac (Buffer, kapacitet 4)");
    line("Ocekivano: ispisuje se A..J (potrosnja prati proizvodnju).");

    gBuf = new Buffer(4);

    thread_t prod, cons;
    thread_create(&prod, &producerBody, (void*)(long)'A');
    thread_create(&cons, &consumerBody, nullptr);

    gDone->wait();
    gDone->wait();
    print_string("\n");

    delete gBuf;                             // ~Buffer ispisuje stanje + zatvara semafore
    line("[TEST 3 OK]");
    Thread::sleep(10);
}

// =====================================================================
//  TEST 4 — time_sleep (sortirano budjenje)
//  Niti se uspavljuju na 15, 5, 10 tikova ALI se prave tim redom.
//  Ako sleep-lista radi, redosled budjenja mora biti 5 -> 10 -> 15.
// =====================================================================
static void sleeperBody(void* arg) {
    long ticks = (long)arg;
    Thread::sleep(ticks);
    print_string("probudila se nit posle ");
    print_int((int)ticks);
    print_string(" tikova\n");
    gDone->signal();
}

static void test_sleep() {
    banner("TEST 4: time_sleep (ocekivani redosled budjenja 5, 10, 15)");

    thread_t s1, s2, s3;
    thread_create(&s1, &sleeperBody, (void*)15);
    thread_create(&s2, &sleeperBody, (void*)5);
    thread_create(&s3, &sleeperBody, (void*)10);

    gDone->wait();
    gDone->wait();
    gDone->wait();
    line("[TEST 4 OK]");
    Thread::sleep(10);
}

// =====================================================================
//  TEST 5 — PeriodicThread
//  Aktivira se zadati broj puta sa periodom, pa sam sebe terminira.
// =====================================================================
class Blinker : public PeriodicThread {
public:
    Blinker(time_t p, int n) : PeriodicThread(p), left(n) {}
protected:
    void periodicActivation() override {
        print_string("  periodicna aktivacija, preostalo ");
        print_int(left - 1);
        print_string("\n");
        if (--left <= 0) {
            terminate();                     // run() petlja staje
            gDone->signal();                 // poslednja akcija nad ovim objektom
        }
    }
private:
    int left;
};

static void test_periodic() {
    banner("TEST 5: PeriodicThread (perioda 3 tika, 4 aktivacije)");

    Blinker b(3, 4);
    b.start();

    gDone->wait();
    line("[TEST 5 OK]");
    Thread::sleep(10);                        // pusti periodicnu nit da potpuno izadje
}

// =====================================================================
//  TEST 6 — thread_exit u pola petlje
//  Nit treba da se ugasi na i==3 i nikad ne ispise i>=3.
// =====================================================================
static void earlyExitBody(void* /*arg*/) {
    for (int i = 0; i < 8; i++) {
        print_string("  iteracija ");
        print_int(i);
        print_string("\n");
        if (i == 3) {
            gDone->signal();                 // signaliziraj PRE gasenja
            thread_exit();                   // ovde se nit zavrsava
        }
        busyWait(500000);
    }
    print_string("  OVO NE SME DA SE ISPISE\n");
    gDone->signal();
}

static void test_threadExit() {
    banner("TEST 6: thread_exit u pola posla (staje na iteraciji 3)");

    thread_t t;
    thread_create(&t, &earlyExitBody, nullptr);

    gDone->wait();
    line("[TEST 6 OK]");
    Thread::sleep(10);
}

// =====================================================================
//  TEST 7 — konzola, ULAZ (interaktivno)
//  getc blokira dok ne kucas. Eho ide kroz Console::putc (baferisano).
//  Kucaj nesto pa pritisni Enter.
// =====================================================================
static void test_consoleInput() {
    banner("TEST 7: konzola - ulaz (INTERAKTIVNO)");
    line("Kucaj nekoliko znakova pa Enter; bice eho-ovani nazad:");

    while (true) {
        char c = Console::getc();
        if (c == '\r' || c == '\n') {
            Console::putc('\n');
            break;
        }
        Console::putc(c);                    // eho preko baferisane izlazne niti
    }
    line("[TEST 7 OK]");
    Thread::sleep(10);
}

// =====================================================================
//  userMain
// =====================================================================
void userMain(void* /*arg*/) {
    line("\n#### POCETAK SVEOBUHVATNOG TESTA ####");

    // --- ne-nitni spot-check alokatora ---
    banner("TEST 0: mem_alloc / mem_free");
    void* p1 = mem_alloc(100);
    void* p2 = mem_alloc(100);
    print_string("p1 = "); print_ptr(p1); print_string("\n");
    print_string("p2 = "); print_ptr(p2); print_string("\n");
    line(p1 && p2 && p1 != p2 ? "[TEST 0 OK]" : "[TEST 0 FAIL!]");
    mem_free(p1);
    mem_free(p2);

    // --- sinhronizacioni objekti ---
    gDone  = new Semaphore(0);
    gMutex = new Semaphore(1);


    test_timeSharing();
    test_mutex();
    test_producerConsumer();
    test_sleep();
    test_periodic();
    test_threadExit();
    test_consoleInput();                     // poslednji: ceka na tastaturu

    delete gMutex;
    delete gDone;

    line("\n#### SVI TESTOVI ZAVRSENI ####");
}
