#include "shm.h"
#include "random.h"
#include "utils.h"
#include "logging.h"

#include <console.h>
#include <timer.h>
#include <adc.h>

unsigned int init_seed(void)
{
	unsigned int adc_channel = 0;
	uint16_t adc_value;

	if(adc_sample_sync (adc_channel, &adc_value) != TOCK_STATUSCODE_SUCCESS)
    	return 0;

	srand(adc_value);

	fuzz_log("Initial random seed: %u\n", adc_value);

	return adc_value;
}