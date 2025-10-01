cmake_minimum_required(VERSION 3.28)

project(stargate VERSION 1.0 LANGUAGES C ASM)

set(DEBUG "" CACHE STRING "Debug level (1,2,3)")
message(STATUS "DEBUG LEVEL = ${DEBUG}")

include(FetchContent)

FetchContent_Declare(
   ark-dev-sdk
   GIT_REPOSITORY https://github.com/PSP-Arkfive/ark-dev-sdk.git
   GIT_TAG main
   EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(ark-dev-sdk)

set(ARKSDK ${ark-dev-sdk_SOURCE_DIR})

add_prx_module(stargate exports.exp)

target_sources(stargate PRIVATE 
   main.c
   imports.S
   src/loadmodule_patch.c
   src/nodrm_patch.c
   src/io_patch.c
   src/key_decrypt.c
   src/pspcipher.c
   src/gamefix.c
   src/hide.c
   src/chn_iso.c
)

remove_definitions("-D_PSP_FW_VERSION=600")
add_definitions("-D_PSP_FW_VERSION=660")

target_compile_options(stargate PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)
target_include_directories(stargate PRIVATE include ${ARKSDK}/include)
target_link_directories(stargate PRIVATE ${ARKSDK}/libs)
target_link_libraries(stargate PRIVATE
   -nostartfiles
   -nostdlib
   pspsystemctrl_kernel
   pspsysc_user
   pspsemaphore_660
   pspsdk
   pspmodinfo
   pspkernel
)

if(DEBUG)
   target_compile_definitions(stargate PRIVATE -DDEBUG=${DEBUG})
endif()
