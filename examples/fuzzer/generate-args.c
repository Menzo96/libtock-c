#include <stdio.h>
#include <stdlib.h>
#include "arch.h"
#include "random.h"
#include "sanitise.h"
#include "shm.h"
#include "syscall.h"
#include "tables.h"
#include "trinity.h"	// num_online_cpus
#include "logging.h"

static unsigned long handle_arg_address(struct syscallrecord *rec, unsigned int argnum)
{
	unsigned long addr = 0;
	struct address_arg addr_arg;

	if ((argnum == 1) || (RAND_BOOL()))
	{
		addr_arg = get_address();
		rec->need_free[argnum-1] = addr_arg.need_free;
		return (unsigned long)addr_arg.address;
	}

	/* Half the time, we look to see if earlier args were also ARG_ADDRESS,
	 * and munge that instead of returning a new one from get_address() */

	addr = find_previous_arg_address(rec, argnum);

	switch (rnd() % 4){
	case 0:	
		break;	/* return unmodified */
	case 1:	
		addr++;
		break;
	case 2:	
		addr+= sizeof(int);
		break;
	case 3:	
		addr+= sizeof(long);
		break;
	}

	return addr;
}

static unsigned long handle_arg_range(struct syscallentry *entry, unsigned int argnum)
{
	unsigned long i;
	unsigned long low = 0, high = 0;

	switch (argnum) {
	case 1:	
		low = entry->low1range;
		high = entry->hi1range;
		break;
	case 2:	
		low = entry->low2range;
		high = entry->hi2range;
		break;
	case 3:	
		low = entry->low3range;
		high = entry->hi3range;
		break;
	case 4:	
		low = entry->low4range;
		high = entry->hi4range;
		break;
	case 5:	
		low = entry->low5range;
		high = entry->hi5range;
		break;
	case 6:	
		low = entry->low6range;
		high = entry->hi6range;
		break;
	}

	if (high == 0) {
		fuzz_log("%s forgets to set hirange!\n", entry->name);
		// BUG("Fix syscall definition!\n");
	}

	i = (unsigned long) rand64() % high;
	if (i < low) {
		i += low;
		i &= high;
	}
	return i;
}

enum argtype get_argtype(struct syscallentry *entry, unsigned int argnum)
{
	enum argtype argtype = 0;

	switch (argnum) {
	case 1:	argtype = entry->arg1type;
		break;
	case 2:	argtype = entry->arg2type;
		break;
	case 3:	argtype = entry->arg3type;
		break;
	case 4:	argtype = entry->arg4type;
		break;
	case 5:	argtype = entry->arg5type;
		break;
	case 6:	argtype = entry->arg6type;
		break;
	}

	return argtype;
}

unsigned long get_argval(struct syscallrecord *rec, unsigned int argnum)
{
	switch (argnum) {
	case 1:	return rec->a1;
	case 2:	return rec->a2;
	case 3:	return rec->a3;
	case 4:	return rec->a4;
	case 5:	return rec->a5;
	case 6:	return rec->a6;
	}
	return 0;
}

static unsigned long fill_arg(struct syscallrecord *rec, unsigned int argnum)
{
	struct syscallentry *entry;
	unsigned int call;
	enum argtype argtype;
	struct address_arg addr_arg;

	call = rec->nr;
	entry = syscalls[call].entry;

	if (argnum > entry->num_args)
		return 0;

	argtype = get_argtype(entry, argnum);

	switch (argtype) {
	case ARG_UNDEFINED:
		if (RAND_BOOL())
			return (unsigned long) rand64();
		else
		{
			addr_arg = get_writable_address(page_size);
			rec->need_free[argnum - 1] = addr_arg.need_free;
			return (unsigned long)addr_arg.address;
		}	

	case ARG_LEN:
		return (unsigned long) get_len();

	case ARG_ADDRESS:
		return handle_arg_address(rec, argnum);

	case ARG_NON_NULL_ADDRESS:
		{
			addr_arg = get_non_null_address();
			rec->need_free[argnum - 1] = addr_arg.need_free;
			return (unsigned long)addr_arg.address;
		}

	case ARG_RANGE:
		return handle_arg_range(entry, argnum);

	default:
		return 0;
	}
}

void generic_sanitise(struct syscallrecord *rec)
{
	struct syscallentry *entry;
	unsigned int call;

	call = rec->nr;
	entry = syscalls[call].entry;

	if (entry->arg1type != 0)
		rec->a1 = fill_arg(rec, 1);
	if (entry->arg2type != 0)
		rec->a2 = fill_arg(rec, 2);
	if (entry->arg3type != 0)
		rec->a3 = fill_arg(rec, 3);
	if (entry->arg4type != 0)
		rec->a4 = fill_arg(rec, 4);
	if (entry->arg5type != 0)
		rec->a5 = fill_arg(rec, 5);
	if (entry->arg6type != 0)
		rec->a6 = fill_arg(rec, 6);
}

void generic_free_arg(struct syscallrecord *rec)
{
	struct syscallentry *entry;
	unsigned int i, call;

	call = rec->nr;

	entry = syscalls[call].entry;

	for_each_arg(entry, i) {
		if(rec->need_free[i-1])
		{
			rec->need_free[i-1] = false;
			free((void *) get_argval(rec, i));
		}	
	}
}

void generate_syscall_args(struct syscallrecord *rec)
{
	struct syscallentry *entry;

	entry = syscalls[rec->nr].entry;

	if(entry == NULL)
	{
		printf("entry is null\n");
	}

	rec->state = PREP;
	if (entry->arg1type == ARG_UNDEFINED)
		rec->a1 = (unsigned long) rand64();
	if (entry->arg2type == ARG_UNDEFINED)
		rec->a2 = (unsigned long) rand64();
	if (entry->arg3type == ARG_UNDEFINED)
		rec->a3 = (unsigned long) rand64();
	if (entry->arg4type == ARG_UNDEFINED)
		rec->a4 = (unsigned long) rand64();
	if (entry->arg5type == ARG_UNDEFINED)
		rec->a5 = (unsigned long) rand64();
	if (entry->arg6type == ARG_UNDEFINED)
		rec->a6 = (unsigned long) rand64();

	generic_sanitise(rec);
	if (entry->sanitise)
		entry->sanitise(rec);

}
