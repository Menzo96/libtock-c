#pragma once

#define PAGE_OFFSET		0xC0000000
/*
 * Alternative possibilities for PAGE_OFFSET:
 * default 0x40000000 if VMSPLIT_1G
 * default 0x80000000 if VMSPLIT_2G
 */
#define TASK_SIZE		(PAGE_OFFSET - 0x01000000)
#define MODULE_ADDR		(PAGE_OFFSET - 0x00800000)
#define KERNEL_ADDR		(PAGE_OFFSET + 0x00008000)

#define PAGE_SHIFT		12

#define PAGE_MASK (~(page_size - 1))
#define PTE_FILE_MAX_BITS	29

extern unsigned int page_size;