#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "scheduling.h"
#include "schedulers.h"

void set_task_state(struct Task *task, enum taskState taskNewState)
{
    pthread_mutex_lock(&taskStateMutex);
    task->state = taskNewState;
    pthread_mutex_unlock(&taskStateMutex);
}

void wait_for_rescheduling(int quantum, struct Task *task)
{
    int startTime;
    int waitTime;

    pthread_mutex_lock(&timeMutex);
    startTime = globalTime;
    pthread_mutex_unlock(&timeMutex);

    do
    {
        pthread_mutex_lock(&timeMutex);
        pthread_cond_wait(&timeCond, &timeMutex);
        waitTime = globalTime - startTime;
        pthread_mutex_unlock(&timeMutex);
    } while (task->state != finished && waitTime < quantum);

    usleep(timeUnitUs / 100);
}

void round_robin(struct Task **tasks, int taskCount, int timeout, int quantum)
{
    int taskIndex = 0;

    do
    {
        // Skip finished tasks or those that have not arrived yet
        if (tasks[taskIndex]->state == finished || tasks[taskIndex]->arrivalTime > globalTime)
        {
            taskIndex = (taskIndex + 1) % taskCount;
            continue;
        }

        // Set the task state to running
        if (tasks[taskIndex]->startTime == -1)
            tasks[taskIndex]->startTime = globalTime;
        set_task_state(tasks[taskIndex], running);

        // Wait for the quantum interval
        wait_for_rescheduling(quantum, tasks[taskIndex]);

        //  Check if the task is finished
        if (tasks[taskIndex]->state == finished)
        {
        }
        else
        {
            set_task_state(tasks[taskIndex], preempted);
        }

        // Find the next task to run
        taskIndex = (taskIndex + 1) % taskCount;

    } while (globalTime < timeout);
}


// Helper functions for schedulers

// Returns true when every task is finished
static bool all_tasks_finished(
    struct Task **tasks,
    int taskCount
)
{
    for (int i = 0; i < taskCount; i++)
    {
        if (tasks[i]->state != finished)
        {
            return false;
        }
    }

    return true;
}

// A task is ready if it has arrived and is not finished
static bool task_is_ready(struct Task *task)
{
    return (
        task->state != finished &&
        task->arrivalTime <= globalTime
    );
}


// Start or resume a task
static void start_task(struct Task *task)
{
    // Only set startTime the FIRST time it runs
    if (task->startTime == -1)
    {
        task->startTime = globalTime;
    }

    set_task_state(task, running);
}


// Used if no task has arrived yet
static void idle_wait(void)
{
    usleep(timeUnitUs / 100);
}


// TASK B (FCFS, SPN, HRRN)

// Implement your schedulers here!
void first_come_first_served(
    struct Task **tasks,
    int taskCount,
    int timeout
)
{
    while (
        globalTime < timeout &&
        !all_tasks_finished(tasks, taskCount)
    )
    {
        int best = -1;


        // Find ready task with earliest arrival time
        for (int i = 0; i < taskCount; i++)
        {
            if (!task_is_ready(tasks[i]))
            {
                continue;
            }


            if (
                best == -1 ||
                tasks[i]->arrivalTime <
                    tasks[best]->arrivalTime ||
                (
                    tasks[i]->arrivalTime ==
                        tasks[best]->arrivalTime &&
                    tasks[i]->ID <
                        tasks[best]->ID
                )
            )
            {
                best = i;
            }
        }


        // No task available yet
        if (best == -1)
        {
            idle_wait();
            continue;
        }


        // Start selected task
        start_task(tasks[best]);


        /*
         * FCFS is NON-PREEMPTIVE.
         *
         * We give it a very large quantum (timeout).
         * wait_for_rescheduling() returns earlier if the
         * task becomes finished.
         */
        wait_for_rescheduling(
            timeout,
            tasks[best]
        );
    }
}




void shortest_process_next(
    struct Task **tasks,
    int taskCount,
    int timeout
)
{
    while (
        globalTime < timeout &&
        !all_tasks_finished(tasks, taskCount)
    )
    {
        int best = -1;


        // Find ready task with shortest TOTAL runtime
        for (int i = 0; i < taskCount; i++)
        {
            if (!task_is_ready(tasks[i]))
            {
                continue;
            }


            if (
                best == -1 ||

                tasks[i]->totalRuntime <
                    tasks[best]->totalRuntime ||

                (
                    tasks[i]->totalRuntime ==
                        tasks[best]->totalRuntime &&

                    tasks[i]->arrivalTime <
                        tasks[best]->arrivalTime
                )
            )
            {
                best = i;
            }
        }


        if (best == -1)
        {
            idle_wait();
            continue;
        }


        start_task(tasks[best]);


        // SPN is NON-PREEMPTIVE
        wait_for_rescheduling(
            timeout,
            tasks[best]
        );
    }
}






