/* CSE 536: User-Level Threading Library */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/ulthread.h"

/* Standard definitions */
#include <stdbool.h>
#include <stddef.h>

// user_level_thread

int gThreadID = 0;
int threadsDestroyed = 0;
struct user_level_thread *current_running_thread;
struct user_level_thread uthreads[MAXULTHREADS];
struct user_level_thread scheduling_thread;

/* Get thread ID*/
int get_current_tid()
{
    return current_running_thread->thread_id;
}

/* Thread initialization */
void ulthread_init(int schedalgo)
{
    current_running_thread = &scheduling_thread;
    // current_running_thread->thread_id = gThreadID;
    current_running_thread->algo = schedalgo;
}

/* Thread creation */
bool ulthread_create(uint64 start, uint64 stack, uint64 args[], int priority)
{

    struct user_level_thread *uthread;
    for (uthread = uthreads; uthread < &uthreads[MAXULTHREADS]; uthread++)
    {
        if (uthread->state == FREE)
            break;
    }

    uthread->state = RUNNABLE;
    uthread->thread_id = ++gThreadID;

    memset(&uthread->context, 0, sizeof uthread->context);
    uthread->context.args = (int)args[0];
    uthread->context.ra = start;
    uthread->context.sp = stack;
    uthread->priority = priority;

    /* Please add thread-id instead of '0' here. */
    printf("[*] ultcreate(tid: %d, ra: %p, sp: %p)\n", uthread->thread_id, start, stack);

    return false;
}

void ulthread_schedule_roundrobin()
{
    struct user_level_thread *uthread;

    uthread = current_running_thread + 1;
    while (threadsDestroyed < gThreadID)
    {
        if (uthread > &uthreads[MAXULTHREADS])
            uthread = uthreads;
        if (uthread->state == RUNNABLE || uthread->state == YIELD)
        {
            current_running_thread = uthread;
            current_running_thread->state = RUNNABLE;
            printf("[*] ultschedule (next tid: %d)\n", current_running_thread->thread_id);
            struct user_level_thread *scheduler;
            scheduler = &scheduling_thread;
            ulthread_context_switch(&scheduler->context, &uthread->context);
        }
        uthread = uthread + 1;
    }
}

void ulthread_schedule_priority()
{
    struct user_level_thread *next_thread = 0;
    struct user_level_thread *uthread, *t1, *t2;
    uthread = current_running_thread + 1;
    t2 = current_running_thread + 1;
    while (threadsDestroyed < gThreadID)
    {
        if (uthread > &uthreads[MAXULTHREADS])
            uthread = uthreads;
        if ((uthread->state == RUNNABLE || uthread->state == YIELD))
        {
            for (t1 = uthreads; t1 < &uthreads[MAXULTHREADS]; t1++)
            {
                if ((t1->priority > uthread->priority) && (t1->state == RUNNABLE || t1->state == YIELD) && t1 != current_running_thread)
                {
                    uthread = t1;
                }
            }
            current_running_thread = uthread;
            current_running_thread->state = RUNNABLE;
            printf("[*] ultschedule (next tid: %d)\n", current_running_thread->thread_id);
            struct user_level_thread *scheduler;
            scheduler = &scheduling_thread;
            ulthread_context_switch(&scheduler->context, &uthread->context);
        }
        t2 = t2 + 1;
        uthread = t2;
    }
}

void ulthread_schedule_fcfs()
{
    struct user_level_thread *next_thread = 0;
    struct user_level_thread *uthread, *t1, *t2;
    uthread = current_running_thread + 1;
    t2 = current_running_thread + 1;
    while (threadsDestroyed < gThreadID)
    {
        if (uthread > uthreads + MAXULTHREADS)
        {
            t2 = uthreads;
        }
        if ((uthread->state == RUNNABLE || uthread->state == YIELD))
        {
            for (t1 = uthreads; t1 < uthreads + MAXULTHREADS; t1++)
            {
                if (t1->thread_id != current_running_thread->thread_id)
                {
                    if ((t1->thread_id < uthread->thread_id) && (t1->state == RUNNABLE || t1->state == YIELD))
                    {
                        uthread = t1;
                    }
                }
            }
            current_running_thread = uthread;
            current_running_thread->state = RUNNABLE;
            printf("[*] ultschedule (next tid: %d)\n", current_running_thread->thread_id);
            struct user_level_thread *scheduler;
            scheduler = &scheduling_thread;
            ulthread_context_switch(&scheduler->context, &uthread->context);
        }
        t2 = t2 + 1;
        uthread = t2;
    }
}

/* Thread scheduler */
void ulthread_schedule(void)
{
    if (current_running_thread->algo == ROUNDROBIN)
    {
        ulthread_schedule_roundrobin();
    }
    else if (current_running_thread->algo == PRIORITY)
    {
        ulthread_schedule_priority();
    }
    else if (current_running_thread->algo == FCFS)
    {
        ulthread_schedule_fcfs();
    }
}

/* Yield CPU time to some other thread. */
void ulthread_yield(void)
{
    printf("[*] ultyield(tid: %d)\n", current_running_thread->thread_id);
    current_running_thread->state = YIELD;
    // make_threads_runnable();
    struct user_level_thread *scheduler;
    scheduler = &scheduling_thread;
    ulthread_context_switch(&current_running_thread->context, &scheduler->context);
}

/* Destroy thread */
void ulthread_destroy(void)
{
    threadsDestroyed++;
    printf("[*] ultdestroy(tid: %d)\n", current_running_thread->thread_id);
    current_running_thread->state = FREE;
    struct user_level_thread *scheduler;
    scheduler = &scheduling_thread;
    ulthread_context_switch(&current_running_thread->context, &scheduler->context);
}