#ifndef TASK3_H
#define TASK3_H
#include "pthread.h"

typedef struct TASK3_rwlock_t {
    pthread_mutex_t mutex;
    pthread_cond_t write_ok;
    pthread_cond_t read_ok;
    unsigned long long int read_count, write_waiting;
    int is_locked;
} TASK3_rwlock_t;

int TASK3_rwlock_init(TASK3_rwlock_t *rwlock);

int TASK3_rwlock_destroy(TASK3_rwlock_t *rwlock);

int TASK3_rwlock_rdlock(TASK3_rwlock_t *rwlock);

int TASK3_rwlock_wrlock(TASK3_rwlock_t *rwlock);

int TASK3_rwlock_unlock(TASK3_rwlock_t *rwlock);

int TASK3_run_rwlock(int const num_threads);

int TASK3_run(int const num_threads);

#endif
