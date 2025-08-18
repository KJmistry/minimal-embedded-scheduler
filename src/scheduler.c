/*****************************************************************************
 * @file    scheduler.c
 * @author  Kshitij Mistry
 * @brief   Task Scheduler module implementation file.
 *
 * This file implements the task scheduler module, which allows for the registration,
 * deregistration, and execution of periodic tasks. It provides functions to manage
 * tasks that run at specified intervals, along with necessary data structures and enumerations.
 *
 *****************************************************************************/

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "common_def.h"
#include "common_utils.h"
#include "scheduler.h"
#include <time.h>

/*****************************************************************************
 * MACROS
 *****************************************************************************/
/** Maximum number of tasks in allowed in the scheduler list */
#define	SCHEDULED_TASKS_MAX         10

/** Maximum allowed clock resolution by platform should not exceed 1000 millisec */
#define CLOCK_RESOLUTION_MS_MAX     1000

/** Minimum sleep of application super loop in ns */
#define	BASE_SLEEP_TIME_IN_NS       (1000000LL)

/*****************************************************************************
 * VARIABLES
 *****************************************************************************/
/** Timer Resolution Ms */
static cU32_t gTimerResolutionInMilliSec = 0;

/** Timer Resolution Ns */
static cI64_t gTimerResolutionInNanoSec = 0;

/** Max permitted delay of scheduler */
static cU32_t gMaxPermittedDelayMilliSec = 0;

/** Monotonic clock reference time */
static cU64_t gLastMonotonicTimeInNanoSec = 0;

/** Time Since Epoch clock reference time */
static struct tm gLastTimeRefSinceEpoch;

/** Elapsed nano sec Since Update Tick called Last */
static cU64_t gElapsedNanoSec = 0;

/** schedule task desc */
static ScheduleTask_t gTaskList[SCHEDULED_TASKS_MAX];

/** total count of registered task */
static cU8_t gTaskCnt = 0;

/** Time reference from beginning of caller super loop */
static cI64_t gBeginTime = 0;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

static cBool isTaskAlreadyInList(void (*callback)(void));

static void reshuffleTaskList(void);

/*****************************************************************************
 * FUNCTION DEFINATIONS
 *****************************************************************************/
/**
 * @brief Initializes task scheduler
 * @param timerResolutionMs - Timer resolution in millisec
 * @param pMaxPermittedDelayMs - Max permitted scheduler delay in millisec
 * @return Return success or failure
 */
cBool Scheduler_Init(cU32_t timerResolutionMs, cU32_t *pMaxPermittedDelayMs)
{
    cU8_t taskId;
    struct timespec resolution;

    for (taskId = 0; taskId < SCHEDULED_TASKS_MAX; taskId++)
    {
        gTaskList[taskId].callback = NULL;
        gTaskList[taskId].runInterval = 0;
        gTaskList[taskId].readyToRun = c_FALSE;
        gTaskList[taskId].clockTick.clockTicksCnt = 0;
        gTaskList[taskId].clockTick.clockTicksThreshold = 0;
    }

    if ((0 == timerResolutionMs) || (timerResolutionMs >= CLOCK_RESOLUTION_MS_MAX))
    {
        return (c_FALSE);
    }

    /* Get the CPU Clock resolution */
    if (0 != clock_getres(CLOCK_MONOTONIC, &resolution))
    {
        EPRINT("fail to get cpu clock resolution");
        return (c_FALSE);
    }

    if (resolution.tv_sec > 0)
    {
        EPRINT("clock resolution is more than 1 second: [resolution=%ld]", resolution.tv_sec);
        return (c_FALSE);
    }

    DPRINT("cpu clock resolution: [sec=%ld], [n_sec=%ld]", resolution.tv_sec, resolution.tv_nsec);

    /* Validate the resolution */
    if (((cI64_t)timerResolutionMs) < (resolution.tv_nsec / NANO_SECONDS_PER_MILLI_SECOND))
    {
        EPRINT("can not satisfy given clock resolution: [value=%d ms]", timerResolutionMs);
        return (c_FALSE);
    }

    /* Save the Timer Resolution in milli sec */
    gTimerResolutionInMilliSec = timerResolutionMs;

    /* Save the Timer Resolution in nano sec */
    gTimerResolutionInNanoSec = (cU64_t) (((cU64_t)gTimerResolutionInMilliSec) * NANO_SECONDS_PER_MILLI_SECOND);

    if (NULL != pMaxPermittedDelayMs)
    {
        /* Save max permitted delay in milli sec */
        gMaxPermittedDelayMilliSec = *pMaxPermittedDelayMs;
    }
    else
    {
        /* Default value to 100 times of timer resolution if not provided */
        gMaxPermittedDelayMilliSec = gTimerResolutionInMilliSec * 100;
    }

    return (c_TRUE);
}

