#ifndef __UTHREAD_H__
#define __UTHREAD_H__

#include <stdbool.h>

#define MAXULTHREADS 10

enum ulthread_state
{
  FREE,
  RUNNABLE,
  YIELD,
};

enum ulthread_scheduling_algorithm
{
  ROUNDROBIN,
  PRIORITY,
  FCFS, // first-come-first serve
};

struct context
{
  uint64 ra;
  uint64 sp;

  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
  int args;
};

struct user_level_thread
{
  enum ulthread_state state;
  int thread_id;
  struct context context;
  int priority;
  enum ulthread_scheduling_algorithm algo;
};

void ulthread_context_switch(struct ctx *, struct ctx *);

#endif