#pragma once

#include "syscall.h"
#include "stats.h"
#include "exit.h"

#define NO_PID -1

extern unsigned int max_runners;

#define for_each_runner(i)	for (i = 0; i < max_runners; i++)

#define RUNNER_BUF_SZ 2048

typedef enum {
  // Test runner has just started; awaiting supervisor signal to begin tests.
  RunnerInit,

  // An individual test is ready to be started.
  RunnerStart,

  // An individual test has completed.
  RunnerEnd,

  // The test runner has finished all tests, and is exiting.
  RunnerCleanup,

} runner_state_t;

typedef struct runnerdata {

  runner_state_t state;

	syscallrecord_t syscall;

	/* last time the runner made progress. */
	// struct uint32_t tp;

	unsigned long op_nr;

  struct stats_s stats;

  enum exit_reasons exit_reason;

  activesyscalls_t *active_syscalls;

	unsigned int seed;

	int pid;

} runnerdata_t;

extern runnerdata_t *runnerdata;

runnerdata_t* init_runner(void);