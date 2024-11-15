#ifndef SWITCH_H
#define SWITCH_H

#include "sched.h"

struct proc_type {
    double mean_cpu_time;
    double unblocking_probability;
};

#define CPU1 {25.0, 0.8}
#define CPU2 {40.0, 1.0}
#define IO1 {15.0, 0.5}
#define IO2 {10.0, 0.4}

void init_simulation(int steps);
int done();
int swtch(struct proc* target);
int idle();
void print_stats();

#endif