//----------------------------------------------------------------------------
/**
 * @brief Function that register Tasks (add it to list)
 * @param (*callback) - Address of function to be registered
 * @param runInterval - Interval for execution of task
 * @return Error code to indicate success or failure of function
 */
cStatus_e Scheduler_RegisterTask(void (*callback)(void), TimeInterval_e runInterval)
{
    cBool isRegistered = c_FALSE;
    cU8_t taskId;

    /* validate arguments */
    if (NULL == callback)
    {
        return (cStatus_INVALID_ARG);
    }

    /* Check if max tasks are already registered */
    if (gTaskCnt >= SCHEDULED_TASKS_MAX)
    {
        return (cStatus_NO_RESOURCE);
    }

    if (c_TRUE == isTaskAlreadyInList(callback))
    {
        return (cStatus_OPERATION_FAIL);
    }

    for (taskId = 0; taskId < SCHEDULED_TASKS_MAX; taskId++)
    {
        if (NULL != gTaskList[taskId].callback)
        {
            continue;
        }

        gTaskList[taskId].callback = callback;
        gTaskList[taskId].runInterval = runInterval;
        gTaskList[taskId].readyToRun = c_FALSE;
        gTaskList[taskId].clockTick.clockTicksCnt = 0;
        gTaskList[taskId].clockTick.clockTicksThreshold = 0;
        isRegistered = c_TRUE;

        switch (runInterval)
        {
            case TIME_INTERVAL_10MS:
                gTaskList[taskId].clockTick.clockTicksThreshold = (10 / gTimerResolutionInMilliSec);
                break;

            case TIME_INTERVAL_20MS:
                gTaskList[taskId].clockTick.clockTicksThreshold = (20 / gTimerResolutionInMilliSec);
                break;

            case TIME_INTERVAL_100MS:
                gTaskList[taskId].clockTick.clockTicksThreshold = (100 / gTimerResolutionInMilliSec);
                break;

            default:
                /* we are not using TimerClockTick_t structure for tasks having run interval of more than 1 sec */
                break;
        }

        /* Increase Tasks cnt */
        gTaskCnt++;
        break;
    }

    if (c_FALSE == isRegistered)
    {
        return (cStatus_OPERATION_FAIL);
    }

    return (cStatus_SUCCESS);
}

//----------------------------------------------------------------------------
/**
 * @brief Function that deregister Tasks (remove from list)
 * @param (*callback) - Address of function to be deregistered
 * @return Error code to indicate success or failure of function
 */
cStatus_e Scheduler_DeregisterTask(void (*callback)(void))
{
    cU8_t taskId;

    /* validate argument */
    if (NULL == callback)
    {
        return (cStatus_INVALID_ARG);
    }

    /* If no tasks are registered, we can't deregister */
    if (0 == gTaskCnt)
    {
        return (cStatus_SUCCESS);
    }

    for (taskId = 0; taskId < SCHEDULED_TASKS_MAX; taskId++)
    {
        if (callback != gTaskList[taskId].callback)
        {
            continue;
        }

        gTaskList[taskId].readyToRun = c_FALSE;
        gTaskList[taskId].runInterval = TIME_INTERVAL_NONE;
        gTaskList[taskId].callback = NULL;
        gTaskList[taskId].clockTick.clockTicksCnt = 0;
        gTaskList[taskId].clockTick.clockTicksThreshold = 0;

        /* Decrease Tasks cnt */
        gTaskCnt--;
    }

    /* Reshuffle Task List */
    reshuffleTaskList();

    return (cStatus_SUCCESS);
}

