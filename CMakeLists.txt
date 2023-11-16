cmake_minimum_required(VERSION 3.10)
project(raspred2 C)

set(CMAKE_C_STANDARD 99)
link_directories(/Users/danil/CLionProjects/raspred2)
#target_link_libraries()

add_executable(pa2
        pa23.c
        pa2345.h
        ../common/common.h
        ../common/ipc.h
        ipc.c
        utility.c
        utility.h
        logger.c
        logger.h
        bank_robbery.c
        clock.c
        clock.h)
