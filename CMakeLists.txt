cmake_minimum_required(VERSION 3.31)
project(DNSProxyServer C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra)

set(SOURCES
        main.c
        config.c
        dns.c
        blacklist.c
        log.c
)

add_executable(DNSProxyServer ${SOURCES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML REQUIRED yaml-0.1)
include_directories(${YAML_INCLUDE_DIRS})
target_link_libraries(DNSProxyServer ${YAML_LIBRARIES})
