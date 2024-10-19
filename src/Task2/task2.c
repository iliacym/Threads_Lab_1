#include "task2.h"
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

double screen[4] = {-0.811526786, -0.811520633, 0.1845268428, 0.1845318705};
int TASK2_MAX_ITER = 1000, CURR_STEP, MAX_STEP, PPX, PPY;
int TASK2_SAVE_COORDS = 1;
int TASK2_SAVE_POINTS = 0;
int write_mode = 0;

pthread_mutex_t mutex_bar, mutex_file;
FILE* file;
int const buff_size = 256 * 1024;

typedef struct TASK2_POINT {
    double x, y;
    int color;
} TASK2_POINT;

typedef struct TASK2_COORD_POINT {
    int x, y;
} TASK2_COORD_POINT;

typedef struct TASK2_POINTS {
    int num_points;
    TASK2_POINT** points;
    TASK2_COORD_POINT** coords;
} TASK2_POINTS;

typedef struct DATA {
    int rank, threads;
    TASK2_POINTS* points;
} DATA;

TASK2_POINTS* create_points(int const num_points) {
    TASK2_POINTS* points = calloc(1, sizeof(TASK2_POINTS));
    points->num_points = num_points;

    points->points = calloc(num_points, sizeof(TASK2_POINT*));
    points->coords = calloc(num_points, sizeof(TASK2_COORD_POINT*));
    for (int i = 0; i < num_points; ++i) {
        points->points[i] = calloc(1, sizeof(TASK2_POINT));
        points->coords[i] = calloc(1, sizeof(TASK2_COORD_POINT));
    }

    return points;
}

void delete_points(TASK2_POINTS* points) {
    for (int i = 0; i < points->num_points; ++i) {
        free(points->points[i]);
        free(points->coords[i]);
    }

    free(points->points);
    free(points->coords);
    free(points);
}

void get_points(TASK2_POINTS const* points) {
    double const wight = screen[1] - screen[0], height = screen[3] - screen[2], mid_x = screen[0], mid_y = screen[2];
    int const ppx = (int)sqrt(points->num_points * wight / height), ppy = (int)
                  sqrt(points->num_points * height / wight);
    double const step_x = wight / (ppx - 1), step_y = height / (ppy - 1);
    int i = 0, x_it = 0;

    while (x_it < ppx) {
        int y_it = 0;
        double const x = screen[0] + step_x * x_it;
        while (y_it < ppy) {
            double const y = screen[2] + step_y * y_it;
            points->points[i]->x = x;
            points->points[i]->y = y;
            points->coords[i]->x = x_it;
            points->coords[i]->y = y_it++;
            ++i;
        }
        ++x_it;
    }
    for (int j = i; j < points->num_points; ++j) {
        points->points[j]->x = mid_x;
        points->points[j]->y = mid_y;
    }

    PPX = ppx;
    PPY = ppy;
}

double sqr(TASK2_POINT const* p) {
    return p->x * p->x + p->y * p->y;
}

