#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>


int main(){

        int ns_max = 100;
        int histogram[ns_max];
        memset(histogram, 0, sizeof(int)*ns_max);

	struct tms buffer;
	long tixps = sysconf(_SC_CLK_TCK);

	for(int i = 0; i < 10*1000*1000; i++){
                clock_t t1 = times(&buffer);
		clock_t t2 = times(&buffer);

                long ns = (t2-t1) * 1000000000L/tixps;

                if(ns >= 0 && ns < ns_max){
                        histogram[ns]++;
                }
        }

        for(int i = 0; i < ns_max; i++){
                printf("%d\n", histogram[i]);
        }
        return 0;
}


