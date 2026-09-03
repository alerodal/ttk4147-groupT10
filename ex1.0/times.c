#include <string.h>
#include <stdio.h>
#include <sys/times.h>
#include <unistd.h>

int main(void) {
    int histogram[10];
    memset(histogram, 0, sizeof(histogram));

    struct tms buffer;

    for (int i = 0; i < 10 * 1000 * 1000; i++) {
        clock_t t1 = times(&buffer);
        clock_t t2 = times(&buffer);

        clock_t ticks = t2 - t1;

        if (ticks >= 0 && ticks < 10) {
            histogram[ticks]++;
        }
    }

    for (int i = 0; i < 10; i++) {
        printf("%d\n", histogram[i]);
    }

    return 0;
}
