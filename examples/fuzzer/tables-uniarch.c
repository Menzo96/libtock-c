/*
 * Functions for handling the system call tables.
 * These functions are only used by architectures that have either 32 or 64 bit syscalls, but not both.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "arch.h"
#include "syscall.h"
#include "random.h"
#include "shm.h"
#include "tables.h"
#include "logging.h"

const struct syscalltable *syscalls;

unsigned int max_nr_syscalls;

void activate_syscall(unsigned int calln)
{
	activate_syscall_in_table(calln, &shm->active_syscalls->nr, syscalls, shm->active_syscalls->sysc);
}

//TODO - may need to addapt for runner data
//In runner it crashes because runners don't have shm initialized
void deactivate_syscall_uniarch(unsigned int calln)
{
	deactivate_syscall_in_table(calln, &shm->active_syscalls->nr, syscalls, shm->active_syscalls->sysc);
}

void toggle_syscall_n(int calln, bool state, const char *arg, const char *arg_name)
{
	struct syscallentry *entry;

	if (calln == -1) {
		fuzz_log("No idea what syscall (%s) is.\n", arg);
		exit(EXIT_FAILURE);
	}

	validate_specific_syscall(syscalls, calln);

	entry = syscalls[calln].entry;

	if (state == true) {
		entry->flags |= ACTIVE;
		activate_syscall(calln);
	} else {
		entry->flags |= TO_BE_DEACTIVATED;
	}

	fuzz_log("Marking syscall %s (%d) as to be %sabled.\n",
		arg_name, calln,
		state ? "en" : "dis");
}


void enable_random_syscalls_uniarch(void)
{
	unsigned int call;
	struct syscallentry *entry;

retry:
	call = rnd() % max_nr_syscalls;
	entry = syscalls[call].entry;

	if (validate_specific_syscall_silent(syscalls, call) == false)
		goto retry;

	/* if we've set this to be disabled, don't enable it! */
	if (entry->flags & TO_BE_DEACTIVATED)
		goto retry;

	toggle_syscall_n(call, true, entry->name, entry->name);
}


int setup_syscall_group_uniarch(unsigned int group)
{
	unsigned int i;

	for_each_syscall(i) {
		if (syscalls[i].entry->group == group)
			activate_syscall(i);
	}

	if (shm->active_syscalls->nr == 0) {
		fuzz_log("No syscalls found in group\n");
		return false;
	} else {
		fuzz_log("Found %d syscalls in group\n", shm->active_syscalls->nr);
	}

	return true;
}

void mark_all_syscalls_active_uniarch(void)
{
	unsigned int i;

	for_each_syscall(i) {
		struct syscallentry *entry = syscalls[i].entry;
		if (entry == NULL)
			continue;

		entry->flags |= ACTIVE;
		activate_syscall(i);
	}
}

void init_syscalls_uniarch(void)
{
	unsigned int i;

	for_each_syscall(i) {
		struct syscallentry *entry = syscalls[i].entry;
		if (entry == NULL)
			continue;

		if (entry->flags & ACTIVE)
			if (entry->init)
				entry->init();
	}
}

void deactivate_disabled_syscalls_uniarch(void)
{
	unsigned int i;

	for_each_syscall(i) {
		struct syscallentry *entry = syscalls[i].entry;

		if (entry == NULL)
			continue;

		if (entry->flags & TO_BE_DEACTIVATED) {
			entry->flags &= ~(ACTIVE|TO_BE_DEACTIVATED);
			deactivate_syscall_uniarch(i);
			fuzz_log("Marked syscall %s (%d) as deactivated.\n",
				entry->name, entry->number);
		}
	}
}

void dump_syscall_tables_uniarch(void)
{
	unsigned int i;

	fuzz_log("syscalls: %d\n", max_nr_syscalls);

	for_each_syscall(i) {
		struct syscallentry *entry = syscalls[i].entry;

		if (entry == NULL)
			continue;

		fuzz_log("entrypoint %d %s : ", entry->number, entry->name);
		show_state(entry->flags & ACTIVE);
		if (entry->flags & AVOID_SYSCALL)
			fuzz_log(" AVOID");
		fuzz_log("\n");
	}
}

void display_enabled_syscalls_uniarch(void)
{
        unsigned int i;

	for_each_syscall(i) {
		struct syscallentry *entry = syscalls[i].entry;

		if (entry == NULL)
			continue;

		if (entry->flags & ACTIVE)
			fuzz_log("syscall %d:%s enabled.\n", i, entry->name);
	}
}
