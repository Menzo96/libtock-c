#pragma once

#include "arch.h"
#include "runner.h"
#include "exit.h"
#include "stats.h"
#include "syscall.h"

//TODO - find a better size (e.g 5 * sizeof(syscall)), may overflow
#define SYSCALLS_BUF_SZ 256

void create_shm(void);
void init_shm(void);

struct shm_s {

	// struct syscallrecord syscall;
	runnerdata_t **runners;
	unsigned int nr_runners;

	struct stats_s stats;

	/* rng related state */
	unsigned int seed;

	/* Indices of syscall in syscall table that are active.
	 * All indices shifted by +1. Empty index equals to 0.
	 */
	activesyscalls_t *active_syscalls;
	// int active_syscalls[MAX_NR_SYSCALL];
	// unsigned int nr_active_syscalls;

	/* various flags. */
	enum exit_reasons exit_reason;
	bool ready;

	//global debug flag.
	bool debug;
	
};
extern struct shm_s *shm;
extern unsigned int shm_size;

bool random_syscall(void);
