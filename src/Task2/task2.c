#include "task2.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "../utils/timer.h"

// long double screen[4] = {-0.811526786, -0.811520633, 0.1845268428, 0.1845318705}; //star
long double screen[4] = {-2, 1, -1, 1}; //default
// long double screen[4] = {0.335590236, 0.335767595, -0.38698134, -0.38681068}; //sun 1000 +0.15
// long double screen[4] = {-0.8116076793, -0.8116060273, 0.18460990711, 0.18461113012}; //shima 1600
// long double screen[4] = {-0.81405528322, -0.81405516201,  0.189598755165, 0.189598844697};//neuron 1600 +0.35
// long double screen[4] = {-0.109314378533, -0.109314351037,  0.8950811785628, 0.8950812006340};//virus 1600 +0.35
unsigned long long int TASK2_MAX_ITER = 1000, CURR_STEP, MAX_STEP, PPX, PPY;

pthread_mutex_t mutex_bar;
int const buff_size = 256 * 1024;

typedef struct TASK2_POINT {
    long double x, y;
    unsigned long long int color;
} TASK2_POINT;

typedef struct TASK2_COORD_POINT {
    unsigned long long int x, y;
} TASK2_COORD_POINT;

typedef struct TASK2_POINTS {
    unsigned long long int num_points;
    TASK2_POINT **points;
    TASK2_COORD_POINT **coords;
} TASK2_POINTS;

typedef struct DATA {
    unsigned long long int rank, threads, file_number;
    TASK2_POINTS *points;
} DATA;

TASK2_POINTS* create_points(unsigned long long int const num_points) {
    TASK2_POINTS *points = calloc(1, sizeof(TASK2_POINTS));
    points->num_points = num_points;

    points->points = calloc(num_points, sizeof(TASK2_POINT*));
    points->coords = calloc(num_points, sizeof(TASK2_COORD_POINT*));
    for (unsigned long long int i = 0; i < num_points; ++i) {
        points->points[i] = calloc(1, sizeof(TASK2_POINT));
        points->coords[i] = calloc(1, sizeof(TASK2_COORD_POINT));
    }

    return points;
}

void delete_points(TASK2_POINTS *points) {
    for (unsigned long long int i = 0; i < points->num_points; ++i) {
        free(points->points[i]);
        free(points->coords[i]);
    }

    free(points->points);
    free(points->coords);
    free(points);
}

void get_ppxy(unsigned long long int const num_points) {
    long double const wight = screen[1] - screen[0], height = screen[3] - screen[2];
    unsigned long long int const ppx = (unsigned long long int)sqrtl(num_points * wight / height),
                                 ppy = (unsigned long long int)sqrtl(num_points * height / wight);

    PPX = ppx;
    PPY = ppy;
}

void get_points(TASK2_POINTS const *points,
                unsigned long long int *x_it,
                unsigned long long int *y_it,
                unsigned long long int *write_points,
                unsigned long long int const points_limit,
                unsigned long long int const num_points) {
    long double const wight = screen[1] - screen[0], height = screen[3] - screen[2], mid_x = screen[0], mid_y = screen[
                          2];
    unsigned long long int const ppx = (unsigned long long int)sqrtl(num_points * wight / height),
                                 ppy = (unsigned long long int)sqrtl(num_points * height / wight);
    long double const step_x = wight / (ppx - 1), step_y = height / (ppy - 1);
    unsigned long long int i = 0;

    while (*x_it < ppx) {
        long double const x = screen[0] + step_x * *x_it;
        while (*y_it < ppy) {
            long double const y = screen[2] + step_y * *y_it;
            points->points[i]->x = x;
            points->points[i]->y = y;
            points->coords[i]->x = *x_it;
            points->coords[i]->y = (*y_it)++;
            ++i;

            ++*write_points;
            if (*write_points >= points_limit) {
                return;
            }
        }
        *y_it = 0;
        ++*x_it;
    }

    // for (unsigned long long int j = i; j < points->num_points; ++j) {
    //     points->points[j]->x = mid_x;
    //     points->points[j]->y = mid_y;
    // }
}

long double sqr(TASK2_POINT const *p) {
    return p->x * p->x + p->y * p->y;
}

