cmake_minimum_required(VERSION 3.10)
project(DNSProxyServer C)

set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -g)

# Source files
set(SOURCES
        src/main.c
        src/core/dns.c
        src/config/config.c
        src/blacklist/blacklist.c
        src/log/log.c
)

include_directories(src/core src/config src/blacklist src/log)

add_executable(DNSProxyServer ${SOURCES})

find_library(YAML_LIBRARY NAMES yaml)
if(NOT YAML_LIBRARY)
    message(FATAL_ERROR "libyaml not found")
endif()

target_link_libraries(DNSProxyServer ${YAML_LIBRARY})