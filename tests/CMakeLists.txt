cmake_minimum_required(VERSION 3.16)
project(ring_buffer_tests C)
set(CMAKE_C_STANDARD 11)

add_executable(test_ring_buffer test_ring_buffer.c ../Core/Src/ring_buffer.c)
target_include_directories(test_ring_buffer PRIVATE ../Core/Inc)
target_compile_options(test_ring_buffer PRIVATE -Wall -Wextra -Werror -g -fsanitize=address,undefined)
target_link_options(test_ring_buffer PRIVATE -fsanitize=address,undefined)

enable_testing()
add_test(NAME ring_buffer COMMAND test_ring_buffer)