void* mandelbrot_set(void *raw_data) {
    DATA const data = *(DATA*)raw_data;
    TASK2_POINTS const *points = data.points;
    unsigned long long int const threads = data.threads, rank = data.rank;

    unsigned long long int const num_points = points->num_points,
                                 start = num_points / threads * rank,
                                 end = rank == threads - 1 ? num_points : num_points / threads * (rank + 1);

    for (unsigned long long int i = start; i < end; ++i) {
        TASK2_POINT *point = points->points[i],
                    start_point;

        start_point.x = point->x;
        start_point.y = point->y;
        unsigned long long int iter = 0;

        for (unsigned long long int j = 0; j < TASK2_MAX_ITER; ++j) {
            TASK2_POINT prev_point;

            prev_point.x = point->x;
            prev_point.y = point->y;

            point->x = point->x * point->x + start_point.x - point->y * point->y;
            point->y = 2 * prev_point.x * point->y + start_point.y;

            if (sqr(point) > 4) {
                iter = TASK2_MAX_ITER - 1 - j;
                break;
            }
        }

        point->color = iter;
        point->x = start_point.x;
        point->y = start_point.y;

        pthread_mutex_lock(&mutex_bar);
        ++CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);
    }

    return NULL;
}

void* progress(void *i) {
    while (1) {
        pthread_mutex_lock(&mutex_bar);
        unsigned long long int const step = CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);

        if (step >= MAX_STEP) {
            printf("\r100%%\n");
            break;
        }

        printf("\r%.0Lf%%", (long double)step / MAX_STEP * 100);
        usleep(200000);
    }

    return NULL;
}

unsigned long long int int_to_str(long long int number, char *tmp) {
    unsigned long long int i = 0;

    do {
        tmp[i++] = '0' + (char)(number % 10);
        number /= 10;
    } while (number != 0);

    for (unsigned long long int j = 0; j < i / 2; j++) {
        char const temp = tmp[j];
        tmp[j] = tmp[i - j - 1];
        tmp[i - j - 1] = temp;
    }

    return i;
}

unsigned long long int double_to_str(long double const number, char *tmp) {
    long double exp = floorl(log10l(fabsl(number)));
    if (isinf(exp)) {
        exp = 0;
    }

    long double normalized_number = fabsl(number) * powl(10, -exp);
    unsigned long long int last_index = 0;

    if (number < 0) {
        tmp[last_index++] = '-';
    }

    for (unsigned long long int i = 0; i < 10; ++i) {
        if (i != 1) {
            unsigned long long int const int_part = (unsigned long long int)normalized_number;

            normalized_number -= int_part;
            normalized_number *= 10;

            tmp[last_index] = '0' + (char)int_part;
        } else {
            tmp[last_index] = '.';
        }

        ++last_index;
    }

    tmp[last_index++] = 'e';
    if (exp >= 0) {
        tmp[last_index++] = '+';
    } else {
        tmp[last_index++] = '-';
    }

    long long int const int_exp = (long long int)fabsl(exp);

    last_index += int_to_str(int_exp, tmp + last_index);
    tmp[last_index] = '\0';

    return last_index;
}

void* write_file(void *raw_data) {
    DATA const data = *(DATA*)raw_data;
    TASK2_POINTS const *points = data.points;
    unsigned long long int const threads = data.threads, rank = data.rank;
    unsigned long long int const num_points = points->num_points,
                                 start = num_points / threads * rank,
                                 end = rank == threads - 1 ? num_points : num_points / threads * (rank + 1),
                                 file_number = data.file_number;

    char file_name[100];
    sprintf(file_name, "results/task2_coords_%llu.csv", file_number);
    FILE *file = fopen(file_name, "wb");
    setvbuf(file, NULL, _IOFBF, buff_size);
    fprintf(file, "%llu\n", end - start + 1);

    char *buffer = calloc(buff_size, sizeof(char)), tmp[100] = {0};
    unsigned long long int curr_pos = 0;
    for (unsigned long long int i = start; i < end; ++i) {
        unsigned long long int str_len = 0;

        str_len = int_to_str((long long int)roundl(points->coords[i]->x), tmp);
        tmp[str_len++] = ';';
        str_len += int_to_str((long long int)roundl(points->coords[i]->y), tmp + str_len);
        tmp[str_len++] = ';';
        str_len += double_to_str((long double)points->points[i]->color / (TASK2_MAX_ITER - 1), tmp + str_len);
        tmp[str_len++] = '\n';

        if (curr_pos + str_len >= buff_size) {
            fwrite(buffer, sizeof(char), curr_pos, file);

            curr_pos = 0;
        }

        if (str_len != 0) {
            memcpy(buffer + curr_pos, tmp, str_len);
            curr_pos += str_len;
        }

        pthread_mutex_lock(&mutex_bar);
        ++CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);
    }

    if (curr_pos != 0) {
        fwrite(buffer, sizeof(char), curr_pos, file);
    }
    fclose(file);
    free(buffer);

    return NULL;
}

