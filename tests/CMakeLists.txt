cmake_minimum_required(VERSION 3.16)
project(ring_buffer_tests C)
set(CMAKE_C_STANDARD 11)

option(RB_COVERAGE "Build with gcov coverage" OFF)

add_executable(test_ring_buffer test_ring_buffer.c ../Core/Src/ring_buffer.c)
target_include_directories(test_ring_buffer PRIVATE ../Core/Inc)
target_compile_options(test_ring_buffer PRIVATE
    -Wall -Wextra -Werror -g
    -UNDEBUG
    -fsanitize=address,undefined)
target_link_options(test_ring_buffer PRIVATE -fsanitize=address,undefined)

if(RB_COVERAGE)
    target_compile_options(test_ring_buffer PRIVATE --coverage -O0)
    target_link_options(test_ring_buffer PRIVATE --coverage)
endif()

enable_testing()

set(RB_TESTS
    test_starts_empty
    test_fifo_order
    test_fill_to_capacity
    test_overflow_keeps_data
    test_wraparound
    test_push_zero_len
    test_refill_after_wrap
    test_partial_push_mid_buffer
    test_interleaved_vs_reference
    test_reset_nonempty
    test_independent_instances
    test_random_vs_reference
)

foreach(t IN LISTS RB_TESTS)
    add_test(NAME ${t} COMMAND test_ring_buffer ${t})
endforeach()
