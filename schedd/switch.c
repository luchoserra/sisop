#include "switch.h"

#include "sched.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

extern struct proc proc[];
extern struct proc_type proc_type[];

struct proc_stats {
    int runtime;
};

struct sim_status {
    int remaining_steps;
    int runtime;
    int idle_runtime;
    struct proc_type proc_type[NUMPROC];
    struct proc_stats proc_stats[NUMPROC];
} sim_status;

struct proc_type idle_type = {10.0, 1.0};

double drand(){
    return (double) rand() / (double) RAND_MAX;
}

double intended_run(struct proc_type *pc){
    return -1 * pc->mean_cpu_time * log(drand());
}

int should_unlock(struct proc_type *pc){
    return drand() < pc->unblocking_probability;
}

void init_simulation(int steps){
    sim_status.remaining_steps = steps;
    sim_status.runtime = 0;

    for (int i = 0; i < NUMPROC; i++) {
        sim_status.proc_stats[i].runtime = 0;
        sim_status.proc_type[i] = proc_type[i];
    }
}

int done(){
    return sim_status.remaining_steps <= 0;
}

void step(){
    sim_status.remaining_steps -= 1;

    //simulate state transitions
    for(int i=0; i<NUMPROC; i++){
        if (proc[i].status == BLOCKED && should_unlock(&sim_status.proc_type[i])){
            proc[i].status = RUNNABLE;
        }
    }
}

struct proc_type* ptype(struct proc* target){
    return &sim_status.proc_type[target->pid];
}

struct proc_stats* pstats(struct proc* target){
    return &sim_status.proc_stats[target->pid];
}

int swtch(struct proc* target){
    int intended;

    step();

    intended = intended_run(ptype(target));
    if (intended > TIMESLICE) {
        intended = TIMESLICE;
        target->status = RUNNABLE;
    } else {
        target->status = BLOCKED;
    }

    pstats(target)->runtime += intended;
    sim_status.runtime += intended;
    return intended;
}

int idle(){
    int intended;
    step();
    intended = intended_run(&idle_type);
    sim_status.runtime += intended;
    sim_status.idle_runtime += intended;
    
    return intended;
}


void print_stats(){
    double rt;
    rt = (double) sim_status.runtime;
    
    printf("Total time: %d\n", sim_status.runtime);
    printf("Idle time: %.0f%%\n", 100 * (double) sim_status.idle_runtime / rt);

    for(int i=0; i < NUMPROC; i++) {
        printf("Proc %d: %.0f%%\n", i, 100 * (double) sim_status.proc_stats[i].runtime / rt);
    }
}