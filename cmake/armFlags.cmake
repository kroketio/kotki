# - tries to determine -march, -mcpu, -mfpu flags
# - reads /proc/cpuinfo
# - fails when NEON is not present
# - fails when CPU is not from ARM (i.e: Apple M1)

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    execute_process(COMMAND python3 armFlags.py OUTPUT_VARIABLE arm_compile_opts WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    set(NEON_FOUND true CACHE BOOL "NEON available on host")
    set(ARM_FOUND true CACHE BOOL "arm")
    separate_arguments(ARM_COMPILE_OPTIONS UNIX_COMMAND ${arm_compile_opts})

    message(STATUS "adding compile options: ${arm_compile_opts}")
    add_compile_options("${ARM_COMPILE_OPTIONS}")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    EXEC_PROGRAM("/usr/sbin/sysctl -n machdep.cpu.features" OUTPUT_VARIABLE CPUINFO)
    message(FATAL "Apple ARM not supported")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(FATAL "Windows ARM not supported")
ENDIF()
