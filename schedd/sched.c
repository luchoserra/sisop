#include "sched.h"
#include "switch.h"
#include <stdlib.h>
#include <limits.h>

extern struct proc proc[];

int rr_index = 0;

/* First In First Out */
struct proc* select_next_fifo() {
    for (int i = 0; i < NUMPROC; i++) {
        if (proc[i].status == RUNNABLE) {
            return &proc[i];
        }
    }
    return NULL;
}

/* Shortest Job First */
struct proc *select_next_sjf() {
    struct proc* shortest = NULL;

    for (int i = 0; i < NUMPROC; i++) {
        if (proc[i].status == RUNNABLE) {
            if (shortest == NULL || proc[i].runtime < shortest->runtime) {
                shortest = &proc[i];
            }
        }
    }

    return shortest;
}

/* Shortest Time to Completion First */
struct proc* select_next_stcf() {
    struct proc* shortest = NULL;

    for (int i = 0; i < NUMPROC; i++) {
        if (proc[i].status == RUNNABLE) {
            if (shortest == NULL || proc[i].duration < shortest->duration) {
                shortest = &proc[i];
            }
        }
    }

    return shortest;
}

/* Round Robin */
struct proc *select_next_rr() {
    for (int i = 0; i < NUMPROC; i++) {
        int index = (rr_index + i) % NUMPROC;
        if (proc[index].status == RUNNABLE) {
            rr_index = (index + 1) % NUMPROC; // Update to the next index
            return &proc[index];
        }
    }
    return NULL;
}

struct proc* select_next(enum Policy policy) {
    switch(policy) {
        case FIFO: return select_next_fifo();
        case SJF: return select_next_sjf();
        case STCF: return select_next_stcf();
        case RR: return select_next_rr();
    }
    return NULL;
}

void scheduler(){
    enum Policy current_policy = FIFO;

    while (!done()){
        struct proc *candidate = select_next(current_policy);

        if (candidate != NULL) {
            candidate->status = RUNNING;
            candidate->runtime += swtch(candidate);
        } else {
            idle();
        }
    }
}