/*****************************************************************************
 * @file    taskTimer.h
 * @author  Kshitij Mistry
 * @brief   Task Timer utility header file.
 *
 * This header file defines the task timer utility, which allows for the registration,
 * deregistration, and execution of timers that expire after a specified duration.
 *
 *****************************************************************************/
#pragma once

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "common_stddef.h"

/*****************************************************************************
 * STRUCTURES
 *****************************************************************************/
/**
 * @brief   Task timer link list node information
 */
typedef struct TaskTimer_t
{
    /** Time interval in millisec at which tick will be provided to task timer module */
    cU32_t timerTickInMs;

    /** Timer tick count to be wait for timer expire */
    cU32_t ticks;

    /** Application data */
    void *userData;

    /** Timer expiry callback routine */
    void (*callback)(void *userData);

    /** Reference of next node in the list */
    struct TaskTimer_t *next;

}TaskTimer_t;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

void TaskTimer_Init(void);

void TaskTimer_Exit(void);

cBool TaskTimer_Start(TaskTimer_t *taskTimer, cU32_t expireAfterMs, void (*callback)(void *userData), void *userData);

void TaskTimer_Stop(TaskTimer_t *taskTimer);

void TaskTimer_Tick(void);

cBool TaskTimer_IsRunning(TaskTimer_t *taskTimer);

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