//----------------------------------------------------------------------------
/**
 * @brief Function that reset all timer event and saves start point
 */
void Scheduler_Reset(void)
{
    cU8_t taskId;

    /* Set begin time as current time for reference to calculate dynamic sleep */
    gBeginTime = Utils_GetMonotonicTimeInNanoSec();

    /* Get the Time Since Epoch */
    Utils_GetTimeSinceEpoch(&gLastTimeRefSinceEpoch);

    /* Reset Last ref monotonic time */
    gLastMonotonicTimeInNanoSec = Utils_GetMonotonicTimeInNanoSec();

    /* Reset Elapsed nano sec */
    gElapsedNanoSec = 0;

    /* Reset all Timer Eve clock tick count */
    for (taskId = 0; taskId < SCHEDULED_TASKS_MAX; taskId++)
    {
        gTaskList[taskId].clockTick.clockTicksCnt = 0;
        gTaskList[taskId].readyToRun = c_FALSE;
    }

    DPRINT("reset scheduler timer");
}

//----------------------------------------------------------------------------
/**
 * @brief Function that gets the the time and calculate difference to generate timer events
 */
void Scheduler_UpdateTick(void)
{
    cU64_t curMonotonicTimeInNanSec = 0;
    cU8_t taskId;
    cU32_t timerEvents = 0;
    struct tm curTimeStamp;

    /* Get Current Monotonic time in nano sec */
    curMonotonicTimeInNanSec = Utils_GetMonotonicTimeInNanoSec();

    /* Update elapsed nano seconds */
    gElapsedNanoSec += (curMonotonicTimeInNanSec - gLastMonotonicTimeInNanoSec);

    /* Now check the difference */
    if (gElapsedNanoSec >= gTimerResolutionInNanoSec)
    {
        do
        {
            /* Decrement Elapsed time */
            gElapsedNanoSec -= gTimerResolutionInNanoSec;

            /* Increment clock tick count */
            for (taskId = 0; taskId < gTaskCnt; taskId++)
            {
                gTaskList[taskId].clockTick.clockTicksCnt++;
            }

        } while (gElapsedNanoSec >= gTimerResolutionInNanoSec);


        /* Compare clock tick cnt with threshold cnt */
        for (taskId = 0; taskId < gTaskCnt; taskId++)
        {
            /*
             * As we only derive task from TimerClockTick_t structure which are having run interval less than 1 sec
             * So, if mClockTicksThreshold is "zero" means task run interval is >= 1 sec
             */
            if (0 == gTaskList[taskId].clockTick.clockTicksThreshold)
            {
                continue;
            }

            if (gTaskList[taskId].clockTick.clockTicksCnt >= gTaskList[taskId].clockTick.clockTicksThreshold)
            {
                gTaskList[taskId].clockTick.clockTicksCnt -= gTaskList[taskId].clockTick.clockTicksThreshold;

                /* set timer event */
                timerEvents |= gTaskList[taskId].runInterval;
            }
        }

        /* Get Time since epoch to execute task whose run interval greater/equal to 1 sec */
        Utils_GetTimeSinceEpoch(&curTimeStamp);

        if (curTimeStamp.tm_sec != gLastTimeRefSinceEpoch.tm_sec)
        {
            /* set timer event for 1 sec */
            timerEvents |= TIME_INTERVAL_1SEC;
        }

        if (curTimeStamp.tm_min != gLastTimeRefSinceEpoch.tm_min)
        {
            /* set timer event for 1 min */
            timerEvents |= TIME_INTERVAL_1MIN;
        }

        /* save current time as reference */
        gLastTimeRefSinceEpoch = curTimeStamp;
    }

    /** Mark all tasks ready to run */
    for (taskId = 0; taskId < gTaskCnt; taskId++)
    {
        if (timerEvents & gTaskList[taskId].runInterval)
        {
            gTaskList[taskId].readyToRun = c_TRUE;
        }
    }

    /* save current time as reference */
    gLastMonotonicTimeInNanoSec = curMonotonicTimeInNanSec;
}

