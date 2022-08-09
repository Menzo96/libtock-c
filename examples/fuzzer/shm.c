/*
 * Shared mapping creation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "arch.h"
#include "random.h"
#include "shm.h"
#include "utils.h"
#include "logging.h"
#include "runner.h"

struct shm_s *shm;
unsigned int shm_size;

char act_sys_buf[SYSCALLS_BUF_SZ] __attribute__((aligned(SYSCALLS_BUF_SZ)));

void create_shm(void)
{
	shm_size = sizeof(struct shm_s);

	shm = malloc(shm_size);

	memset(shm, 0, shm_size);
}

void init_shm(void)
{
	unsigned int i;

	fuzz_log("shm is at %p\n", shm);

	shm->debug = true;

	shm->stats.op_count = 0;

	// shm->seed = init_seed();

	shm->exit_reason = STILL_RUNNING;

	printf("sizeof(activesyscalls_t): %d\n", sizeof(activesyscalls_t));

	memset(&act_sys_buf[0], 0, SYSCALLS_BUF_SZ);
	shm->active_syscalls = (activesyscalls_t *)(&act_sys_buf[0]);

	shm->runners = (runnerdata_t**)malloc(max_runners * sizeof(runnerdata_t*));
	shm->nr_runners = 0;

	for_each_runner(i) {
		shm->runners[i] = NULL;
	}
}
