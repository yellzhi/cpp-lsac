
cmake_minimum_required(VERSION 3.9.3)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/evmc/.git OR NOT EXISTS ${CMAKE_SOURCE_DIR}/cmake/cable/.git)
    message(FATAL_ERROR "Git submodules not initialized, execute:\n  git submodule update --init")
endif()


#add_compile_options(-Werror -Wunused-variable -Wno-unused-parameter)
add_compile_options(-Wno-unused-parameter)


include(cmake/cable/bootstrap.cmake)
include(CableBuildInfo)
include(CableBuildType)
include(CableToolchains)
include(GNUInstallDirs)

cable_configure_toolchain(DEFAULT cxx11-pic)

set(BRC_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${BRC_CMAKE_DIR})

# Map current configuration to configurations of imported targets.
set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Hunter configuration types")
set(HUNTER_JOBS_NUMBER 4 CACHE STRING "Hunter jobs number")
set(HUNTER_PASSWORDS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Hunter/passwords.cmake CACHE STRING "Hunter password path")

# In CI builds upload the binaries if the GITHUB_USER_PASSWORD was decrypted
# (only for branches and internal PRs).
if("$ENV{CI}" AND NOT "$ENV{GITHUB_USER_PASSWORD}" STREQUAL "")
    set(run_upload YES)
else()
    set(run_upload NO)
endif()
option(HUNTER_RUN_UPLOAD "Upload binaries to the cache server" ${run_upload})

include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.44.tar.gz"
    SHA1 "c4cfcc0cd39fdae8de08f6205b7f34cab4a7ba79"
    LOCAL
)

project(brcd)
set(PROJECT_VERSION 1.5.0-alpha.6)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug;Release;RelWithDebInfo)

cable_add_buildinfo_library(PROJECT_NAME brcd)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY On)

include(EthCcache)
include(EthOptions)
include(EthCompilerSettings)
include(EthExecutableHelper)
include(EthDependencies)
include(EthUtils)
#include(ProjectMHD)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
hunter_add_package(Boost COMPONENTS program_options filesystem system thread context fiber log)
find_package(Boost CONFIG REQUIRED program_options filesystem system thread context fiber log)

hunter_add_package(jsoncpp)
find_package(jsoncpp CONFIG REQUIRED)

hunter_add_package(Snappy)
find_package(Snappy CONFIG REQUIRED)

hunter_add_package(cryptopp)
find_package(cryptopp CONFIG REQUIRED)

hunter_add_package(libjson-rpc-cpp)
find_package(libjson-rpc-cpp CONFIG REQUIRED)

hunter_add_package(libscrypt)
find_package(libscrypt CONFIG REQUIRED)

hunter_add_package(ethash)
find_package(ethash CONFIG REQUIRED)

find_package(MHD REQUIRED)
#
#IF(NOT ${MIRCROHTTPD_ROOT} STREQUAL "")
#    message(STATUS "MIRCROHTTPD_ROOT: ${MIRCROHTTPD_ROOT}")
#    set(MIRCROHTTPD_INCLUDE ${MIRCROHTTPD_ROOT}/include)
#    set(MIRCROHTTPD_LIBRARY ${MIRCROHTTPD_ROOT}/lib)
#    ELSE()
#    message(FATAL_ERROR "please set  micorhttpd root : MIRCROHTTPD_ROOT.")
#ENDIF()

##openssl
find_package(OpenSSL REQUIRED)
IF(NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
    set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
    set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
    message(STATUS "Setting up OpenSSL root and include vars to ${OPENSSL_ROOT_DIR}, ${OPENSSL_INCLUDE_DIR}")
    IF("${OPENSSL_ROOT_DIR}" STREQUAL "")
        hunter_add_package(OpenSSL)
    ENDIF()
ENDIF()


include(ProjectSecp256k1)
include(ProjectLibFF)

find_package(Threads)

if(MINIUPNPC)
    find_package(Miniupnpc 1.8.2013 REQUIRED)
endif()

set(UTILS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/utils")

configure_project()

if(TESTS)
    # Import binaryen library for testeth.
    # TODO: remove this once testeth uses binaryen as a CLI tool.
    include(ProjectBinaryen)
endif()

add_subdirectory(evmc)

#Global include path for all libs.
include_directories("${CMAKE_SOURCE_DIR}")

option(ALETH_INTERPRETER_SHARED "Build brcd-interpreter as a shared library" OFF)
add_subdirectory(libbrcd-interpreter)

add_subdirectory(libdevcore)
add_subdirectory(libdevcrypto)
add_subdirectory(libp2p)

add_subdirectory(libbrccore)
add_subdirectory(libevm)
add_subdirectory(libbrcdchain)
add_subdirectory(libbrchashseal)
add_subdirectory(libpoaseal)
add_subdirectory(libdposseal)

add_subdirectory(libwebthree)
add_subdirectory(libweb3jsonrpc)

add_subdirectory(brcd)

add_subdirectory(wallet)

if (TOOLS)
    add_subdirectory(brcd-key)
    add_subdirectory(brcd-vm)
    add_subdirectory(rlp)
endif()

add_subdirectory(test)
#if (TESTS)
#    enable_testing()
#    add_subdirectory(test)
#endif()


install(FILES ${PROJECT_BINARY_DIR}/brcd/buildinfo.json DESTINATION ${CMAKE_INSTALL_DATADIR}/brcd)


if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
include(CPack)
