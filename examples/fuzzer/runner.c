#include "runner.h"
#include "random.h"
#include "exit.h"

char runner_data_buf[RUNNER_BUF_SZ] __attribute__((aligned(RUNNER_BUF_SZ)));

runnerdata_t *runnerdata;

runnerdata_t* init_runner(void)
{
    memset(&runner_data_buf[0], 0, RUNNER_BUF_SZ);

    runnerdata_t *rd = (runnerdata_t *)(&runner_data_buf[0]);

    rd->state = RunnerInit;

	rd->pid = NO_PID;

    rd->seed = init_seed();

    rd->exit_reason = STILL_RUNNING;

    rd->stats.op_count = 0;

    return rd;
}