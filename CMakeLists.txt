cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(seabolt
        DESCRIPTION "The C Connector library for Neo4j"
        LANGUAGES C CXX)

IF (DEFINED ENV{SEABOLT_VERSION})
    set(_SEABOLT_VERSION $ENV{SEABOLT_VERSION})
ELSE ()
    set(_SEABOLT_VERSION "1.7.4-dev")
ENDIF ()

set(SEABOLT_VERSION ${_SEABOLT_VERSION} CACHE STRING "The version of seabolt being built")

message(STATUS "Building seabolt: ${SEABOLT_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Internals)
discover_os()
set_version()
set_names()
configure_rpath()
fix_compiler_settings()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ON_WINDOWS OFF)
set(ON_MACOS OFF)
set(ON_POSIX OFF)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(ON_WINDOWS ON)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(ON_MACOS ON)
endif ()

set(ON_GCC OFF)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(ON_GCC ON)
endif ()

if (NOT ON_WINDOWS)
    include(CheckIncludeFile)
    check_include_file("unistd.h" HAVE_UNISTD_H)
    if (HAVE_UNISTD_H)
        set(ON_POSIX ON)
    endif ()
endif ()

if (WIN32 AND NOT CYGWIN)
    set(CMAKE_INSTALL_LIBDIR "lib")
    set(CMAKE_INSTALL_BINDIR "bin")
    set(CMAKE_INSTALL_INCLUDEDIR "include")
    set(CMAKE_INSTALL_PKGCONFIGDIR ".")
    set(CMAKE_INSTALL_CMAKEDIR "cmake")
    set(INCLUDEDIR_RELATIVE_PKGCONFIGDIR "include/${SEABOLT_NAME}")
    set(LIBDIR_RELATIVE_PKGCONFIGDIR "lib")
else ()
    include(GNUInstallDirs)
    set(CMAKE_INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
    set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${SEABOLT_NAME})
    set(INCLUDEDIR_RELATIVE_PKGCONFIGDIR "../../${CMAKE_INSTALL_INCLUDEDIR}/${SEABOLT_NAME}")
    set(LIBDIR_RELATIVE_PKGCONFIGDIR "../../${CMAKE_INSTALL_LIBDIR}")
endif ()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}/${SEABOLT_NAME} CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKEDIR ${CMAKE_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")
set(INSTALL_PKGCONFIGDIR ${CMAKE_INSTALL_PKGCONFIGDIR} CACHE PATH "Installation directory for pkg-config files")

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if (IS_BIG_ENDIAN)
    set(BIG_ENDIAN 1)
else ()
    set(BIG_ENDIAN 0)
endif ()

include(CMakeDependentOption)
option(WITH_TLS_SUPPORT "Build seabolt with TLS support" ON)
cmake_dependent_option(WITH_TLS_SECURE_CHANNEL "Use Windows Secure Channel for TLS support" ON "WITH_TLS_SUPPORT;ON_WINDOWS;NOT WITH_TLS_OPENSSL" OFF)
cmake_dependent_option(WITH_TLS_OPENSSL "Use OPENSSL for TLS support" ON "WITH_TLS_SUPPORT;NOT WITH_TLS_SECURE_CHANNEL" OFF)

if (WITH_TLS_SUPPORT)
    message(STATUS "Building seabolt with TLS support")

    if (WITH_TLS_OPENSSL)
        message(STATUS "Using OPENSSL for TLS")
    endif ()

    if (WITH_TLS_SECURE_CHANNEL)
        message(STATUS "Using Windows Secure Channel for TLS")
    endif ()

    if (WITH_TLS_SECURE_CHANNEL AND NOT ON_WINDOWS)
        message(FATAL_ERROR "Windows Secure Channel can only be enabled when building on Windows")
    endif ()
endif ()

enable_testing()
include(src/CMakeLists.txt)

# <<< Install and export targets >>>
include(InstallRequiredSystemLibraries)

install(
        TARGETS
        ${SEABOLT_SHARED}
        ${SEABOLT_STATIC}
        EXPORT
        ${SEABOLT_NAME}Targets
        ARCHIVE
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        RUNTIME
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin
        LIBRARY
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        PUBLIC_HEADER
        DESTINATION ${INSTALL_INCLUDEDIR}/bolt
        COMPONENT dev)

install(
        EXPORT ${SEABOLT_NAME}Targets
        NAMESPACE "${SEABOLT_NAME}::"
        DESTINATION ${INSTALL_CMAKEDIR}
        COMPONENT dev)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
endif ()
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)

install(
        FILES
        ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${SEABOLT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPATIBILITY SameMinorVersion)

configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/seaboltConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${SEABOLT_NAME}Config.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PKGCONFIGDIR}/${SEABOLT_NAME}.pc
        DESTINATION ${INSTALL_PKGCONFIGDIR}
        COMPONENT dev)

if (NOT ON_WINDOWS OR WITH_TLS_SECURE_CHANNEL)
    install(
            FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_PKGCONFIGDIR}/${SEABOLT_NAME}-static.pc
            DESTINATION ${INSTALL_PKGCONFIGDIR}
            COMPONENT dev)
endif ()

install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${SEABOLT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${SEABOLT_NAME}ConfigVersion.cmake
        DESTINATION ${INSTALL_CMAKEDIR}
        COMPONENT dev)

include(CMakeCPack.cmake)
