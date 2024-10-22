cmake_minimum_required(VERSION 3.2)
project(Thread_Lab_1)

set(CMAKE_C_STANDARD 11)

add_executable(
        Thread_Lab_1 main.c
        src/Task1/task1.c
        src/Task1/task1.h
        src/Task2/task2.c
        src/Task2/task2.h
        src/Task3/task3.c
        src/Task3/task3.h
        src/utils/pth_ll_rwl.c
        src/utils/pth_ll_rwl.h
        src/utils/my_rand.c
        src/utils/my_rand.h
        src/utils/timer.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DLINUX)
    target_link_libraries(Thread_Lab_1 m)
endif ()
