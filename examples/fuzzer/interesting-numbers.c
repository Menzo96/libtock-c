#include <stdio.h>
#include "arch.h"
#include "random.h"
#include "sanitise.h"

static unsigned int plus_minus_two(unsigned int num)
{
	/* Now munge it for off-by-ones. */
	switch (rnd() % 4) {
	case 0:	
		num -= 2;
		break;
	case 1:	
		num -= 1;
		break;
	case 2:	
		num += 1;
		break;
	case 3:	
		num += 2;
		break;
	}
	return num;
}

static unsigned char get_interesting_8bit_value(void)
{
	switch (rnd() % 5) {
	case 0: 
		return 1;					// one
	case 1: 
		return 0xff;				// max
	case 2: 
		return 1UL << (rnd() & 7);	// 2^n (1 -> 128)
	case 3: 
		return RAND_BYTE();			// 0 -> 0xff
	default: 
		return 0;					// zero
	}
}

static unsigned short get_interesting_16bit_value(void)
{
	switch (rnd() % 4) {
	case 0: 
		return 0x8000 >> (rnd() & 7);		// 2^n (0x100 -> 0x8000)
	case 1: 
		return rnd() & 0xffff;				// 0 -> 0xffff
	case 2: 
		return 0xff00 | RAND_BYTE();		// 0xff00 -> 0xffff
	default: 
		return 0xffff;						// max
	}
}

static unsigned int get_interesting_32bit_value(void)
{
	switch (rnd() % 10) {
	case 0: 
		return 0x80000000 >> (rnd() & 0x1f);	// 2^n (1 -> 0x10000)
	case 1: 
		return rnd();							// 0 -> RAND_MAX (likely 0x7fffffff)
	case 2: 
		return (unsigned int) 0xff << (4 * (rnd() % 7));
	case 3: 
		return 0xffff0000;
	case 4: 
		return 0xffffe000;
	case 5: 
		return 0xffffff00 | RAND_BYTE();
	case 6: 
		return 0xffffffff - page_size;
	case 7: 
		return page_size;
	case 8: 
		return page_size * ((rnd() % (0xffffffff/page_size)) + 1);
	default: 
		return 0xffffffff;						// max
	}
}

unsigned long get_interesting_value(void)
{
	unsigned long low = 0;

	switch (rnd() % 3) {
	case 0:	
		low = get_interesting_8bit_value();
		break;
	case 1:	
		low = get_interesting_16bit_value();
		break;
	case 2: 
		low = get_interesting_32bit_value();
		break;
	}

	low = (rnd() & 0xf) ? low : plus_minus_two(low);	// 1 in 16 call plus_minus_two

	if (RAND_BOOL()) {	// FIXME: This should likely be less aggressive than 50/50
		switch (rnd() % 11) {
		case 0: 
			return 0x0000000100000000UL | low;
		case 1: 
			return 0x7fffffff00000000UL | low;
		case 2: 
			return 0x8000000000000000UL | low;
		case 3: 
			return 0xffffffff00000000UL | low;
		case 4: 
			return 0xffffffffffffff00UL | RAND_BYTE();
		case 5: 
			return 0xffffffffffffffffUL - page_size;
		case 6: 
			return PAGE_OFFSET | (low << 4);
		case 7: 
			return KERNEL_ADDR | (low & 0xffffff);
		case 8: 
			return MODULE_ADDR | (low & 0xffffff);
		case 9: 
			return low;
		case 10: 
			return (unsigned long)(low << 16);
		}
	}

	return low;
}
