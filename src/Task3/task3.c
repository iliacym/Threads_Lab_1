#include "task3.h"
#include "stdio.h"

#define and &&
#define or ||
int BIBA = 0;

int TASK3_rwlock_init(TASK3_rwlock_t *rwlock) {
    int ret = 0;

    ret = pthread_mutex_init(&rwlock->mutex, NULL);
    if (ret != 0) {
        return ret;
    }

    ret = pthread_cond_init(&rwlock->write_ok, NULL);
    if (ret != 0) {
        return ret;
    }

    ret = pthread_cond_init(&rwlock->read_ok, NULL);
    if (ret != 0) {
        return ret;
    }

    rwlock->read_count = rwlock->write_waiting = 0;
    rwlock->is_locked = 0;

    return 0;
}

int TASK3_rwlock_destroy(TASK3_rwlock_t *rwlock) {
    int ret = 0;

    ret = pthread_mutex_destroy(&rwlock->mutex);
    if (ret != 0) {
        return ret;
    }

    ret = pthread_cond_destroy(&rwlock->write_ok);
    if (ret != 0) {
        return ret;
    }

    ret = pthread_cond_destroy(&rwlock->read_ok);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int TASK3_rwlock_rdlock(TASK3_rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);

    while (rwlock->is_locked != 0 or rwlock->write_waiting != 0) {
        pthread_cond_wait(&rwlock->read_ok, &rwlock->mutex);
    }

    ++rwlock->read_count;

    pthread_mutex_unlock(&rwlock->mutex);

    return 0;
}

int TASK3_rwlock_wrlock(TASK3_rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);

    ++rwlock->write_waiting;

    while (rwlock->is_locked != 0 or rwlock->read_count != 0) {
        pthread_cond_wait(&rwlock->write_ok, &rwlock->mutex);
    }

    --rwlock->write_waiting;
    rwlock->is_locked = 1;

    pthread_mutex_unlock(&rwlock->mutex);

    return 0;
}

int TASK3_rwlock_unlock(TASK3_rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);

    if (rwlock->read_count != 0) {
        --rwlock->read_count;

        if (rwlock->read_count == 0) {
            pthread_cond_signal(&rwlock->write_ok);
        }
    } else if (rwlock->is_locked != 0) {
        rwlock->is_locked = 0;

        if (rwlock->write_waiting != 0) {
            pthread_cond_signal(&rwlock->write_ok);
        } else {
            pthread_cond_broadcast(&rwlock->read_ok);
        }
    } else {
        pthread_mutex_unlock(&rwlock->mutex);

        return -1;
    }

    pthread_mutex_unlock(&rwlock->mutex);

    return 0;
}
