#pragma once

#include "arch.h"
// #include "child.h"
#include "exit.h"
// #include "files.h"
// #include "locks.h"
// #include "net.h"
#include "stats.h"
#include "syscall.h"
// #include "types.h"

void create_shm(void);
void init_shm(void);

struct shm_s {

	struct syscallrecord syscall;

	struct stats_s stats;

	/* rng related state */
	unsigned int seed;

	/* Indices of syscall in syscall table that are active.
	 * All indices shifted by +1. Empty index equals to 0.
	 */
	int active_syscalls[MAX_NR_SYSCALL];
	unsigned int nr_active_syscalls;

	/* various flags. */
	enum exit_reasons exit_reason;
	bool ready;

	//global debug flag.
	bool debug;
	
};
extern struct shm_s *shm;
extern unsigned int shm_size;

bool random_syscall(void);
