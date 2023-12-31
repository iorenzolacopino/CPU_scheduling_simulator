#include "fake_process.h"
#include "linked_list.h"
#pragma once
#define MAX_NUM_PROCESSES 3


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  FakePCB** running;
  ListHead ready;
  ListHead waiting;
  int timer;
  int *burst_time;
  ScheduleFn schedule_fn;
  void* schedule_args;

  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
int FakeOS_isFree(FakeOS *os);
int FakeOS_isRunning(FakeOS *os);