void TASK2_set_iter(unsigned long long int const iter) {
    TASK2_MAX_ITER = iter;
}

void TASK2_set_borders(long double const x0, long double const x1, long double const y0, long double const y1) {
    screen[0] = x0;
    screen[1] = x1;
    screen[2] = y0;
    screen[3] = y1;
}

double TASK2_run(unsigned long long int const num_points,
                 unsigned long long int const batch_size,
                 int const num_threads) {
#   ifdef LINUX
    mkdir("results", 0777);
#   else
    mkdir("results");
#   endif

    TASK2_POINTS *points = create_points(batch_size);
    get_ppxy(num_points);

    pthread_t *threads = calloc(num_threads + 1, sizeof(pthread_t));
    DATA *data = calloc(num_threads, sizeof(DATA));

    pthread_mutex_init(&mutex_bar, NULL);

    FILE *file = fopen("results/task2_coords_info.csv", "wb");
    setvbuf(file, NULL, _IOFBF, buff_size);

    unsigned long long int batch_count = num_points / batch_size, x_it = 0, y_it = 0, write_points = 0;
    if (num_points % batch_size != 0) {
        ++batch_count;
    }

    fprintf(file, "%llu;%llu;%llu;%llu\n", num_points, PPX, PPY, batch_count * num_threads);
    fclose(file);


    double start, finish, elapsed_time = 0;

    for (unsigned long long int i = 0; i < batch_count; ++i) {
        printf("Mandelbrot progress (Batch %llu of %llu):\n", i + 1, batch_count);

        CURR_STEP = 0;

        GET_TIME(start)
        if (i != batch_count - 1) {
            get_points(points, &x_it, &y_it, &write_points, batch_size * (i + 1), num_points);
            MAX_STEP = batch_size;
        } else {
            if (num_points % batch_size != 0) {
                get_points(points, &x_it, &y_it, &write_points, batch_size * i + num_points % batch_size, num_points);
                MAX_STEP = num_points % batch_size;
            } else {
                get_points(points, &x_it, &y_it, &write_points, batch_size * (i + 1), num_points);
                MAX_STEP = batch_size;
            }
        }

        for (int j = 0; j < num_threads; ++j) {
            data[j].points = points;
            data[j].rank = j;
            data[j].threads = num_threads;

            pthread_create(&threads[j], NULL, mandelbrot_set, &data[j]);
        }

        pthread_create(&threads[num_threads], NULL, progress, NULL);

        for (int j = 0; j < num_threads; ++j) {
            pthread_join(threads[j],NULL);
        }
        GET_TIME(finish);

        elapsed_time += finish - start;

        pthread_join(threads[num_threads], NULL);
        printf("Mandelbrot completed (Batch %llu of %llu):\n", i + 1, batch_count);

        printf("File progress (Batch %llu of %llu):\n", i + 1, batch_count);
        CURR_STEP = 0;
        for (int j = 0; j < num_threads; ++j) {
            data[j].points = points;
            data[j].rank = j;
            data[j].threads = num_threads;
            data[j].file_number = num_threads * i + j;

            pthread_create(&threads[j], NULL, write_file, &data[j]);
        }
        pthread_create(&threads[num_threads], NULL, progress, NULL);

        for (int j = 0; j < num_threads; ++j) {
            pthread_detach(threads[j]);
        }
        pthread_join(threads[num_threads], NULL);

        printf("File completed (Batch %llu of %llu):\n", i + 1, batch_count);
    }

    pthread_mutex_destroy(&mutex_bar);
    free(data);
    free(threads);
    delete_points(points);

    return elapsed_time;
}
