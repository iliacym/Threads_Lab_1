#ifndef TASK3_H
#define TASK3_H
#include "pthread.h"

typedef struct TASK3_rwlock_t {
    pthread_mutex_t mutex;
    pthread_cond_t write_cond;
    pthread_cond_t read_cond;
    unsigned long long int write_wait, read_wait;
    int is_locked;
} TASK3_rwlock_t;

#endif