void highest_response_ratio_next(
    struct Task **tasks,
    int taskCount,
    int timeout
)
{
    while (
        globalTime < timeout &&
        !all_tasks_finished(tasks, taskCount)
    )
    {
        int best = -1;

        double bestRatio = -1.0;


        for (int i = 0; i < taskCount; i++)
        {
            if (!task_is_ready(tasks[i]))
            {
                continue;
            }


            /*
             * Waiting time:
             *
             * current time - arrival time
             */
            int waitingTime =
                globalTime -
                tasks[i]->arrivalTime;


            /*
             * HRRN:
             *
             *          waiting + service
             * ratio = -----------------
             *               service
             *
             * service = totalRuntime
             */
            double ratio =
                (
                    waitingTime +
                    tasks[i]->totalRuntime
                )
                /
                (double)tasks[i]->totalRuntime;


            if (
                best == -1 ||
                ratio > bestRatio
            )
            {
                best = i;
                bestRatio = ratio;
            }
        }


        if (best == -1)
        {
            idle_wait();
            continue;
        }


        start_task(tasks[best]);


        // HRRN is NON-PREEMPTIVE
        wait_for_rescheduling(
            timeout,
            tasks[best]
        );
    }
}








// TASK C (SRT, FB)
void shortest_remaining_time(
    struct Task **tasks,
    int taskCount,
    int timeout,
    int quantum
)
{
    while (
        globalTime < timeout &&
        !all_tasks_finished(tasks, taskCount)
    )
    {
        int best = -1;

        int bestRemaining = 0;


        // Find task with shortest remaining runtime
        for (int i = 0; i < taskCount; i++)
        {
            if (!task_is_ready(tasks[i]))
            {
                continue;
            }


            int remaining =
                tasks[i]->totalRuntime -
                tasks[i]->currentRuntime;


            if (
                best == -1 ||

                remaining < bestRemaining ||

                (
                    remaining == bestRemaining &&
                    tasks[i]->arrivalTime <
                        tasks[best]->arrivalTime
                )
            )
            {
                best = i;
                bestRemaining = remaining;
            }
        }


        if (best == -1)
        {
            idle_wait();
            continue;
        }


        start_task(tasks[best]);


        /*
         * SRT is PREEMPTIVE.
         *
         * Run only for one quantum = 10 time units.
         */
        wait_for_rescheduling(
            quantum,
            tasks[best]
        );


        // Stop task if it did not finish
        if (tasks[best]->state != finished)
        {
            set_task_state(
                tasks[best],
                preempted
            );
        }
    }
}



void feedback(
    struct Task **tasks,
    int taskCount,
    int timeout,
    int quantum
)
{
    /*
     * Smaller number = higher priority.
     *
     * New tasks start at priority level 0.
     */
    int priorityLevel[taskCount];


    for (int i = 0; i < taskCount; i++)
    {
        priorityLevel[i] = 0;
    }


    while (
        globalTime < timeout &&
        !all_tasks_finished(tasks, taskCount)
    )
    {
        int best = -1;


        /*
         * Find ready task with highest priority.
         *
         * Level 0 is better than level 1,
         * level 1 is better than level 2, etc.
         */
        for (int i = 0; i < taskCount; i++)
        {
            if (!task_is_ready(tasks[i]))
            {
                continue;
            }


            if (
                best == -1 ||

                priorityLevel[i] <
                    priorityLevel[best] ||

                (
                    priorityLevel[i] ==
                        priorityLevel[best] &&

                    tasks[i]->arrivalTime <
                        tasks[best]->arrivalTime
                )
            )
            {
                best = i;
            }
        }


        if (best == -1)
        {
            idle_wait();
            continue;
        }


        start_task(tasks[best]);


        // Feedback is PREEMPTIVE
        wait_for_rescheduling(
            quantum,
            tasks[best]
        );


        if (tasks[best]->state != finished)
        {
            // Stop it after one quantum
            set_task_state(
                tasks[best],
                preempted
            );


            /*
             * It used its complete quantum,
             * so lower its priority.
             */
            priorityLevel[best]++;
        }
    }
}
