#include "syscall.h"
#include "tock-syscalls.h"

struct syscalltable syscalls_tock[6] = {

/* 0 */	{ .entry = &syscal_yield },
		{ .entry = &syscall_command },
		{ .entry = &syscall_subscribe },
		{ .entry = &syscall_allow_readwrite },
/* 5 */	{ .entry = &syscall_allow_readonly },
		{ .entry = &syscall_memop }
};

struct syscallentry syscal_yield = {
	.name = "yield",
	.num_args = 0,
	// .sanitise = sanitise_yield,
	.flags = AVOID_SYSCALL, // No args to fuzz, confuses fuzzer
};

struct syscallentry syscall_command = {
	.name = "command",
	.num_args = 4,
	// .sanitise = sanitise_command,
	.arg1name = "driver",
	.arg1type = ARG_RANGE,
	.low1range = 0,     
	.hi1range = 16,     
	.arg2name = "command",
	.arg2type = ARG_UNDEFINED,
	.arg3name = "arg1",
	.arg3type = ARG_UNDEFINED,
    .arg4name = "arg2",
	.arg4type = ARG_UNDEFINED,
};

struct syscallentry syscall_subscribe = {
	.name = "subscribe",
	.num_args = 4,
	// .sanitise = sanitise_subscribe,
	.arg1name = "driver",
	.arg1type = ARG_RANGE,
	.low1range = 0,     
	.hi1range = 16,     
	.arg2name = "subscribe",
	.arg2type = ARG_UNDEFINED,
	.arg3name = "uc",
	.arg3type = ARG_ADDRESS,
    .arg4name = "userdata",
	.arg4type = ARG_ADDRESS,
};

struct syscallentry syscall_allow_readwrite = {
	.name = "allow_readwrite",
	.num_args = 4,
	// .sanitise = sanitise_allow_readwrite,
	.arg1name = "driver",
	.arg1type = ARG_RANGE,
	.low1range = 0,     
	.hi1range = 16,     
	.arg2name = "allow",
	.arg2type = ARG_UNDEFINED,
	.arg3name = "ptr",
	.arg3type = ARG_ADDRESS,
    .arg4name = "size",
	.arg4type = ARG_LEN,
};

struct syscallentry syscall_allow_readonly = {
	.name = "allow_readonly",
	.num_args = 4,
	// .sanitise = sanitise_allow_readonly,
	.arg1name = "driver",
	.arg1type = ARG_RANGE,
	.low1range = 0,     
	.hi1range = 16,     
	.arg2name = "allow",
	.arg2type = ARG_UNDEFINED,
	.arg3name = "ptr",
	.arg3type = ARG_ADDRESS,
    .arg4name = "size",
	.arg4type = ARG_LEN,
};

struct syscallentry syscall_memop = {
	.name = "memop",
	.num_args = 2,
	// .sanitise = sanitise_memop,
	.arg1name = "op_type",
	.arg1type = ARG_UNDEFINED,
	.arg2name = "arg1",
	.arg2type = ARG_UNDEFINED,
};