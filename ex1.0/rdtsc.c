#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

uint64_t rdtsc(void){
    uint64_t val;
    asm volatile("isb; mrs %0, cntvct_el0; isb; " : "=r"(val) :: "memory"); 
    // You can check the current CPU frequency with $sudo dmesg | grep MHz
    return val;
}

int main(void){
	int ns_max = 500;
	int histogram[ns_max];
	memset(histogram, 0, sizeof(histogram));

	for(int i = 0; i < 10*1000*1000; i++){
    
    		uint64_t  t1 = rdtsc();    
		uint64_t  t2 = rdtsc();
    
    		uint64_t ns = (t2 - t1) * 1000000000ULL/54000000ULL;
    
    		if(ns >= 0 && ns < ns_max){
        		histogram[ns]++;
    		}
	}

	for(int i = 0; i < ns_max; i++){
    		printf("%d\n", histogram[i]);
	}
	
	return 0;
}
