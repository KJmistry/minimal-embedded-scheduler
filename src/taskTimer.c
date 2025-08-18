/*****************************************************************************
 * @file    taskTimer.c
 * @author  Kshitij Mistry
 * @brief   Task Timer utility implementation file.
 *
 * This file implements the task timer utility, which allows for the registration,
 * deregistration, and execution of timers that expire after a specified duration.
 *
 *****************************************************************************/

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "taskTimer.h"

/*****************************************************************************
 * MACROS
 *****************************************************************************/
/** Timer tick in miliseconds */
#define TASK_TIMER_TICK_RESOLUTION_IN_MS (100)

/*****************************************************************************
 * VARIABLES
 *****************************************************************************/
/** Task timer linked list head */
static TaskTimer_t *taskTimerHead = NULL;

/*****************************************************************************
 * FUNCTION DEFINATIONS
 *****************************************************************************/
//----------------------------------------------------------------------------
/**
 * @brief Initialize timer task utility
 */
void TaskTimer_Init(void)
{
    taskTimerHead = NULL;
}

//----------------------------------------------------------------------------
/**
 * @brief Exit timer task utility
 */
void TaskTimer_Exit(void)
{
    if (taskTimerHead == NULL)
    {
        return;
    }

    TaskTimer_t *curNode = taskTimerHead;
    while (curNode != NULL)
    {
        TaskTimer_t *prevNode = curNode;
        curNode = curNode->next;

        /* Stop single shot timer */
        TaskTimer_Stop(prevNode);
    }
}

//----------------------------------------------------------------------------
/**
 * @brief Insert timer in the timer list head
 * @param taskTimer - Timer Handle
 * @param expireAfterMs - Time in seconds after which timer expires
 * @param callback - callback routine
 * @param userData - Pointer to user data
 * @return Returns true if started successfully, false otherwise
 * @note It timer is already inserted in the list, it will not be inserted again but number of ticks will get overwrite
 */
cBool TaskTimer_Start(TaskTimer_t *taskTimer, cU32_t expireAfterMs, void (*callback)(void *userData), void *userData)
{
    /* Validate input params */
    if ((taskTimer == NULL) || (callback == NULL))
    {
        return c_FALSE;
    }

    /* Derive ticks to expire timer */
    taskTimer->ticks = (expireAfterMs/TASK_TIMER_TICK_RESOLUTION_IN_MS);

    /* Store application data and callback */
    taskTimer->userData = userData;
    taskTimer->callback = callback;

    /* It will add the task at beginning of the list */
    if (taskTimerHead == NULL)
    {
        taskTimerHead = taskTimer;
        taskTimer->next = NULL;
    }
    else
    {
        if (c_TRUE == TaskTimer_IsRunning(taskTimer))
        {
            return (c_TRUE);
        }

        taskTimer->next = taskTimerHead;
        taskTimerHead = taskTimer;
    }

    return (c_TRUE);
}

//----------------------------------------------------------------------------
/**
 * @brief Remove timer from the lists
 * @param taskTimer - Timer handle
 */
void TaskTimer_Stop(TaskTimer_t *taskTimer)
{
    if (taskTimerHead == NULL)
    {
        return;
    }

    if (taskTimerHead == taskTimer)
    {
        taskTimerHead = taskTimerHead->next;
        taskTimer->next = NULL;
    }
    else
    {
        TaskTimer_t *prevNode = taskTimerHead;
        TaskTimer_t *curNode = taskTimerHead->next;

        while (curNode != NULL)
        {
            if (curNode == taskTimer)
            {
                prevNode->next = curNode->next;
                return;
            }

            prevNode = curNode;
            curNode = curNode->next;
        }
    }
}

//----------------------------------------------------------------------------
/**
 * @brief Decrement number of ticks of each timer in the list
 */
void TaskTimer_Tick(void)
{
    if (taskTimerHead == NULL)
    {
        return;
    }

    TaskTimer_t *curNode = taskTimerHead;
    while (curNode != NULL)
    {
        curNode->ticks--;
        if (curNode->ticks)
        {
            curNode = curNode->next;
            continue;
        }

        TaskTimer_t *expireNode = curNode;
        curNode = curNode->next;

        /* Stop single shot timer */
        TaskTimer_Stop(expireNode);

        /* Execute timer expiry callback */
        (*expireNode->callback)(expireNode->userData);
    }
}

//----------------------------------------------------------------------------
/**
 * @brief Check if timer is running or not
 * @param taskTimer - Timer handle
 * @return Returns true if timer is running, false otherwise
 */
cBool TaskTimer_IsRunning(TaskTimer_t *taskTimer)
{
    TaskTimer_t *nextNode;

    if ((taskTimerHead == NULL) || (taskTimer == NULL))
    {
        return (c_FALSE);
    }

    if (taskTimerHead == taskTimer)
    {
        return (c_TRUE);
    }

    nextNode = taskTimerHead->next;
    while (nextNode != NULL)
    {
        if (nextNode == taskTimer)
        {
            return (c_TRUE);
        }
        nextNode = nextNode->next;
    }

    return (c_FALSE);
}

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
