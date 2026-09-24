#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define N 5

pthread_mutex_t forks[N];
pthread_barrier_t barrier;

void *philosopher(void *arg)
{
    int id = *(int *)arg;

    int left = id;
    int right = (id + 1) % N;

    int first;
    int second;

    if (left < right) {
        first = left;
        second = right;
    } else {
        first = right;
        second = left;
    }


    pthread_barrier_wait(&barrier);

    printf("Philosopher %d waits for fork %d\n", id, first);
    pthread_mutex_lock(&forks[first]);

    printf("Philosopher %d got fork %d\n", id, first);

    pthread_mutex_lock(&forks[second]);

    printf("Philosopher %d got fork %d and is eating\n",
           id, second);

    usleep(100000);

    pthread_mutex_unlock(&forks[second]);
    pthread_mutex_unlock(&forks[first]);

    printf("Philosopher %d finished eating\n", id);

    return NULL;
}

int main(void)
{
    pthread_t threads[N];
    int ids[N];

    pthread_barrier_init(&barrier, NULL, N);

    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        ids[i] = i;

        pthread_create(
            &threads[i],
            NULL,
            philosopher,
            &ids[i]
        );
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}
