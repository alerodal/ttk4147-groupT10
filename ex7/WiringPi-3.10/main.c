#define _GNU_SOURCE

#include <wiringPi.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PININ1  8
#define PINOUT1 9
#define PININ2  7
#define PINOUT2 15
#define PININ3  16
#define PINOUT3 1

void io_init(void) {
    wiringPiSetup();

    pinMode(PININ1, INPUT); pinMode(PINOUT1, OUTPUT);
    pinMode(PININ2, INPUT); pinMode(PINOUT2, OUTPUT);
    pinMode(PININ3, INPUT); pinMode(PINOUT3, OUTPUT);

    digitalWrite(PINOUT1, HIGH);
    digitalWrite(PINOUT2, HIGH);
    digitalWrite(PINOUT3, HIGH);
}

typedef struct {
    int input;
    int output;
} Task;


int set_cpu(int cpu_id) {
    cpu_set_t cpu;

    CPU_ZERO(&cpu);
    CPU_SET(cpu_id, &cpu);

    return pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set_t),
        &cpu
    );
}


void *responseTask(void *arg) {
    Task *task = (Task *)arg;

    // TASK B
    if (set_cpu(0) != 0) {
        printf("Failed to set CPU affinity\n");
        return NULL;
    }

    // TASK A
    /*
    while (1) {
        if (digitalRead(task->input) == LOW) {
            digitalWrite(task->output, LOW);
        } else {
            digitalWrite(task->output, HIGH);
        }
    }
    */



    struct timespec period = {
        .tv_sec = 0,
        .tv_nsec = 2000000
    };

    while (1) {

        if (digitalRead(task->input) == LOW) {
            digitalWrite(task->output, LOW);
        } else {
            digitalWrite(task->output, HIGH);
        }

        clock_nanosleep(CLOCK_MONOTONIC, 0, &period, NULL);
    }

    return NULL;
}


void *disturbanceTask(void *arg) {
    (void)arg;

    if (set_cpu(0) != 0) {
        printf("Failed to set CPU affinity\n");
        return NULL;
    }

    while (1) {
        asm volatile("" ::: "memory");
    }

    return NULL;
}


int main(int argc, char *argv[]) {
    io_init();

    Task tasks[3] = {
        {PININ1, PINOUT1},  // Channel A
        {PININ2, PINOUT2},  // Channel B
        {PININ3, PINOUT3}   // Channel C
    };

    pthread_t threads[3];

    // TASK A
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, responseTask, &tasks[i]);
    }

    // TASK C
    int numDisturbances = -1;

    if (argc > 1) {
        numDisturbances = atoi(argv[1]);
    }

    if (numDisturbances < 0 || numDisturbances > 100) {
        printf("Choose between 0 and 100 disturbance threads\n");
        scanf("%d", &numDisturbances);
    }

    pthread_t disturbances[100];

    for (int i = 0; i < numDisturbances; i++) {
        pthread_create(&disturbances[i], NULL, disturbanceTask, NULL);
    }

    printf("Running with %d disturbance threads\n", numDisturbances);
    // TASK C END

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
