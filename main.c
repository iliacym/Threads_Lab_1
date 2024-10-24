#include "stdlib.h"
#include "stdio.h"
#include "src/Task1/task1.h"
#include "src/Task2/task2.h"
#include "src/Task3/task3.h"
#include "src/utils/timer.h"


int main() {
    system("cls");

    char option;
    unsigned long long int n_trials, n_points, batch_size;
    int n_threads, inserts_in_main, total_ops;
    double start, finsh, search_percent, insert_percent;

    printf("-----------------------------\n");
    printf("| THREAD LAB1 WELCOMES YOU! |\n");
    printf("-----------------------------\n");

    while (1) {
        printf("\nChoose task:\n"
            "[1] - Task 1 (calculation of the pi number)\n"
            "[2] - Task 2 (calculation of the Mandelbrot set)\n"
            "[3] - Task 3 (implementation of rwlock)\n"
            "[e] - Exit\n");

        scanf("%c", &option);
        getchar();

        switch (option) {
        case '1':
            while (1) {
                printf("Enter number of trials and number of threads\nTrials = ");
                scanf("%llu", &n_trials);
                getchar();

                if (n_trials <= 0) {
                    printf("Number of trials must be greater than 0\n");
                } else {
                    break;
                }
            }
            while (1) {
                printf("Threads = ");
                scanf("%d", &n_threads);
                getchar();

                if (n_threads <= 0) {
                    printf("Number of threads must be greater than 0\n");
                } else {
                    break;
                }
            }
            GET_TIME(start);
            double const task1_result = TASK1_run(n_trials, n_threads);
            GET_TIME(finsh);

            printf("Pi = %lf, Elapsed time = %e sec.\n", task1_result, finsh - start);
            break;

        case '2':
            while (1) {
                printf("Enter number of points, batch size and number of threads\nPoints = ");
                scanf("%llu", &n_points);
                getchar();

                printf("Batch size = ");
                scanf("%llu", &batch_size);
                getchar();

                if (batch_size > n_points) {
                    printf("Batch size must be less or equal to number of points\n");
                } else {
                    break;
                }
            }

            while (1) {
                printf("Threads = ");
                scanf("%d", &n_threads);
                getchar();

                if (n_threads <= 0) {
                    printf("Number of threads must be greater than 0\n");
                } else {
                    break;
                }
            }

            while (1) {
                printf("Do you want to set custom borders? [y/n]\n");
                char custom;
                scanf("%c", &custom);
                getchar();

                if (custom == 'y') {
                    long double x0, x1, y0, y1;
                    printf("Enter x0, x1, y0, y1\n");
                    scanf("%Lf %Lf %Lf %Lf", &x0, &x1, &y0, &y1);
                    getchar();

                    if (x0 < x1 && y0 < y1) {
                        TASK2_set_borders(x0, x1, y0, y1);
                        break;
                    } else {
                        printf("Left border should be less than right\n");
                    }
                } else if (custom == 'n') {
                    break;
                } else {
                    printf("Invalid option\n");
                }
            }

            while (1) {
                printf("Do you want to set custom number of iters? [y/n]\n");
                char custom;
                scanf("%c", &custom);
                getchar();

                if (custom == 'y') {
                    unsigned long long int iter;
                    printf("Enter number of iters\n");
                    scanf("%llu", &iter);
                    getchar();

                    TASK2_set_iter(iter);

                    break;
                } else if (custom == 'n') {
                    break;
                } else {
                    printf("Invalid option\n");
                }
            }

            printf("Elapsed time = %e sec.\n", TASK2_run(n_points, batch_size, n_threads));
            break;

        case '3':
            printf("Enter number of threads\n");
            while (1) {
                printf("Threads = ");
                scanf("%d", &n_threads);
                getchar();

                if (n_threads <= 0) {
                    printf("Number of threads must be greater than 0\n");
                } else {
                    break;
                }
            }

            while (1) {
                printf("Enter the initial number of points in list and total number of operations\n");
                scanf("%d %d", &inserts_in_main, &total_ops);
                getchar();
                printf("Enter search percent and insert percent, both numbers are in [0, 1]\n");
                scanf("%lf %lf", &search_percent, &insert_percent);
                getchar();

                if (inserts_in_main < 0 || total_ops < 0 || search_percent < 0 || insert_percent < 0 || search_percent >
                    1 || insert_percent > 1) {
                    printf("Incorrect data\n");
                } else {
                    break;
                }
            }
            out const TASK3_data = TASK3_run(n_threads, inserts_in_main, total_ops, search_percent, insert_percent);
            printf("Elapsed time for example = %e sec.\n", TASK3_data.time_example);
            printf("Elapsed time for our realisation = %e sec.\n", TASK3_data.time_my);
            if (TASK3_data.is_equal == 1) {
                printf("Lists are equal.\n");
            } else {
                printf("Oops, unluck.\n");
            }
            break;

        case 'e':
            return 0;

        default:
            printf("Invalid option\n");
            break;
        }
    }
}
