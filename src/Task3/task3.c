#include <stdlib.h>
#include "task3.h"
#include "../utils/my_rand.h"
#include "../utils/timer.h"
#include "../utils/pth_ll_rwl.h"


const int MAX_KEY = 100000000;
int inserts_in_main, total_ops, thread_count, check;
double insert_percent, search_percent;

list_node_s *head;

void *current_rwlock;
pthread_rwlock_t rwlock;
TASK3_rwlock_t my_rwlock;
pthread_mutex_t count_mutex;


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

    while (rwlock->is_locked != 0 || rwlock->write_waiting != 0) {
        pthread_cond_wait(&rwlock->read_ok, &rwlock->mutex);
    }

    ++rwlock->read_count;

    pthread_mutex_unlock(&rwlock->mutex);

    return 0;
}

int TASK3_rwlock_wrlock(TASK3_rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->mutex);

    ++rwlock->write_waiting;

    while (rwlock->is_locked != 0 || rwlock->read_count != 0) {
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


void init_(void *raw_rwlock) {
    if (check == 0) {
        pthread_rwlock_init(raw_rwlock,NULL);
    } else {
        TASK3_rwlock_init(raw_rwlock);
    }
}

void destroy_(void *raw_rwlock) {
    if (check == 0) {
        pthread_rwlock_destroy(raw_rwlock);
    } else {
        TASK3_rwlock_destroy(raw_rwlock);
    }
}

void rdlock_(void *raw_rwlock) {
    if (check == 0) {
        pthread_rwlock_rdlock(raw_rwlock);
    } else {
        TASK3_rwlock_rdlock(raw_rwlock);
    }
}

void wrlock_(void *raw_rwlock) {
    if (check == 0) {
        pthread_rwlock_wrlock(raw_rwlock);
    } else {
        TASK3_rwlock_wrlock(raw_rwlock);
    }
}

void unlock_(void *raw_rwlock) {
    if (check == 0) {
        pthread_rwlock_unlock(raw_rwlock);
    } else {
        TASK3_rwlock_unlock(raw_rwlock);
    }
}

void* Thread_work(void *rank) {
    unsigned seed = (long)rank + 1;
    int my_member_count = 0, my_insert_count = 0, my_delete_count = 0;
    int const ops_per_thread = total_ops / thread_count;
    for (int i = 0; i < ops_per_thread; i++) {
        double const which_op = my_drand(&seed);
        int const val = my_rand(&seed) % MAX_KEY;
        if (which_op < search_percent) {
            rdlock_(current_rwlock);
            Member(&head, val);
            unlock_(current_rwlock);
            my_member_count++;
        } else if (which_op < search_percent + insert_percent) {
            wrlock_(current_rwlock);
            Insert(&head, val);
            unlock_(current_rwlock);
            my_insert_count++;
        } else {
            wrlock_(current_rwlock);
            Delete(&head, val);
            unlock_(current_rwlock);
            my_delete_count++;
        }
    }

    pthread_mutex_lock(&count_mutex);
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}


double TASK3_variant() {
    int attempts;
    unsigned seed = 1;
    head = create();

    long i = attempts = 0;
    while (i < inserts_in_main && attempts < 2 * inserts_in_main) {
        int const key = my_rand(&seed) % MAX_KEY;
        int const success = Insert(&head, key);
        attempts++;
        if (success)
            i++;
    }

    pthread_t *thread_handles;
    thread_handles = malloc(thread_count * sizeof(pthread_t));
    pthread_mutex_init(&count_mutex, NULL);
    init_(current_rwlock);
    double start, finish;
    GET_TIME(start);
    for (i = 0; i < thread_count; i++)
        pthread_create(&thread_handles[i], NULL, Thread_work, (void*)i);

    for (i = 0; i < thread_count; i++)
        pthread_join(thread_handles[i], NULL);
    GET_TIME(finish);

    Free_list(&head);


    destroy_(current_rwlock);
    pthread_mutex_destroy(&count_mutex);
    free(thread_handles);
    return finish - start;
}


double TASK3_run(int const num_threads, int const inserts_in_main_, int const total_ops_, double const search_percent_,
                 double const insert_percent_) {
    inserts_in_main = inserts_in_main_, total_ops = total_ops_, thread_count = num_threads;
    search_percent = search_percent_, insert_percent = insert_percent_;

    check = 0;
    current_rwlock = &rwlock;
    double const time_to_run = TASK3_variant(num_threads);

    check = 1;
    current_rwlock = &my_rwlock;
    double const time_to_run_my = TASK3_variant(num_threads);

    return time_to_run_my;
}
