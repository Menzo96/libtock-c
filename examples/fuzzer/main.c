#include <stdio.h>

#include <tock.h>
#include <ipc.h>

#include "arch.h"
#include "tock-syscalls.h"
#include "tables.h"
#include "shm.h"
#include "utils.h"
#include "trinity.h"
#include "random.h"
#include "logging.h"
#include "runner.h"

#define FUZZER_SUPERVISOR 1

#define FUZZER_RUNNER (1 - FUZZER_SUPERVISOR)

unsigned int page_size;
unsigned int max_runners;

#if FUZZER_SUPERVISOR
//this callback will be called when the client notifies the supervisor
static void supervisor_callback(int pid,
                                __attribute__ ((unused)) int len,
                                int buf,
                                __attribute__ ((unused)) void *ud) {
   
  if (buf == 0) {
    printf("supervisor: supervisor_callback buf\n");
    return;
  }

  //cast the buffer from the runner to the runner_data structure
  //TODO - this must be done one time
  //Insert states (Init state, Run state etc)
  runnerdata_t *runner      = (runnerdata_t *)buf;

  printf("supervisor: supervisor_callback in\n");

  switch(runner->state)
  {
    case RunnerInit:

      printf("supervisor: supervisor_callback RunnerInit\n");
      if (shm->nr_runners >= max_runners)
      {
        fuzz_log("Runners limit reached\n");
        return;
      }

      //save the runner data structure in the supervisor's array
      shm->runners[shm->nr_runners] = runner;
      shm->nr_runners++;
      runner->pid = pid;
      
      //share the active callbacks with the client
      ipc_share(pid, shm->active_syscalls, SYSCALLS_BUF_SZ);
      printf("supervisor: ipc_share\n");

      //notify the client to start fuzzing
      ipc_notify_client(pid);

      fuzz_log("Runner-%d has started\n", pid);

      break;

    case RunnerStart:
      printf("supervisor: supervisor_callback RunnerStart\n");
      break;

    case RunnerEnd:
      printf("supervisor: supervisor_callback RunnerEnd\n");
      ipc_notify_client(pid);

      fuzz_log("Runner-%d finished\n", pid);
      
      break;

    case RunnerCleanup:
      printf("supervisor: supervisor_callback RunnerCleanup\n");
      break;

    default:
      break;

  }

  printf("supervisor: supervisor_callback out\n");

}

#endif

#if FUZZER_RUNNER

static bool done = false;

static void continue_callback(__attribute__ ((unused)) int pid,
                              __attribute__ ((unused)) int len,
                              int buf,
                              void *ud) {
  if (buf == 0) {
    printf("runner: continue_callback buf\n");
    return;
  }

  if (ud == NULL) {
    printf("runner: continue_callback ud\n");
    return;
  }

  runnerdata_t *runner                  = (runnerdata_t *)ud;
  activesyscalls_t *active_syscalls     = (activesyscalls_t *)buf;

  printf("runner: continue_callback in\n");

  switch(runner->state)
  {
    case RunnerInit:
      //get shared buffer - active systemcalls 
      runner->active_syscalls = active_syscalls;
      printf("runner: got active syscalls\n");
      break;
  }

  done = true;
  printf("runner: continue_callback out\n");
}

/** \brief Notify the test supervisor, and await approval to continue the test.
 */
static void sync_with_supervisor(size_t svc) {
  printf("runner: sync_with_supervisor in\n");
  done = false;
  ipc_notify_service(svc);
  printf("runner: sync_with_supervisor ipc_notify_service\n");
  yield_for(&done);
  printf("runner: sync_with_supervisor out\n");
}

#endif

int main(void) 
{

  char supervisor_name[] = "org.tockos.fuzzer.supervisor";
  page_size = 10;
  int ret = EXIT_SUCCESS;

  //Select syscall table
  syscalls = SYSCALLS;
  max_nr_syscalls = ARRAY_SIZE(SYSCALLS);


#if  FUZZER_SUPERVISOR

  max_runners = 5;
  
  //Create and Init shm
  create_shm();

  init_shm();

  init_fuzz_log();

  //Some syscall table checks
  if (munge_tables() == false) {
    ret = EXIT_FAILURE;
    // goto out;
  }

  //Init syscalls - basically call their init() function if active
  init_syscalls();

  ipc_register_service_callback(supervisor_name,
                                supervisor_callback, NULL);

  printf("supervisor: ipc_register_service_callback\n");

  // out:
	//   exit(ret);


#elif   FUZZER_RUNNER

  size_t supervisor_svc;
  int err;

  syscalls_todo = 10;

  init_fuzz_log();

  runnerdata = init_runner();

  delay_ms(1000);

  err = ipc_discover(supervisor_name, &supervisor_svc);
  if (err < 0) 
  {
    return;
  }
  printf("runner: ipc_discover\n");

  ipc_register_client_callback(supervisor_svc, continue_callback, runnerdata);

  // ipc_register_client_callback(supervisor_svc, continue_callback, NULL);

  printf("runner: ipc_register_client_callback\n");

  ipc_share(supervisor_svc, runnerdata, RUNNER_BUF_SZ);

  // ipc_share(supervisor_svc, _led_buf, 64);

  printf("runner: ipc_share\n");

  runnerdata->state = RunnerInit;
  sync_with_supervisor(supervisor_svc);

  printf("runner: sync_with_supervisor\n");

  fuzz_log("I, runner-%d will start now\n", runnerdata->pid);

  while (runnerdata->exit_reason == STILL_RUNNING) {

    printf("runner: random_syscall before\n");
    ret = random_syscall();
    printf("runner: random_syscall after\n");

    delay_ms(100);

    if (ret == FAIL)
      goto out;

    if (syscalls_todo) {
      if (runnerdata->stats.op_count >= syscalls_todo) {
        runnerdata->exit_reason = EXIT_REACHED_COUNT;
        goto out;
      }
    }
  }

out:
  runnerdata->state = RunnerEnd;
  fuzz_log("I, runner-%d have finished now\n", runnerdata->pid);
  sync_with_supervisor(supervisor_svc);
	exit(ret);

#endif


}

void panic(int reason)
{
	runnerdata->exit_reason = reason;
}