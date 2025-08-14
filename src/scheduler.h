/*****************************************************************************
 * @file    scheduler.h
 * @author  Kshitij Mistry
 * @brief   Task Scheduler module header file.
 *
 * This header file defines the task scheduler module, which allows for the registration,
 * deregistration, and execution of periodic tasks. It provides an interface for managing
 * tasks that run at specified intervals, along with necessary data structures and enumerations.
 *
 *****************************************************************************/
#pragma once

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "common_stddef.h"

/*****************************************************************************
 * ENUMS
 *****************************************************************************/
/**
 * @brief	It lists different types task intervals used when registering task with Task Scheduler module
 */
typedef enum
{
   /** Time interval unknown */
   TIME_INTERVAL_NONE				= 0x00,

   /** Time interval 10ms tick */
   TIME_INTERVAL_10MS				= 1<<0,

   /** Time interval 20ms tick */
   TIME_INTERVAL_20MS				= 1<<1,

   /** Time interval 100ms tick */
   TIME_INTERVAL_100MS				= 1<<2,

   /** Time interval 1sec tick */
   TIME_INTERVAL_1SEC				= 1<<3,

   /** Time interval 1min tick */
   TIME_INTERVAL_1MIN				= 1<<4

}TimeInterval_e;

/*****************************************************************************
 * STRUCTURES
 *****************************************************************************/
/**
 * @brief	This structure contain clock tick properties for each task which are of execution interval less than 1 sec
 */
typedef struct
{
    /** Tick counter for timer */
    cU32_t clockTicksCnt;

    /** Max ticks for timer */
    cU16_t clockTicksThreshold;

}TimerClockTick_t;

/**
 * @brief	Descriptor of every periodic task
 */
typedef struct
{
    /** Is task ready to execute */
    cBool readyToRun;

    /** Base tick interval for timer task */
    cU8_t runInterval;

    /** Timer tick calculator */
    TimerClockTick_t clockTick;

    /** Callback function for timer */
    void (*callback)(void);

}ScheduleTask_t;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

cBool Scheduler_Init(cU32_t timerResolutionMs);

cStatus_e Scheduler_RegisterTask(void (*callback)(void), TimeInterval_e runInterval);

cStatus_e Scheduler_DeregisterTask(void (*callback)(void));

void Scheduler_Reset(void);

void Scheduler_UpdateTick(void);

void Scheduler_ExecuteTasksReadyToRun(void);

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
