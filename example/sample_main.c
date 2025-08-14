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

/** Minimum sleep of application super loop in ns */
#define	BASE_SLEEP_TIME_IN_NS       (1000000LL)

/** Smallest timer tick used to schedule tasks in millisec */
#define	TIMER_TICK_RESOLUTION_IN_MS (10)

/** Smallest timer tick used to schedule tasks in nanosec
* @note Update this define as per changes in TIMER_TICK_RESOLUTION_IN_MS
*/
#define	TIMER_TICK_RESOLUTION_IN_NS (10000000LL)

/** Maximum acceptable delay count for application after going into sleep.
    If the sleep system call is delayed more then the below mentioned value then
    application will print the Warning log indicating the same.
    The count is kept as 100 considering the minimum timeout value for Heart-beat.
*/
#define MAX_SLEEP_DELAY_CNT_IN_MS   (1000)

/*****************************************************************************
 * VARIABLES
 *****************************************************************************/

/** Daemon super loop run status */
static cBool gRunSuperLoopF = c_TRUE;

/** Time reference from beginning of super loop */
static cI64_t gBeginTime = 0;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/
static void waitNextTick(void);

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
    cStatus_e nmSts;

    // Initialize task scheduler to manage tasks of main thread
    if (c_FALSE == Scheduler_Init(TIMER_TICK_RESOLUTION_IN_MS))
    {
        return (-1);
    }


    // nmSts = Scheduler_RegisterTask(TaskTimer_Tick, TIME_INTERVAL_100MS);

    // if (cStatus_SUCCESS != nmSts)
    // {
    //     EPRINT("failed to register timer task: [nmSts=%s]", ENUM_TO_STR_NMSTS(nmSts));
    //     // return (-1);
    // }

    DPRINT("scheduled tasks registered successfully");

    // Reset the Timer to measure the correct time from now...
    Scheduler_Reset();

    // Set begin time as current time for reference to wait next tick function
    gBeginTime = Utils_GetMonotonicTimeInNanoSec();

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

        // Dynamic Sleep that can be maximum of 10 ms
        waitNextTick();
    }

    // Deregister timer task
    // Scheduler_DeregisterTask(TaskTimer_Tick);

    return 0;
}

//----------------------------------------------------------------------------
/**
 * @brief This function dynamically sleeps the main thread according to smallest task is ready to run
 */
static void waitNextTick(void)
{
    static cI64_t diffNs;
    static cU64_t endTime;

    // Add the nanoseconds of timer tick resolution to derive the Sleep
    gBeginTime += TIMER_TICK_RESOLUTION_IN_NS;

    // get current time
    endTime = Utils_GetMonotonicTimeInNanoSec();

    // Get the Difference
    diffNs = gBeginTime - endTime;

    if (diffNs > 0)
    {
        Utils_SleepNanoSec(diffNs);
    }
    else
    {
        // convert into positive
        diffNs = ((-diffNs) / NANO_SECONDS_PER_MILLI_SECOND);
        if (diffNs > 0)
        {
            // Add warning only if we're delayed by minimum heart-beat interval.
            if (diffNs > MAX_SLEEP_DELAY_CNT_IN_MS)
            {
                WPRINT("we are running late by [%d]ms", (cI32_t)diffNs);
            }

            // If we are running late by few ms then we will not sleep for 10 ms but only for 1 ms &
            // execute the tasks again. Without this 1 ms sleep, CPU usage may go around 100%.
            Utils_SleepNanoSec(BASE_SLEEP_TIME_IN_NS);
        }
    }
}

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
