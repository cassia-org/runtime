cmake_minimum_required(VERSION 3.14)

project(aresolv LANGUAGES C)

set(GEN_INSTALL_TARGET OFF CACHE BOOL "")

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")
add_library(aresolv ${SOURCES})

target_include_directories(aresolv PUBLIC include)
target_include_directories(aresolv PRIVATE include_private)
target_compile_options(aresolv PRIVATE -Wno-int-to-pointer-cast -include netbsd-compat.h)
target_compile_definitions(aresolv PRIVATE ANDROID_CHANGES)

if (${GEN_INSTALL_TARGET})
  configure_file(aresolv.pc.in aresolv.pc @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/aresolv.pc DESTINATION lib/pkgconfig)
  install(TARGETS aresolv)
  install(DIRECTORY include/
          DESTINATION include
          FILES_MATCHING PATTERN "*.h*")
endif()
