#include <stdio.h>

#include <tock.h>
#include <timer.h>

void init_fuzz_log(void);

void fuzz_log(const char* format, ...);