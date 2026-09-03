#include <sys/times.h>
#include <unistd.h>

clock_t clock_add(clock_t lhs, clock_t rhs) {
    return lhs + rhs;
}

int clock_cmp(clock_t lhs, clock_t rhs) {
    if (lhs < rhs)
        return -1;
    if (lhs > rhs)
        return 1;
    return 0;
}

void busy_wait(clock_t t) {
    struct tms buffer;

    clock_t now = times(&buffer);
    clock_t then = clock_add(now, t);

    while (clock_cmp(now, then) < 0) {
        for (int i = 0; i < 10000; i++) {
        }

        now = times(&buffer);
    }
}

int main(void) {
    long ticks_per_second = sysconf(_SC_CLK_TCK);

    // Wait for 1 second
    clock_t wait = ticks_per_second;

    busy_wait(wait);

    return 0;
}