void* mandelbrot_set(void* raw_data) {
    DATA const data = *(DATA*)raw_data;
    TASK2_POINTS const* points = data.points;
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

void* progress(void* i) {
    while (1) {
        pthread_mutex_lock(&mutex_bar);
        int const step = CURR_STEP;
        pthread_mutex_unlock(&mutex_bar);

        if (step >= MAX_STEP) {
            printf("\r100%%\n");
            break;
        }

        printf("\r%.0lf%%", (double)step / MAX_STEP * 100);
        usleep(200000);
    }

    return NULL;
}

int int_to_str(long long int number, char* tmp) {
    int i = 0;

    do {
        tmp[i++] = '0' + (char)(number % 10);
        number /= 10;
    }
    while (number != 0);

    for (int j = 0; j < i / 2; j++) {
        char const temp = tmp[j];
        tmp[j] = tmp[i - j - 1];
        tmp[i - j - 1] = temp;
    }

    return i;
}

int double_to_str(double const number, char* tmp) {
    double exp = floor(log10(fabs(number)));
    if (isinf(exp)) {
        exp = 0;
    }

    double normalized_number = fabs(number) * pow(10, -exp);
    int last_index = 0;

    if (number < 0) {
        tmp[last_index++] = '-';
    }

    for (int i = 0; i < 10; ++i) {
        if (i != 1) {
            int const int_part = (int)normalized_number;

            normalized_number -= int_part;
            normalized_number *= 10;

            tmp[last_index] = '0' + (char)int_part;
        }
        else {
            tmp[last_index] = '.';
        }

        ++last_index;
    }

    tmp[last_index++] = 'e';
    if (exp >= 0) {
        tmp[last_index++] = '+';
    }
    else {
        tmp[last_index++] = '-';
    }

    int const int_exp = (int)fabs(exp);

    last_index += int_to_str(int_exp, tmp + last_index);
    tmp[last_index] = '\0';

    return last_index;
}

void* write_file(void* raw_data) {
    DATA const data = *(DATA*)raw_data;
    TASK2_POINTS const* points = data.points;
    int const threads = data.threads, rank = data.rank;
    int const num_points = points->num_points,
              start = num_points / threads * rank,
              end = rank == threads - 1 ? num_points : num_points / threads * (rank + 1);


    char *buffer = calloc(buff_size, sizeof(char)), tmp[100] = {0};
    int curr_pos = 0;
    for (int i = start; i < end; ++i) {
        int str_len = 0;
        if (write_mode == 0) {
            if (points->points[i]->color == 0) {
                str_len = double_to_str(points->points[i]->x, tmp);
                tmp[str_len++] = ';';
                str_len += double_to_str(points->points[i]->y, tmp + str_len);
                tmp[str_len++] = '\n';
            }
        }
        else {
            str_len = int_to_str((int)round(points->coords[i]->x), tmp);
            tmp[str_len++] = ';';
            str_len += int_to_str((int)round(points->coords[i]->y), tmp + str_len);
            tmp[str_len++] = ';';
            str_len += double_to_str((double)points->points[i]->color / TASK2_MAX_ITER, tmp + str_len);
            tmp[str_len++] = '\n';
        }

        if (curr_pos + str_len >= buff_size) {
            pthread_mutex_lock(&mutex_file);
            fwrite(buffer, sizeof(char), curr_pos, file);
            pthread_mutex_unlock(&mutex_file);

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
        pthread_mutex_lock(&mutex_file);
        fwrite(buffer, sizeof(char), curr_pos, file);
        pthread_mutex_unlock(&mutex_file);
    }

    free(buffer);
}

void TASK2_run(int const num_points, int const num_threads) {
    TASK2_POINTS* points = create_points(num_points);
    get_points(points);

    pthread_t* threads = calloc(num_threads + 1, sizeof(pthread_t));
    DATA* data = calloc(num_threads, sizeof(DATA));

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
        pthread_detach(threads[i]);
    }
    pthread_join(threads[num_threads], NULL);
    printf("Mandelbrot Completed\n");


    if (TASK2_SAVE_POINTS != 0) {
        file = fopen("task2_points.csv", "wb");
        setvbuf(file, NULL, _IOFBF, buff_size);

        printf("Points file progress:\n");
        CURR_STEP = 0;
        for (int i = 0; i < num_threads; ++i) {
            data[i].points = points;
            data[i].rank = i;
            data[i].threads = num_threads;

            pthread_create(&threads[i], NULL, write_file, &data[i]);
        }
        pthread_create(&threads[num_threads], NULL, progress, NULL);

        for (int i = 0; i < num_threads; ++i) {
            pthread_detach(threads[i]);
        }
        pthread_join(threads[num_threads], NULL);

        printf("Points file completed\n");
    }

    if (TASK2_SAVE_COORDS != 0) {
        write_mode = 1;
        file = fopen("task2_coords.csv", "wb");
        setvbuf(file, NULL, _IOFBF, buff_size);

        fprintf(file, "%d;%d;%d\n", num_points, PPX, PPY);

        printf("Coords file progress:\n");
        CURR_STEP = 0;
        for (int i = 0; i < num_threads; ++i) {
            data[i].points = points;
            data[i].rank = i;
            data[i].threads = num_threads;

            pthread_create(&threads[i], NULL, write_file, &data[i]);
        }
        pthread_create(&threads[num_threads], NULL, progress, NULL);

        for (int i = 0; i < num_threads; ++i) {
            pthread_detach(threads[i]);
        }
        pthread_join(threads[num_threads], NULL);

        printf("Coords file completed\n");
    }

    pthread_mutex_destroy(&mutex_file);
    pthread_mutex_destroy(&mutex_bar);
    fclose(file);
    free(data);
    free(threads);
    delete_points(points);
}
