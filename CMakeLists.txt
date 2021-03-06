cmake_minimum_required(VERSION 3.17)
project(homework C)

set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

# Add main.c file of project root directory as source file
set(
        SOURCE_FILES main.c cpu-limited-direct-execution/homework.c 13-vm-intro/memory-user.c 14-vm-api/null.c 14-vm-api/array-of-ints.c 14-vm-api/vector.c 19-vm-tlb-paging/tlb.c 21-vm-beyondphys/mem.c 27-threads-api/main-race.c 27-threads-api/main-deadlock-global.c 27-threads-api/main-deadlock.c 27-threads-api/main-signal-cv.c 27-threads-api/main-signal.c
)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(simple_example ${SOURCE_FILES})

add_executable(cpu_lde cpu-limited-direct-execution/homework.c)
add_executable(13_memory_user 13-vm-intro/memory-user.c)
add_executable(14_null 14-vm-api/null.c)
add_executable(14_array_of_ints 14-vm-api/array-of-ints.c)
add_executable(14_vector 14-vm-api/vector.c)
add_executable(19_tlb 19-vm-tlb-paging/tlb.c)
add_executable(21_mem 21-vm-beyondphys/mem.c)
add_executable(27_main_race 27-threads-api/main-race.c)
add_executable(27_main_deadlock_global 27-threads-api/main-deadlock-global.c)
add_executable(27_main_deadlock 27-threads-api/main-deadlock.c)
add_executable(27_main_signal_cv 27-threads-api/main-signal-cv.c)
add_executable(27_main_signal 27-threads-api/main-signal.c)
add_executable(30_one-cv-while 30-threads-cv/main-one-cv-while.c)
add_executable(30_two-cvs-if 30-threads-cv/main-two-cvs-if.c)
add_executable(30_two-cvs-while-extra-unlock 30-threads-cv/main-two-cvs-while-extra-unlock.c)
add_executable(30_two-cvs-while 30-threads-cv/main-two-cvs-while.c)
add_executable(31_fork-join 31-threads-sema/fork-join.c)
add_executable(31_rendezvous 31-threads-sema/rendezvous.c)
add_executable(31_barrier 31-threads-sema/barrier.c)
add_executable(31_reader-writer 31-threads-sema/reader-writer.c)
add_executable(31_reader-writer-nostarve 31-threads-sema/reader-writer-nostarve.c)
add_executable(31_mutex-nostarve 31-threads-sema/mutex-nostarve.c)
add_executable(32_vector-deadlock 32-threads-bugs/vector-deadlock.c)
add_executable(32_vector-global-order 32-threads-bugs/vector-global-order.c)
add_executable(32_vector-try-wait 32-threads-bugs/vector-try-wait.c)
add_executable(32_vector-avoid-hold-and-wait 32-threads-bugs/vector-avoid-hold-and-wait.c)
add_executable(32_vector-nolock 32-threads-bugs/vector-nolock.c)
add_executable(39_files-stat 39-file-files-and-directories/stat.c)
add_executable(39_files-ls 39-file-files-and-directories/myls.c)
add_executable(39_files-tail 39-file-files-and-directories/mytail.c)
add_executable(39_files-find 39-file-files-and-directories/myfind.c)
add_executable(45_check-xor 45-file-integrity/check-xor.c)
add_executable(45_check-fletcher 45-file-integrity/check-fletcher.c)
add_executable(45_test-xor-fletcher 45-file-integrity/test-xor-fletcher.c)
add_executable(48_udp-client 48-dist-intro/UDP-client.c)
add_executable(48_udp-server 48-dist-intro/UDP-server.c)
