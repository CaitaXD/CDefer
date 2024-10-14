#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>
#include <errno.h>


#ifndef MONITOR_API
#   define MONITOR_API static inline
#endif

struct monitor_t {
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

MONITOR_API struct monitor_t monitor_new();

MONITOR_API void monitor_wait_one(struct monitor_t *monitor);

MONITOR_API int monitor_timed_wait_one(struct monitor_t *monitor, const struct timespec *abstime);

MONITOR_API void monitor_notify_one(struct monitor_t *monitor);

MONITOR_API void monitor_notify_all(struct monitor_t *monitor);

MONITOR_API void (monitor_lock)(struct monitor_t *monitor);

MONITOR_API int (monitor_try_lock)(struct monitor_t *monitor);

MONITOR_API void (monitor_unlock)(struct monitor_t *monitor);

#define monitor_lock(monitor__) \
    using((monitor_lock)((monitor__)), (monitor_unlock)((monitor__)))

#define monitor_try_lock(monitor__) \
    using(auto __locked = (monitor_try_lock)((monitor__)), if(__locked) (monitor_unlock)((monitor__)))\
        if(__locked)

#define monitor_wait(monitor__, expression__) \
    ({ while (!(expression__)) (monitor_wait_one)((monitor__)); })

#define monitor_timed_wait(monitor__, expression__, abstime__) \
    ({ while (!(expression__)) (monitor_timed_wait_one)((monitor__), (abstime__)); })

#define micro_seconds(x__) (struct timespec){ .tv_sec = x__ / 1000, .tv_nsec = (x__ % 1000) * 1000000 }
#define nano_seconds(x__) (struct timespec){ .tv_sec = x__ / 1000000000, .tv_nsec = (x__ % 1000000000) * 1000 }
#define milli_seconds(x__) (struct timespec){ .tv_sec = x__ / 1000000, .tv_nsec = (x__ % 1000000) * 1000 }
#define seconds(x__) (struct timespec){ .tv_sec = x__, .tv_nsec = 0 }

#define builin_pause() __builtin_ia32_pause()

#define busy_wait(expression__) \
    while (!(expression__))\
        finalizer_expression(builin_pause();)

#define monitor_break defer_break

struct monitor_t monitor_new() {
    return (struct monitor_t){
        .lock = PTHREAD_MUTEX_INITIALIZER,
        .cond = PTHREAD_COND_INITIALIZER
    };
}

void (monitor_lock)(struct monitor_t *monitor) {
    pthread_mutex_lock(&monitor->lock);
}

void (monitor_unlock)(struct monitor_t *monitor) {
    pthread_mutex_unlock(&monitor->lock);
}

void (monitor_wait_one)(struct monitor_t *monitor) {
    pthread_cond_wait(&monitor->cond, &monitor->lock);
}

void (monitor_notify_one)(struct monitor_t *monitor) {
    pthread_cond_signal(&monitor->cond);
}

void (monitor_notify_all)(struct monitor_t *monitor) {
    pthread_cond_broadcast(&monitor->cond);
}

int (monitor_try_lock)(struct monitor_t *monitor) {
    return pthread_mutex_trylock(&monitor->lock);
}

int monitor_timed_wait_one(struct monitor_t *monitor, const struct timespec *abstime) {
    return pthread_cond_timedwait(&monitor->cond, &monitor->lock, abstime);
}

#endif //MONITOR_H
