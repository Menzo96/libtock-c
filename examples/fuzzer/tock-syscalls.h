#pragma once

/*
 * derived from libtock/tock.h
 */

#include "sanitise.h"
#include "syscall.h"

#define SYSCALLS syscalls_tock

enum tock_syscall_num {
	TOCK_YIELD,
	TOCK_COMMAND,
	TOCK_SUBSCRIBE,
	TOCK_ALLOW_RW,
	TOCK_ALLOW_RO,
	TOCK_MEMOP,
};

extern struct syscalltable syscalls_tock[6];

extern struct syscallentry syscal_yield;
extern struct syscallentry syscall_command;
extern struct syscallentry syscall_subscribe;
extern struct syscallentry syscall_allow_readwrite;
extern struct syscallentry syscall_allow_readonly;
extern struct syscallentry syscall_memop;