/*****************************************************************************
 * @file    sample_main.c
 * @author  Kshitij Mistry
 * @brief   Sample main function for scheduler module.
 *
 * This file contains the main function for a sample application that demonstrates
 * the usage of the scheduler module. It initializes the scheduler, registers tasks,
 * and runs a super loop to manage task execution. The main loop continuously updates
 * the scheduler and executes tasks that are ready to run. It also implements a dynamic
 * sleep mechanism to optimize CPU usage while waiting for the next task to be ready.
 *
 *****************************************************************************/

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "common_def.h"
#include "common_utils.h"
#include "scheduler.h"

/*****************************************************************************
 * MACROS
 *****************************************************************************/

/** Smallest timer tick used to schedule tasks in millisec */
#define	TIMER_TICK_RESOLUTION_IN_MS (10)

/** Smallest timer tick used to schedule tasks in nanosec
* @note Update this define as per changes in TIMER_TICK_RESOLUTION_IN_MS
*/
#define	TIMER_TICK_RESOLUTION_IN_NS (10000000LL)

/* Print warning debugs if we are running late by more than below value */
#define MAX_PERMITTED_DELAY (1000)

/*****************************************************************************
 * VARIABLES
 *****************************************************************************/

/** Daemon super loop run status */
static cBool gRunSuperLoopF = c_TRUE;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/
static void waitNextTick(void);

// TODO :
// Add enum to str for status code
// Add Task Timers feature (in same or separate file)

/*****************************************************************************
 * FUNCTION DEFINATIONS
 *****************************************************************************/
/**
 * @brief This is main function of Network Manager Daemon
 * @param argc - number of arguments
 * @param **argv - argument list
 * @return 0 if successfully exited else -1 on error occurs
 */
int main(int argc, char** argv)
{
    cStatus_e cStatus;
    cU32_t maxPermittedDelayMs = MAX_PERMITTED_DELAY;

    // Initialize task scheduler to manage tasks of main thread
    if (c_FALSE == Scheduler_Init(TIMER_TICK_RESOLUTION_IN_MS, &maxPermittedDelayMs))
    {
        EPRINT("failed to initialize scheduler");
        return (-1);
    }

    // cStatus = Scheduler_RegisterTask(TaskTimer_Tick, TIME_INTERVAL_100MS);
    // if (cStatus_SUCCESS != cStatus)
    // {
    //     EPRINT("failed to register timer task: [nmSts=%s]", ENUM_TO_STR_NMSTS(cStatus));
    //     // return (-1);
    // }

    DPRINT("scheduled tasks registered successfully");

    // Reset the Timer to measure the correct time from now...
    Scheduler_Reset();

    DPRINT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    DPRINT("START MAIN LOOP");
    DPRINT("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    // Run daemon super loop
    while (gRunSuperLoopF)
    {
        // Read the RTC continuously
        Scheduler_UpdateTick();

        // Execute the Queued Tasks
        Scheduler_ExecuteTasksReadyToRun();

        // Dynamic Sleep that can be min 10 ms
        Utils_SleepNanoSec(Scheduler_GetDynamicSleep());
    }

    // Deregister timer task
    // Scheduler_DeregisterTask(TaskTimer_Tick);

    return 0;
}

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
