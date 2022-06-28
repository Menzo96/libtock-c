/*
 * Functions for handling the system call tables.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "syscall.h"
#include "shm.h"
#include "tables.h"
#include "utils.h"	// ARRAY_SIZE
#include "tock-syscalls.h"
#include "logging.h"

#include <console.h>

unsigned long syscalls_todo = 0;

int search_syscall_table(const struct syscalltable *table, unsigned int nr_syscalls, const char *arg)
{
	unsigned int i;

	/* search by name */
	for (i = 0; i < nr_syscalls; i++) {
		if (table[i].entry == NULL)
			continue;

		if (strcmp(arg, table[i].entry->name) == 0) {
			fuzz_log("Found %s at %u\n", table[i].entry->name, i);
			return i;
		}
	}

	return -1;
}

void validate_specific_syscall(const struct syscalltable *table, int call)
{
	struct syscallentry *entry;

	if (call == -1)
		return;

	entry = table[call].entry;
	if (entry == NULL)
		return;

	if (entry->flags & AVOID_SYSCALL)
		fuzz_log("%s is marked as AVOID. Skipping\n", entry->name);

	if (entry->flags & NI_SYSCALL)
		fuzz_log("%s is NI_SYSCALL. Skipping\n", entry->name);
}

int validate_specific_syscall_silent(const struct syscalltable *table, int call)
{
	struct syscallentry *entry;

	if (call == -1)
		return false;

	entry = table[call].entry;
	if (entry == NULL)
		return false;

	if (entry->flags & AVOID_SYSCALL)
		return false;

	if (entry->flags & NI_SYSCALL)
		return false;

	return true;
}

void activate_syscall_in_table(unsigned int calln, unsigned int *nr_active, const struct syscalltable *table, int *active_syscall)
{
	struct syscallentry *entry = table[calln].entry;

	//Check if the call is activated already, and activate it only if needed
	if (entry->active_number == 0) {
		//Sanity check
		if ((*nr_active + 1) > MAX_NR_SYSCALL) {
			fuzz_log("[tables] MAX_NR_SYSCALL needs to be increased. More syscalls than active table can fit.\n");
			exit(EXIT_FAILURE);
		}

		//save the call no
		active_syscall[*nr_active] = calln + 1;
		(*nr_active) += 1;
		entry->active_number = *nr_active;
	}
}

void deactivate_syscall_in_table(unsigned int calln, unsigned int *nr_active, const struct syscalltable *table, int *active_syscall)
{
	struct syscallentry *entry;

	entry = table[calln].entry;

	//Check if the call is activated already, and deactivate it only if needed
	if ((entry->active_number != 0) && (*nr_active > 0)) {
		unsigned int i;

		for (i = entry->active_number - 1; i < *nr_active - 1; i++) {
			active_syscall[i] = active_syscall[i + 1];
			table[active_syscall[i] - 1].entry->active_number = i + 1;
		}
		//The last step is to erase the last item.
		active_syscall[*nr_active - 1] = 0;
		(*nr_active) -= 1;
		entry->active_number = 0;
	}
}

void deactivate_syscall(unsigned int call)
{
	deactivate_syscall_uniarch(call);
}

void count_syscalls_enabled(void)
{

	fuzz_log("Enabled %d syscalls. Disabled %d syscalls.\n",
		shm->nr_active_syscalls, max_nr_syscalls - shm->nr_active_syscalls);
	
}

void init_syscalls(void)
{
	init_syscalls_uniarch();
}

bool no_syscalls_enabled(void)
{
	unsigned int total;

	total = shm->nr_active_syscalls;

	if (total == 0)
		return true;
	else
		return false;
}

/* Make sure there's at least one syscall enabled. */
int validate_syscall_tables(void)
{
	if (shm->nr_active_syscalls == 0)
		return false;
	else
		return true;
}

static void check_syscall(struct syscallentry *entry)
{
	/* check that we have a name set. */
#define CHECK(NUMARGS, ARGNUM, ARGTYPE, ARGNAME)	\
	if (entry == NULL)								\
		return;										\
	if (entry->num_args > 0) {						\
		if (entry->num_args > NUMARGS) {			\
			if (entry->ARGNAME == NULL)  {			\
				fuzz_log("arg %d of %s has no name\n", ARGNUM, entry->name);	\
				exit(EXIT_FAILURE);					\
			}										\
		}											\
	}												\

	CHECK(0, 1, arg1type, arg1name);
	CHECK(1, 2, arg2type, arg2name);
	CHECK(2, 3, arg3type, arg3name);
	CHECK(3, 4, arg4type, arg4name);
	CHECK(4, 5, arg5type, arg5name);
	CHECK(5, 6, arg6type, arg6name);
}

