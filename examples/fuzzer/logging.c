#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <tock.h>
#include <timer.h>

#include "logging.h"

#define TIMER_STEP 5

static volatile uint32_t counter = 0;

static void log_timer_cb (__attribute__ ((unused)) int arg0,
                      __attribute__ ((unused)) int arg1,
                      __attribute__ ((unused)) int arg2,
                      __attribute__ ((unused)) void* userdata) 
                      {
  counter = counter + TIMER_STEP;
}

void init_fuzz_log(void)
{
    tock_timer_t *timer = (tock_timer_t*)malloc(sizeof(tock_timer_t));
    timer_every(TIMER_STEP, log_timer_cb, NULL, timer);
}

static uint32_t get_time(void) 
{
    return counter;
}

void fuzz_log(const char* format, ...)
{
    va_list argptr;

    va_start(argptr, format);
    printf("[%lu]:", get_time());
    vfprintf(stdout, format, argptr);
    va_end(argptr);
}