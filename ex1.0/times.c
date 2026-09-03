#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

int main(void) {
    int histogram[10];
    memset(histogram, 0, sizeof(histogram));

    struct tms buffer;

    for (int i = 0; i < 1000000; i++) {
        clock_t t1 = times(&buffer);
        clock_t t2 = times(&buffer);

        clock_t diff = t2 - t1;

        if (diff >= 0 && diff < 10) {
            histogram[diff]++;
        }
    }

    for (int i = 0; i < 10; i++) {
        printf("%d\n", histogram[i]);
    }

    return 0;
}
