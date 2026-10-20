#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/single_threaded.h>

#ifndef LABIRIT_PRELOAD_COUNT_CALLS
#define LABIRIT_PRELOAD_COUNT_CALLS 0
#endif

_Static_assert(RAND_MAX == 2147483647, "This experiment requires a 31-bit RAND_MAX");

/* One stream per process, as with libc rand(); not one stream per thread.
   glibc's single-thread flag lets the common sequential case avoid locking.
   Never write to __libc_single_threaded. */
static uint64_t state = 1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#if LABIRIT_PRELOAD_COUNT_CALLS
static unsigned long long calls;
static unsigned long long reseeds;
#endif

__attribute__((visibility("default")))
int rand(void)
{
    bool lock = !__libc_single_threaded;
    if (lock)
        pthread_mutex_lock(&mutex);
    uint64_t value = (state += UINT64_C(0x9e3779b97f4a7c15));
#if LABIRIT_PRELOAD_COUNT_CALLS
    calls++;
#endif
    if (lock)
        pthread_mutex_unlock(&mutex);

    value = (value ^ (value >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    value = (value ^ (value >> 27)) * UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31;
    /* 31 high bits: always in [0,RAND_MAX], no narrowing overflow. */
    return (int)(value >> 33);
}

__attribute__((visibility("default")))
void srand(unsigned int seed)
{
    bool lock = !__libc_single_threaded;
    if (lock)
        pthread_mutex_lock(&mutex);
    state = (uint64_t)seed;
#if LABIRIT_PRELOAD_COUNT_CALLS
    reseeds++;
#endif
    if (lock)
        pthread_mutex_unlock(&mutex);
}

#if LABIRIT_PRELOAD_COUNT_CALLS
__attribute__((destructor))
static void report_calls(void)
{
    bool lock = !__libc_single_threaded;
    if (lock)
        pthread_mutex_lock(&mutex);
    unsigned long long total_calls = calls;
    unsigned long long total_reseeds = reseeds;
    if (lock)
        pthread_mutex_unlock(&mutex);
    fprintf(stderr, "labirit preload: rand_calls=%llu srand_calls=%llu\n",
            total_calls, total_reseeds);
}
#endif
