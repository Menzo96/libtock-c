/*
 * Functions for actually doing the system calls.
 */
#include <stdio.h>
#include <string.h>
#include <tock.h>

#include "arch.h"
#include "tock-syscalls.h"
#include "random.h"
#include "sanitise.h"
#include "shm.h"
#include "syscall.h"
#include "tables.h"
#include "trinity.h"
#include "utils.h"
#include "runner.h"

void do_syscall(struct syscallrecord *rec)
{
	unsigned int call;

	syscall_return_t command_ret;
	subscribe_return_t subscribe_ret;
	allow_rw_return_t allow_rw_ret;
	allow_ro_return_t allow_ro_ret;
	memop_return_t memop_ret;

	call = rec->nr;
	runnerdata->stats.op_count++;

	rec->state = BEFORE;

	switch(call)
	{
		case TOCK_YIELD:
			yield();
			break;

		case TOCK_COMMAND:
			command_ret = command((uint32_t)rec->a1, (uint32_t)rec->a2, (int)rec->a3, (int)rec->a4);
			rec->retval = tock_command_return_novalue_to_returncode(command_ret);
			break;

		case TOCK_SUBSCRIBE:
			subscribe_ret = subscribe((uint32_t)rec->a1, (uint32_t)rec->a2, (subscribe_upcall*)(rec->a3), (void*)rec->a4);
			rec->retval = tock_status_to_returncode(subscribe_ret.status);
			break;

		case TOCK_ALLOW_RW:
			allow_rw_ret = allow_readwrite((uint32_t)rec->a1, (uint32_t)rec->a2, (void*)rec->a3, (size_t)rec->a4);
			rec->retval = tock_status_to_returncode(allow_rw_ret.status);
			break;

		case TOCK_ALLOW_RO:
			allow_ro_ret = allow_readonly((uint32_t)rec->a1, (uint32_t)rec->a2, (const void*)rec->a3, (size_t)rec->a4);
			rec->retval = tock_status_to_returncode(allow_ro_ret.status);
			break;

		case TOCK_MEMOP:
			memop_ret = memop((uint32_t)rec->a1, (int)rec->a2);
			rec->retval = tock_status_to_returncode(memop_ret.status);
			break;

		default:
			break;
	}
	rec->state = AFTER;
}

void handle_syscall_ret(struct syscallrecord *rec)
{
	struct syscallentry *entry;
	unsigned int call;

	call = rec->nr;
	entry = syscalls[call].entry;

	if (rec->retval < 0) {
		entry->failures++;
		runnerdata->stats.failures++;

	} else {
		entry->successes++;
		runnerdata->stats.successes++;
	}
	entry->attempted++;

	generic_free_arg(rec);
}
