#include <stdio.h>
#include <stdlib.h>

#include "arch.h"
#include "random.h"
#include "sanitise.h"
#include "shm.h"
#include "tables.h"
#include "utils.h"

struct address_arg get_writable_address(unsigned long size)
{
	struct address_arg addr_arg = {NULL, true};

	addr_arg.address = malloc(size);

	if(RAND_BOOL())
	{
		free(addr_arg.address);
		addr_arg.need_free = false;
	}

	return addr_arg;
}

struct address_arg get_non_null_address(void)
{
	unsigned long size = page_size;

	return get_writable_address(size);
}

struct address_arg get_address(void)
{
	struct address_arg addr_arg = {NULL, true};
	if (ONE_IN(100))
		return addr_arg;

	return get_non_null_address();
}

static bool is_arg_address(enum argtype argtype)
{
	if (argtype == ARG_ADDRESS)
		return true;
	if (argtype == ARG_NON_NULL_ADDRESS)
		return true;
	return false;
}

unsigned long find_previous_arg_address(struct syscallrecord *rec, unsigned int argnum)
{
	struct syscallentry *entry;
	unsigned long addr = 0;
	unsigned int call;

	call = rec->nr;
	entry = syscalls[call].entry;

	if (argnum > 1)
		if (is_arg_address(entry->arg1type) == true)
			addr = rec->a1;

	if (argnum > 2)
		if (is_arg_address(entry->arg2type) == true)
			addr = rec->a2;

	if (argnum > 3)
		if (is_arg_address(entry->arg3type) == true)
			addr = rec->a3;

	if (argnum > 4)
		if (is_arg_address(entry->arg4type) == true)
			addr = rec->a4;

	if (argnum > 5)
		if (is_arg_address(entry->arg5type) == true)
			addr = rec->a5;

	return addr;
}