#include <sys/time.h>
#include <time.h>

int gettimeofday(struct timeval *tv, __attribute__((unused)) void *tz) {
    struct timespec tp;
    int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    tv->tv_sec = tp.tv_sec;
    tv->tv_usec = tp.tv_nsec * 0.001;
    return ret;
}