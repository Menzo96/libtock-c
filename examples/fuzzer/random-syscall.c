/*
 * Call a single random syscall with random args.
 */

#include "arch.h"
#include "arg-decoder.h"
#include "random.h"
#include "shm.h"
#include "sanitise.h"
#include "syscall.h"
#include "tables.h"
#include "trinity.h"
#include "logging.h"


static int *active_syscalls;

static void choose_syscall_table(void)
{
	active_syscalls = shm->active_syscalls;
}

static bool set_syscall_nr(struct syscallrecord *rec)
{
	struct syscallentry *entry;
	unsigned int syscallnr;

retry:
	if (no_syscalls_enabled() == true) {
		fuzz_log("No more syscalls enabled. Exiting\n");
		shm->exit_reason = EXIT_NO_SYSCALLS_ENABLED;
		return false;
	}

	/* Ok, we're doing another syscall, let's pick one. */
	choose_syscall_table();
	syscallnr = rnd() % max_nr_syscalls;

	/* If we got a syscallnr which is not active repeat the attempt,
	 * since another child has switched that syscall off already.*/
	if (active_syscalls[syscallnr] == 0)
		goto retry;

	syscallnr = active_syscalls[syscallnr] - 1;

	if (validate_specific_syscall_silent(syscalls, syscallnr) == false) {
		deactivate_syscall(syscallnr);
		goto retry;
	}

	entry = get_syscall_entry(syscallnr);
	if (entry->flags & EXPENSIVE) {
		if (!ONE_IN(1000))
			goto retry;
	}

	rec->nr = syscallnr;

	return true;
}

bool random_syscall(void)
{
	struct syscallrecord *rec;
	int ret = false;

	rec = &shm->syscall;

	fuzz_log("stats.op_count: %ld\n", shm->stats.op_count);

	if (set_syscall_nr(rec) == FAIL)
		return FAIL;

	// memset(rec->postbuffer, 0, POSTBUFFER_LEN);

	/* Generate arguments, print them out */
	generate_syscall_args(rec);

	//MNZ - TODO - use for logging
	output_syscall_prefix(rec);

	do_syscall(rec);

	//MNZ - TODO - use for logging
	output_syscall_postfix(rec);

	//MNZ - TODO - adapt this
	handle_syscall_ret(rec);

	ret = true;

	return ret;
}
