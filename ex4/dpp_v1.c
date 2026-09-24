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

    printf("Philosopher %d picks up left fork %d\n", id, left);

    pthread_mutex_lock(&forks[left]);


    pthread_barrier_wait(&barrier);

    printf("Philosopher %d tries to pick up right fork %d\n",
           id, right);

    pthread_mutex_lock(&forks[right]);

    printf("Philosopher %d is eating\n", id);

    usleep(100000);

    pthread_mutex_unlock(&forks[right]);
    pthread_mutex_unlock(&forks[left]);

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
        pthread_create(&threads[i], NULL, philosopher, &ids[i]);
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
