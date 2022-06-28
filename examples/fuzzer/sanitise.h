#pragma once

#include <stdbool.h>
#include "syscall.h"

struct address_arg
{
    void* address;
    bool need_free;
};

void generic_sanitise(struct syscallrecord *rec);
void generic_free_arg(struct syscallrecord *rec);

unsigned long get_interesting_value(void);

struct address_arg get_address(void);
struct address_arg get_non_null_address(void);
struct address_arg get_writable_address(unsigned long size);
unsigned long find_previous_arg_address(struct syscallrecord *rec, unsigned int argnum);
struct iovec * alloc_iovec(unsigned int num);
unsigned long get_len(void);
unsigned int get_pid(void);

void gen_unicode_page(char *page);

enum argtype get_argtype(struct syscallentry *entry, unsigned int argnum);
unsigned long get_argval(struct syscallrecord *rec, unsigned int argnum);
void generate_syscall_args(struct syscallrecord *rec);
