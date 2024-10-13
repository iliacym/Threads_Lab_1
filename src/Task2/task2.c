#include "task2.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

double TASK2_R = 2;
double TASK2_EPS = 1e-6;
int TASK2_MAX_ITER = 20000;
int CURR_STEP;
int MAX_STEP;
pthread_mutex_t mutex_bar, mutex_file;
FILE *file;
int const buff_size = 256 * 1024;

typedef struct TASK2_POINT {
    double x, y, color;
} TASK2_POINT;

typedef struct TASK2_POINTS {
    int num_points;
    TASK2_POINT **points;
} TASK2_POINTS;

typedef struct DATA {
    int rank, threads;
    TASK2_POINTS *points;
} DATA;

TASK2_POINTS *create_points(int const num_points) {
    TASK2_POINTS *points = calloc(1, sizeof(TASK2_POINTS));
    points->num_points = num_points;
    points->points = calloc(num_points, sizeof(TASK2_POINT));

    for (int i = 0; i < num_points; ++i) {
        points->points[i] = calloc(1, sizeof(TASK2_POINT));
    }

    return points;
}

void delete_points(TASK2_POINTS *points) {
    for (int i = 0; i < points->num_points; ++i) {
        free(points->points[i]);
    }

    free(points->points);
    free(points);
}

void get_points(TASK2_POINTS const *points) {
    srand(time(0));

    for (int i = 0; i < points->num_points; ++i) {
        double const radius = sqrt((double) rand() / RAND_MAX) * TASK2_R;
        double const angle = (double) rand() / RAND_MAX * 2 * M_PI;

        double const x = radius * cos(angle), y = radius * sin(angle);

        points->points[i]->x = x;
        points->points[i]->y = y;
    }
}

double sqr(TASK2_POINT const *p) {
    return p->x * p->x + p->y * p->y;
}

double l2(TASK2_POINT const *p1, TASK2_POINT const *p2) {
    TASK2_POINT point;

    point.x = p2->x - p1->x;
    point.y = p2->y - p1->y;

    return sqrt(sqr(&point));
}


void *mandelbrot_set(void *raw_data) {
    DATA const data = *(DATA *) raw_data;
    TASK2_POINTS const *points = data.points;
    int const threads = data.threads, rank = data.rank;

    int const num_points = points->num_points,
            start = num_points / threads * rank,
            end = rank == threads - 1 ? num_points : num_points / threads * (rank + 1);

    for (int i = start; i < end; ++i) {
        TASK2_POINT *point = points->points[i],
                start_point;

        start_point.x = point->x;
        start_point.y = point->y;
        int iter = 0;

        for (int j = 0; j < TASK2_MAX_ITER; ++j) {
            TASK2_POINT prev_point;

            prev_point.x = point->x;
            prev_point.y = point->y;

            point->x = point->x * point->x + start_point.x - point->y * point->y;
            point->y = 2 * prev_point.x * point->y + start_point.y;

            if (sqr(point) >= TASK2_R * TASK2_R) {
                iter = TASK2_MAX_ITER - 1 - j;
                break;
            }

            if (l2(&prev_point, point) <= TASK2_EPS) {
                break;
            }
        }
        point->color = (double) iter / TASK2_MAX_ITER;

        pthread_mutex_lock(&mutex_bar);
        ++CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);
    }

    return NULL;
}

void *progress(void *i) {
    while (1) {
        pthread_mutex_lock(&mutex_bar);
        int const step = CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);

        if (step >= MAX_STEP) {
            printf("\r100%%\n");
            break;
        }

        printf("\r%.0lf%%", (double) step / MAX_STEP * 100);
        usleep(200000);
    }

    return NULL;
}

void *write_file(void *raw_data) {
    DATA const data = *(DATA *) raw_data;
    TASK2_POINTS const *points = data.points;
    int const threads = data.threads, rank = data.rank;
    int const num_points = points->num_points,
            start = num_points / threads * rank,
            end = rank == threads - 1 ? num_points : num_points / threads * (rank + 1);;


    char *buffer = calloc(buff_size, sizeof(char)), tmp[100];
    int curr_pos = 0;
    for (int i = start; i < end; ++i) {
        int const str_len = sprintf(tmp, "%e;%e;%e\n", points->points[i]->x, points->points[i]->y, points->points[i]->color);

        if (curr_pos + str_len >= buff_size) {
            pthread_mutex_lock(&mutex_file);
            fwrite(buffer, sizeof(char), curr_pos, file);
            pthread_mutex_unlock(&mutex_file);

            curr_pos = 0;
        }

        memcpy(buffer + curr_pos, tmp, str_len);
        curr_pos += str_len;

        pthread_mutex_lock(&mutex_bar);
        ++CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);
    }

    if (curr_pos != 0) {
        pthread_mutex_lock(&mutex_file);
        fwrite(buffer, sizeof(char), curr_pos, file);
        pthread_mutex_unlock(&mutex_file);
    }

    free(buffer);
}

void TASK2_run(int const num_points, int num_threads) {
    TASK2_POINTS *points = create_points(num_points);
    get_points(points);

    pthread_t *threads = calloc(num_threads + 1, sizeof(pthread_t));
    DATA *data = calloc(num_threads, sizeof(DATA));

    CURR_STEP = 0;
    MAX_STEP = num_points;
    pthread_mutex_init(&mutex_bar, NULL);
    pthread_mutex_init(&mutex_file, NULL);
    printf("Mandelbrot Progress:\n");

    for (int i = 0; i < num_threads; ++i) {
        data[i].points = points;
        data[i].rank = i;
        data[i].threads = num_threads;

        pthread_create(&threads[i], NULL, mandelbrot_set, &data[i]);
    }

    pthread_create(&threads[num_threads], NULL, progress, NULL);

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(threads[num_threads], NULL);
    printf("Mandelbrot Completed\n");

    file = fopen("task2_results.csv", "w");
    setvbuf(file, NULL, _IOFBF, buff_size);

    printf("File progress:\n");
    num_threads = 2;
    for (int i = 0; i < num_threads; ++i) {
        data[i].points = points;
        data[i].rank = i;
        data[i].threads = num_threads;

        pthread_create(&threads[i], NULL, write_file, &data[i]);
    }
    CURR_STEP = 0;
    pthread_create(&threads[num_threads], NULL, progress, NULL);

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    pthread_join(threads[num_threads], NULL);

    printf("File completed\n");

    pthread_mutex_destroy(&mutex_file);
    pthread_mutex_destroy(&mutex_bar);
    fclose(file);
    free(data);
    free(threads);
    delete_points(points);
}
