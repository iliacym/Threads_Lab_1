#include <pthread.h>
#include "task1.h"
#include "../utils/my_rand.h"

static unsigned long long int total_hits = 0;
pthread_mutex_t mutex;

typedef struct {
    unsigned long long int trials;
} thread_data_t;

void *monte_carlo_pi(void *args) {
    thread_data_t *data = (thread_data_t *) args;
    unsigned long long int hits = 0;

    unsigned int seed = time(NULL) ^ pthread_self();
    for (unsigned long long int i = 0; i < data->trials; i++) {
        double x = my_drand(&seed) * 2.0 - 1.0;
        double y = my_drand(&seed) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) {
            hits++;
        }
    }


    pthread_mutex_lock(&mutex);
    total_hits += hits;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

double TASK1_run(unsigned long long int ntrials, int nthreads) {
    pthread_t threads[nthreads];
    thread_data_t thread_data[nthreads];
    unsigned long long int trials_per_thread = ntrials / nthreads;
    total_hits = 0;
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < nthreads; i++) {
        thread_data[i].trials = trials_per_thread;
        pthread_create(&threads[i], NULL, monte_carlo_pi, &thread_data[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    return 4.0 * (double) total_hits / (double) ntrials;
}
