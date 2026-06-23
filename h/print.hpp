#ifndef print
#define print

#include "../lib/hw.h"

int print_char  (char ch);
int print_string(const char* str);

int print_ptr   (const void* ptr);

int print_int   (int val, uint8 base = 10);
int print_size  (size_t val, uint8 base = 10);


#endif //print
