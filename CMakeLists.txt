cmake_minimum_required(VERSION 3.30)
project(mpi_frecuencias)

set(CMAKE_CXX_STANDARD 17)

find_package(MPI)
if (MPI_FOUND)
    message(STATUS "MPI Found")
    message(STATUS "MPI Include path: ${MPI_CXX_VERSION}")
    message(STATUS "MPI Include path: ${MPI_INCLUDE_PATH}")
    message(STATUS "MPI Libraries: ${MPI_LIBRARIES}")
    message(STATUS "MPI Include path: ${MPI_CXX_COMPILER}")
    message(STATUS "MPI Include path: ${MPI_lINK_FLAGS}")
else ()
    message(FATAL_ERROR "MPI Not FOUND")
endif ()
include_directories(${MPI_INCLUDE_PATH})

add_executable(mpi_frecuencias main.cpp)

target_link_libraries(mpi_frecuencias ${MPI_LIBRARIES})