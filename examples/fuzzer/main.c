#include <stdio.h>

#include <tock.h>

#include "arch.h"
#include "tock-syscalls.h"
#include "tables.h"
#include "shm.h"
#include "utils.h"
#include "trinity.h"
#include "random.h"
#include "logging.h"

unsigned int page_size;

int main(void) 
{
  int ret = EXIT_SUCCESS;

  page_size = 4000;

  syscalls_todo = 1000;

  //Select syscall table
  syscalls = SYSCALLS;
  max_nr_syscalls = ARRAY_SIZE(SYSCALLS);

  //Create and Init shm
  create_shm();

  init_shm();

  init_fuzz_log();

  //Some syscall table checks
  if (munge_tables() == false) {
    ret = EXIT_FAILURE;
    goto out;
  }

  //Init syscalls - basically call their init() function if active
  init_syscalls();

	while (shm->exit_reason == STILL_RUNNING) {

		ret = random_syscall();

    delay_ms(100);

		if (ret == FAIL)
			goto out;

		if (syscalls_todo) {
			if (shm->stats.op_count >= syscalls_todo) {
				shm->exit_reason = EXIT_REACHED_COUNT;
				goto out;
			}
		}
	}

out:

	exit(ret);
}

void panic(int reason)
{
	shm->exit_reason = reason;
}