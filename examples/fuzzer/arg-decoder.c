/*
 * Routines to take a syscallrecord and turn it into an ascii representation.
 */

#include <stdio.h>
#include "arch.h"	//PAGE_MASK
#include "arg-decoder.h"
#include "shm.h"
#include "syscall.h"
#include "tables.h"
#include "utils.h"
#include "logging.h"

#include <tock.h>

static char * decode_argtype(char *sptr, unsigned long reg, enum argtype type)
{
	switch (type) {

	case ARG_ADDRESS:
	case ARG_NON_NULL_ADDRESS:
		sptr += snprintf(sptr, PREBUFFER_LEN, "0x%lx", reg);
		break;

	case ARG_UNDEFINED:
	case ARG_LEN:
	case ARG_RANGE:
		if (((long) reg < -16384) || ((long) reg > 16384)) {
			/* Print everything outside -16384 and 16384 as hex. */
			sptr += snprintf(sptr, PREBUFFER_LEN, "0x%lx", reg);
		} else {
			/* Print everything else as signed decimal. */
			sptr += snprintf(sptr, PREBUFFER_LEN, "%ld", (long) reg);
		}
		break;

	default:
		break;
	}

	return sptr;
}

static char * render_arg(struct syscallrecord *rec, char *sptr, unsigned int argnum, struct syscallentry *entry)
{
	const char *name = NULL;
	unsigned long reg = 0;
	enum argtype type = 0;

	switch (argnum) {
	case 1:	type = entry->arg1type;
		name = entry->arg1name;
		reg = rec->a1;
		break;
	case 2:	type = entry->arg2type;
		name = entry->arg2name;
		reg = rec->a2;
		break;
	case 3:	type = entry->arg3type;
		name = entry->arg3name;
		reg = rec->a3;
		break;
	case 4:	type = entry->arg4type;
		name = entry->arg4name;
		reg = rec->a4;
		break;
	case 5:	type = entry->arg5type;
		name = entry->arg5name;
		reg = rec->a5;
		break;
	case 6:	type = entry->arg6type;
		name = entry->arg6name;
		reg = rec->a6;
		break;
	}

	if (argnum != 1)
		sptr += snprintf(sptr, PREBUFFER_LEN, ", ");

	sptr += snprintf(sptr, PREBUFFER_LEN, "%s=", name);

	sptr = decode_argtype(sptr, reg, type);

	if (entry->decode != NULL) {
		char *str;

		str = entry->decode(rec, argnum);
		if (str != NULL) {
			sptr += snprintf(sptr, PREBUFFER_LEN, "%s", str);
			free(str);
		}
	}

	return sptr;
}

/*
 * Used from output_syscall_prefix, and also from postmortem dumper
 */
static unsigned int render_syscall_prefix(struct syscallrecord *rec, char *bufferstart)
{
	struct syscallentry *entry;
	char *sptr = bufferstart;
	unsigned int i;
	unsigned int syscallnr;

	syscallnr = rec->nr;
	entry = get_syscall_entry(syscallnr);

	sptr += snprintf(sptr, PREBUFFER_LEN, "%s(", entry->name);

	for_each_arg(entry, i) {
		sptr = render_arg(rec, sptr, i, entry);
	}

	sptr += snprintf(sptr, PREBUFFER_LEN, ") \n");

	return sptr - bufferstart;
}

static unsigned int render_syscall_postfix(struct syscallrecord *rec, char *bufferstart)
{
	char *sptr = bufferstart;

	sptr += snprintf(sptr, PREBUFFER_LEN, "\t= %ld (%s)", rec->retval, tock_strrcode(rec->retval));
	sptr += snprintf(sptr, PREBUFFER_LEN, "\n");

	return sptr - bufferstart;
}

/* These next two functions are always called from child_random_syscalls() by a fuzzing child.
 * They render the buffer, and output it to stdout.
 * Other contexts (like post-mortem) directly use the buffers.
 */
void output_syscall_prefix(struct syscallrecord *rec)
{
	static char *buffer = NULL;
	unsigned int len;

	if (buffer == NULL)
		buffer = malloc(PREBUFFER_LEN);

	len = render_syscall_prefix(rec, buffer);

	/* copy child-local buffer to shm, and zero out trailing bytes */
	memcpy(rec->prebuffer, buffer, len);
	memset(rec->prebuffer + len, 0, PREBUFFER_LEN - len);

	fuzz_log(rec->prebuffer);
}

void output_syscall_postfix(struct syscallrecord *rec)
{
	static char *buffer = NULL;
	unsigned int len;

	if (buffer == NULL)
		buffer = malloc(POSTBUFFER_LEN);

	len = render_syscall_postfix(rec, buffer);

	/* copy child-local buffer to shm, and zero out trailing bytes */
	memcpy(rec->postbuffer, buffer, len);
	memset(rec->postbuffer + len, 0, POSTBUFFER_LEN - len);
	
	fuzz_log(rec->postbuffer);
}
