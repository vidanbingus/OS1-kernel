#include "../h/print.h"
#include "../lib/console.h"

int print_char(char ch) {
    __putc(ch);
    return 0;
}

int print_string(const char* str) {
    while (*str != '\0') {
        __putc(*(str++));
    }
    return 0;
}

int print_ptr(const void* ptr) {
    __putc('0');
    __putc('x');
    return print_size(size_t(ptr), 16);
}

int print_size(size_t val, uint8 base) {
    if (val == 0) {
        __putc('0');
        return 0;
    }

    static char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    const uint8 bufferSize = 32;
    unsigned char buffer[bufferSize];

    uint8 count = 0;
    while (val != 0) {
        if (count >= bufferSize)
            return 999;
        buffer[count++] = val % base;
        val = val / base;
    }
    while (count != 0) {
        __putc(digits[buffer[--count]]);
    }
    return 0;
}

int print_int(int val, uint8 base)
{
    if (val < 0) {
        __putc('-');
        val = -val;
    }

    return print_size((size_t)val, base);
}


