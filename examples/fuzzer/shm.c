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

struct shm_s *shm;
unsigned int shm_size;

void create_shm(void)
{
	shm_size = sizeof(struct shm_s);

	shm = malloc(shm_size);

	memset(shm, 0, shm_size);
}

void init_shm(void)
{

	fuzz_log("shm is at %p\n", shm);

	shm->debug = true;

	shm->stats.op_count = 0;

	shm->seed = init_seed();

	shm->exit_reason = STILL_RUNNING;

	memset(&shm->syscall, 0, sizeof(struct syscallrecord));
}
