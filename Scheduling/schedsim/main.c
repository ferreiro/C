#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "sched.h"


void usage(int argc, char *argv[])
{
    fprintf(stderr,"Usage: %s -i <input-file> [options]\n",argv[0]);
    fprintf(stderr,"\nList of options:\n");
    fprintf(stderr,"\t-h: Displays this help message\n");
    fprintf(stderr,"\t-n <cpus>: Sets number of CPUs for the simulator (default 1)\n");
    fprintf(stderr,"\t-m <nsteps>: Sets the maximum number of simulation steps (default 50)\n");
    fprintf(stderr,"\t-s <scheduler>: Selects the scheduler for the simulation (default RR)\n");
    fprintf(stderr,"\t-d: Turns on debug mode (default OFF)\n");
    fprintf(stderr,"\t-p: Selects the preemptive version of SJF or PRIO (only if they are selected with -s)\n");
    fprintf(stderr,"\t-t <msecs>: Selects the tick delay for the simulator (default 250)\n");
    fprintf(stderr,"\t-q <quantum>: Set up the timeslice or quantum for the RR algorithm (default 3)\n");
    fprintf(stderr,"\t-l <period>: Set up the load balancing period (specified in simulation steps, default 5)\n");
    fprintf(stderr,"\t-L: List available scheduling algorithms\n");
}


int main(int argc, char *argv[])
{
    FILE* inputfile = NULL;
    char optc;
    slist_t slist;
    sched_class_t* selected_scheduler=&rr_sched;
    int i=0;

    /* Bucle de procesamiento de opciones */
    while ((optc = getopt(argc, argv, "+hi:n:s:dt:m:q:pl:L")) != (char)-1) {
        switch (optc) {
        case 'h':
            usage(argc,argv);
            exit(0);
            break;
        case 'i':
            if((inputfile = fopen(optarg, "r")) == NULL) {
                perror("Can't open the input file");
                exit(1);
            }
            break;
        case 'n':
            nr_cpus=atoi(optarg);
            break;
        case 'm':
            max_simulation_steps=atoi(optarg);
            break;
        case 't':
            tick_delay=atoi(optarg)*1000;
            break;
        case 'd':
            debug_mode=TRUE;
            break;
        case 'p':
            preemptive_scheduler=TRUE;
            break;
        case 's':
            i=0;

            /* Search scheduler by name in the set */
            while(strcmp(available_schedulers[i].sched_name,optarg)!=0) {
                i++;
            }

            if (i>=NR_AVAILABLE_SCHEDULERS) {
                fprintf(stderr,"Scheduler %s is not available",optarg);
                exit(1);
            }

            selected_scheduler=available_schedulers[i].sched_class;

            break;
        case 'L':
            i=0;

            printf("Available schedulers:\n");
            for (i=0; i<NR_AVAILABLE_SCHEDULERS ; i++) {
                printf ("%s\n",available_schedulers[i].sched_name);
            }
            if (inputfile)
                fclose(inputfile);

            exit(0);
            break;
        case 'q':
            rr_quantum=atoi(optarg);
            break;
        case 'l':
            load_balancing_period=atoi(optarg);
            break;
        default:
            fprintf(stderr, "Wrong option: -%c\n", optc);
            usage(argc,argv);
            exit(1);
        }
    }

    if (!inputfile) {
        fprintf(stderr,"No input file was provided\n");
        fprintf(stderr,"Usage: %s -i <input-file> [options]\n",argv[0]);
        exit(1);
    }

    slist=read_task_list_from_file(inputfile);
    sched_start(&slist,selected_scheduler);
    return 0;
}
