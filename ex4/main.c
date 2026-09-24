#include <pthread.h>
#include <stdio.h>

long global = 0;

pthread_barrier_t barr;

void* fn(void* args){
	long local = 0;

	pthread_barrier_wait(&barr);

	for(long i = 0; i < 1000000; i++){
		global++;
		local++;
	}

	printf("Global: %ld, Local: %ld\n", global, local);

	return NULL;
}

int main(){
	pthread_t thread1;
	pthread_t thread2;

	pthread_barrier_init(&barr, NULL, 2);

	pthread_create(&thread1, NULL, fn, NULL);
	pthread_create(&thread2, NULL, fn, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	printf("Final global: %ld\n", global);

	return 0;
}
