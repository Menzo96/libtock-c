/*
 * Call a single random syscall with random args.
 */
#include <stdio.h>

#include "arch.h"
#include "arg-decoder.h"
#include "random.h"
#include "shm.h"
#include "sanitise.h"
#include "syscall.h"
#include "tables.h"
#include "trinity.h"
#include "logging.h"
#include "runner.h"


static unsigned int *active_syscalls;

static void choose_syscall_table(void)
{
	active_syscalls = runnerdata->active_syscalls->sysc;
}

static bool set_syscall_nr(syscallrecord_t *rec)
{
	printf("runner: set_syscall_nr in\n");

	struct syscallentry *entry;
	unsigned int syscallnr;

retry:
	if (no_syscalls_enabled() == true) {
		fuzz_log("No more syscalls enabled. Exiting\n");
		runnerdata->exit_reason = EXIT_NO_SYSCALLS_ENABLED;
		return false;
	}

	printf("runner: set_syscall_nr 1\n");

	/* Ok, we're doing another syscall, let's pick one. */
	choose_syscall_table();
	syscallnr = rnd() % max_nr_syscalls;

	printf("runner: set_syscall_nr max_nr_syscalls: %u\n", max_nr_syscalls);

	printf("runner: set_syscall_nr syscallnr: %u\n", syscallnr);

	/* If we got a syscallnr which is not active repeat the attempt,
	 * since another child has switched that syscall off already.*/
	if (active_syscalls[syscallnr] == 0)
		goto retry;

	syscallnr = active_syscalls[syscallnr] - 1;

	printf("runner: set_syscall_nr syscallnr: %u\n", syscallnr);

	//TODO
	// if (validate_specific_syscall_silent(syscalls, syscallnr) == false) {
	// 	deactivate_syscall(syscallnr);
	// 	goto retry;
	// }

	printf("runner: set_syscall_nr 3\n");

	entry = get_syscall_entry(syscallnr);
	if (entry->flags & EXPENSIVE) {
		if (!ONE_IN(1000))
			goto retry;
	}

	if (entry->flags & AVOID_SYSCALL) {
		goto retry;
	}

	printf("runner: set_syscall_nr: %d\n", syscallnr);
	rec->nr = syscallnr;
	printf("runner: set_syscall_nr out\n");

	return true;
}

bool random_syscall(void)
{
	syscallrecord_t *rec;
	int ret = false;

	rec = &runnerdata->syscall;

	fuzz_log("stats.op_count: %ld\n", runnerdata->stats.op_count);

	printf("runner: random_syscall 0\n");

	if (set_syscall_nr(rec) == FAIL)
		return FAIL;
	
	printf("runner: random_syscall 1\n");

	// memset(rec->postbuffer, 0, POSTBUFFER_LEN);

	/* Generate arguments, print them out */
	generate_syscall_args(rec);

	printf("runner: random_syscall 2\n");

	//MNZ - TODO - use for logging
	output_syscall_prefix(rec);

	printf("runner: random_syscall 3\n");

	do_syscall(rec);

	printf("runner: random_syscall 4\n");

	//MNZ - TODO - use for logging
	output_syscall_postfix(rec);

	printf("runner: random_syscall 5\n");

	//MNZ - TODO - adapt this
	handle_syscall_ret(rec);

	printf("runner: random_syscall 6\n");

	ret = true;

	return ret;
}
