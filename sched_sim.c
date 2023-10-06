#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

// cpu scheduling based on shortest job first policy
typedef struct {
  int quantum;
} SchedSJFArgs;

void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  if (os->running){
    assert(os->running->events.first);
    ProcessEvent *e=(ProcessEvent*)os->running->events.first;
    assert(e->type==CPU);
    List_pushFront(&os->ready, (ListItem*)os->running);
    os->running=0;
  }

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;

  FakePCB* sjf=0;
  int min=999; // we assume that INT_MAX = 999

  ListItem *aux=os->ready.first;
  while (aux){
    FakePCB *pcb=(FakePCB*)aux;
    assert(pcb->events.first);
    ProcessEvent *e=(ProcessEvent*)pcb->events.first;
    assert(e->type==CPU);

    if (e->duration<min){
      sjf=pcb;
      min=e->duration;
    }
    
    aux = aux->next;
  }

  // if cpu burst time is smaller than min
  // put the process in front of the list
  if (sjf){
    List_detach(&os->ready, (ListItem*)sjf);
    List_pushFront(&os->ready, (ListItem*)sjf);
  }

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  ProcessEvent *e=(ProcessEvent*)sjf->events.first;
  if (e->duration>args->quantum){
    ProcessEvent *qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&sjf->events, (ListItem*)qe);
  }
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  int quantum;
  printf("insert quantum time: ");
  scanf("%d", &quantum);
  assert(quantum>0);
  srr_args.quantum=quantum;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
