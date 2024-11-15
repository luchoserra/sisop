#ifndef SCHED_H
#define SCHED_H

#define NUMPROC 5
#define TIMESLICE 20

enum Policy { FIFO, SJF, STCF, RR};

enum Status { RUNNING, RUNNABLE, BLOCKED };

struct proc {
    int pid;
    enum Status status;
    int duration;
    int runtime;
};

void scheduler();
struct proc *select_next_fifo();
struct proc *select_next_cfs();
struct proc *select_next_rr();
struct proc *select_next_sjf();
struct proc *select_next_stcf();

#endif