static void sanity_check(const struct syscalltable *table, unsigned int nr)
{
	unsigned int i;

	for (i = 0; i < nr; i++)
		check_syscall(table[i].entry);
}

void sanity_check_tables(void)
{
	sanity_check(syscalls, max_nr_syscalls);
}

void mark_all_syscalls_active(void)
{
	fuzz_log("Marking all syscalls as enabled.\n");

	mark_all_syscalls_active_uniarch();
}

void check_user_specified_arch(const char *arg, char **arg_name, bool *only_64bit, bool *only_32bit)
{
	//Check if the arch is specified
	char *arg_arch = strstr(arg,",");

	if (arg_arch  != NULL) {
		unsigned long size = 0;

		size = (unsigned long)arg_arch - (unsigned long)arg;
		*arg_name = malloc(size + 1);
		if (*arg_name == NULL)
			exit(EXIT_FAILURE);
		(*arg_name)[size] = 0;
		memcpy(*arg_name, arg, size);

		//identify architecture
		if ((only_64bit != NULL) && (only_32bit != NULL)) {
			if ((strcmp(arg_arch + 1, "64") == 0)) {
				*only_64bit = true;
				*only_32bit = false;
			} else if ((strcmp(arg_arch + 1,"32") == 0)) {
				*only_64bit = false;
				*only_32bit = true;
			} else {
				fuzz_log("Unknown bit width (%s). Choose 32, or 64.\n", arg);
				exit(EXIT_FAILURE);
			}
		}
	} else {
		*arg_name = (char*)arg;//castaway const.
	}
}

void clear_check_user_specified_arch(const char *arg, char **arg_name)
{
	//Release memory only if we have allocated it
	if (((char *)arg) != *arg_name) {
		free(*arg_name);
		*arg_name = NULL;
	}
}

void toggle_syscall(const char *arg, bool state)
{
	int specific_syscall = 0;
	char * arg_name = NULL;

	check_user_specified_arch(arg, &arg_name, NULL, NULL); //We do not care about arch here, just to get rid of arg flags.

	specific_syscall = search_syscall_table(syscalls, max_nr_syscalls, arg_name);
	if (specific_syscall == -1) {
		fuzz_log("No idea what syscall (%s) is.\n", arg);
		goto out;
	}

	toggle_syscall_n(specific_syscall, state, arg, arg_name);

out:
	clear_check_user_specified_arch(arg, &arg_name);
}

void deactivate_disabled_syscalls(void)
{
	fuzz_log("Disabling syscalls marked as disabled by command line options\n");

	deactivate_disabled_syscalls_uniarch();
}

void show_state(unsigned int state)
{
	if (state)
		fuzz_log("Enabled");
	else
		fuzz_log("Disabled");
}

void dump_syscall_tables(void)
{
	dump_syscall_tables_uniarch();
}

static void show_unannotated_uniarch(void)
{
}

void show_unannotated_args(void)
{
	show_unannotated_uniarch();
}

int setup_syscall_group(unsigned int group)
{
	return setup_syscall_group_uniarch(group);
}

const char * print_syscall_name(unsigned int callno)
{
	const struct syscalltable *table;
	unsigned int max;

	max = max_nr_syscalls;
	table = syscalls;

	if (callno >= max) {
		fuzz_log("Bogus syscall number in %s (%u)\n", __func__, callno);
		return "invalid-syscall";
	}

	return table[callno].entry->name;
}

void display_enabled_syscalls(void)
{
	display_enabled_syscalls_uniarch();
}

/* By default, all syscall entries will be disabled.
 * If we didn't pass -c, -x, -r, or -g then mark all syscalls active.
 */
static void decide_if_active(void)
{
	mark_all_syscalls_active();
}

/* This is run *after* we've parsed params */
int munge_tables(void)
{
	decide_if_active();

	sanity_check_tables();

	count_syscalls_enabled();

	if (validate_syscall_tables() == false) {
		fuzz_log("No syscalls were enabled!\n");
		return false;
	}

	return true;
}

/*
 * return a ptr to a syscall table entry
 *
 * Takes the actual syscall number from the syscallrecord struct as an arg.
 */
struct syscallentry * get_syscall_entry(unsigned int callno)
{
	return syscalls[callno].entry;
}