//----------------------------------------------------------------------------
/**
 * @brief Function that calculates the dynamic sleep time
 * @return Sleep time in nanoseconds
 */
cI64_t Scheduler_GetDynamicSleep(void)
{
    static cI64_t diffNs;
    static cU64_t endTime;

    /* Add the nanoseconds of timer tick resolution to derive the Sleep */
    gBeginTime += gTimerResolutionInNanoSec;

    /* get current time */
    endTime = Utils_GetMonotonicTimeInNanoSec();

    /* Get the Difference */
    diffNs = gBeginTime - endTime;

    if (diffNs > 0)
    {
        return (diffNs);
    }
    else
    {
        /* convert into positive */
        diffNs = ((-diffNs) / NANO_SECONDS_PER_MILLI_SECOND);
        if (diffNs > 0)
        {
            /* Add warning only if we're delayed by minimum heart-beat interval */
            if (diffNs > gMaxPermittedDelayMilliSec)
            {
                WPRINT("we are running late by [%d]ms", (cI32_t)diffNs);
            }

            /* If we are running late by few ms then we will not sleep for 10 ms but only for 1 ms &
             * execute the tasks again. Without this 1 ms sleep, CPU usage may go around 100%. */
            return (BASE_SLEEP_TIME_IN_NS);
        }
    }
}

//----------------------------------------------------------------------------
/**
 * @brief This function run all tasks that are ready to run
 */
void Scheduler_ExecuteTasksReadyToRun(void)
{
    cU8_t taskId;

    for (taskId = 0; taskId < gTaskCnt; taskId++)
    {
        if (c_TRUE == gTaskList[taskId].readyToRun)
        {
            gTaskList[taskId].readyToRun = c_FALSE;

            /* Execute the Task */
            (*gTaskList[taskId].callback)();
        }
    }
}

//----------------------------------------------------------------------------
/**
 * @brief Function checks if task is already registered or not
 * @param (*callback) - Address of function to be registered
 * @return true if already registered, false otherwise
 */
static cBool isTaskAlreadyInList(void (*callback)(void))
{
    cU8_t taskId;

    /* validate arguments */
    if (NULL == callback)
    {
        return (c_FALSE);
    }

    /* check whether function is already register or not */
    for (taskId = 0; taskId < gTaskCnt ; taskId++)
    {
        if (callback == gTaskList[taskId].callback)
        {
            return (c_TRUE);
        }
    }

    return (c_FALSE);
}

//----------------------------------------------------------------------------
/**
 * @brief This function moves the task up in the list to fill up empty location
 */
static void reshuffleTaskList(void)
{
    cU8_t taskId;

    if (0 == gTaskCnt)
    {
        return;
    }

    for (taskId = 0; taskId < SCHEDULED_TASKS_MAX; taskId++)
    {
        if (NULL == gTaskList[taskId].callback)
        {
            break;
        }
    }

    cU8_t tmpTaskId = taskId;
    cU8_t shiftCnt = 1;

    /* Empty location found! So shift other tasks up in the list */
    for (shiftCnt = 1; shiftCnt < (SCHEDULED_TASKS_MAX - taskId); shiftCnt++)
    {
        if (NULL == gTaskList[taskId + shiftCnt].callback)
        {
            continue;
        }

        gTaskList[tmpTaskId++]	= gTaskList[taskId + shiftCnt];

        gTaskList[taskId + shiftCnt].readyToRun = c_FALSE;
        gTaskList[taskId + shiftCnt].runInterval = TIME_INTERVAL_NONE;
        gTaskList[taskId + shiftCnt].callback = NULL;
        gTaskList[taskId + shiftCnt].clockTick.clockTicksCnt = 0;
        gTaskList[taskId + shiftCnt].clockTick.clockTicksThreshold = 0;
    }
}

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/