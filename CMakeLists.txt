cmake_minimum_required(VERSION 3.10.2)

# Collect parameters to pass to any CMake subprojects
# (https://cmake.org/pipermail/cmake/2018-January/067002.html)
if (NOT EXTPROJ_CMAKE_ARGS)
  get_cmake_property(vars CACHE_VARIABLES)
  foreach(var ${vars})
    get_property(currentHelpString CACHE "${var}" PROPERTY HELPSTRING)
      if("${currentHelpString}" MATCHES "No help, variable specified on the command line." OR "${currentHelpString}" STREQUAL "")
          # message("${var} = [${${var}}]  --  ${currentHelpString}") # uncomment to see the variables being processed
          list(APPEND EXTPROJ_CMAKE_ARGS "-D${var}=${${var}}")
      endif()
  endforeach()
  set(EXTPROJ_CMAKE_ARGS ${EXTPROJ_CMAKE_ARGS} CACHE STRING "comment")
endif()

project(proj LANGUAGES C CXX ASM VERSION 1.0)

include(ExternalProject)

set(OUTPUT_PREFIX_DIR ${CMAKE_BINARY_DIR}/prefix)
set(WINETOOLS_PATH "" CACHE PATH "Path to wine tools built for the host (when cross compiling)")
set(MINGW_SYSROOT_PATH "" CACHE PATH "Path to a mingw compiler sysroot built for the host")
set(DLL_BUILD_ARCHITECTURES "" CACHE STRING "List of DLL architectures to build")

if (ANDROID)
  set(CASSIAEXT_BUILD_PATH "" CACHE PATH "Path to the cassiaext build folder")
  if (CASSIAEXT_BUILD_PATH STREQUAL "")
    message(FATAL_ERROR "CASSIAEXT_BUILD_PATH is undefined!")
  endif()

  set(CASSIAEXT_PREFIX "${CASSIAEXT_BUILD_PATH}/prefix")
endif()

set(ENV{CMAKE_EXPORT_COMPILE_COMMANDS} ON) # Generate compile_commands.json when possible

if (CMAKE_CXX_FLAGS)
    # These flags break compilation of wine and libtasn1 and are set implictly by the android toolchain file
    string(REPLACE "-fstack-protector-strong" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "-D_FORTIFY_SOURCE=2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "-Werror=format-security" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_ASM_FLAGS ${CMAKE_CXX_FLAGS})
endif()


if (CMAKE_CROSSCOMPILING)
  set(CROSS_COMPILER_TARGET_ARG "--target=${CMAKE_CXX_COMPILER_TARGET}")
  set(CROSS_COMPILER_SYSROOT_ARG "--sysroot=${CMAKE_SYSROOT}")

  if (WINETOOLS_PATH STREQUAL "")
    message(FATAL_ERROR "WINETOOLS_PATH is undefined!")
  endif()

  
  set(wine_EXTRA_CONFIGURE_FLAGS "--with-wine-tools=${WINETOOLS_PATH}")
endif()

set(MINGW_DLL_BUILD_TRIPLES "")
string(REPLACE "," ";" DLL_BUILD_ARCHITECTURES_LIST ${DLL_BUILD_ARCHITECTURES})
foreach(DLL_ARCH IN LISTS DLL_BUILD_ARCHITECTURES_LIST)
  if (${DLL_ARCH} STREQUAL "aarch64")
    list(APPEND MINGW_DLL_BUILD_TRIPLES "aarch64-w64-mingw32")
  elseif(${DLL_ARCH} STREQUAL "x86_64")
    list(APPEND MINGW_DLL_BUILD_TRIPLES "x86_64-w64-mingw32")
  elseif(${DLL_ARCH} STREQUAL "i386")
    list(APPEND MINGW_DLL_BUILD_TRIPLES "i686-w64-mingw32")
  else()
    message(FATAL_ERROR "Unsupported DLL build architecture")
  endif()
endforeach()

set(COMPILER_SETTINGS
  # Autotools strips target from CFLAGS so need to use CC instead
  "CXX=${CMAKE_CXX_COMPILER_LAUNCHER} ${CMAKE_CXX_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "AS=${CMAKE_ASM_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "CPP=${CMAKE_C_COMPILER_LAUNCHER} ${CMAKE_C_COMPILER} ${CROSS_COMPILER_TARGET_ARG} -E"
  "CC=${CMAKE_C_COMPILER_LAUNCHER} ${CMAKE_C_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "AR=${CMAKE_AR}"
  "RANLIB=${CMAKE_RANLIB}"
  "NM=${CMAKE_NM}"
  "STRIP=${CMAKE_STRIP}"

  "CFLAGS=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "ASMFLAGS=${CMAKE_ASM_FLAGS} ${CMAKE_ASM_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "CPPFLAGS=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE_UC}}"
  "CXXFLAGS=${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "LDFLAGS=${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "PKG_CONFIG_LIBDIR=${OUTPUT_PREFIX_DIR}/lib/pkgconfig"
  "PKG_CONFIG_PATH=\"\""
  "PKG_CONFIG=pkg-config"
  "x86_64_CC=${MINGW_SYSROOT_PATH}/bin/clang"
)

if (ANDROID)
  set(COMPILER_SETTINGS_CASSIAEXT
  "CXX=${CMAKE_CXX_COMPILER_LAUNCHER} ${CMAKE_CXX_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "AS=${CMAKE_ASM_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "CPP=${CMAKE_C_COMPILER_LAUNCHER} ${CMAKE_C_COMPILER} ${CROSS_COMPILER_TARGET_ARG} -E"
  "CC=${CMAKE_C_COMPILER_LAUNCHER} ${CMAKE_C_COMPILER} ${CROSS_COMPILER_TARGET_ARG}"
  "AR=${CMAKE_AR}"
  "RANLIB=${CMAKE_RANLIB}"
  "NM=${CMAKE_NM}"
  "STRIP=${CMAKE_STRIP}"

  "CFLAGS=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG} -idirafter ${CASSIAEXT_PREFIX}/include"
  "ASMFLAGS=${CMAKE_ASM_FLAGS} ${CMAKE_ASM_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "CPPFLAGS=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${BUILD_TYPE_UC}}"
  "CXXFLAGS=${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG}"
  "LDFLAGS=${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_${BUILD_TYPE_UC}} ${CROSS_COMPILER_SYSROOT_ARG} -L${CASSIAEXT_PREFIX}/lib"
  "PKG_CONFIG_LIBDIR=${OUTPUT_PREFIX_DIR}/lib/pkgconfig:${CASSIAEXT_PREFIX}/lib/pkgconfig"
  "PKG_CONFIG_PATH=\"\""
  "PKG_CONFIG=pkg-config"
  "x86_64_CC=${MINGW_SYSROOT_PATH}/bin/clang"
  )
endif()

set(ACLOCAL_ENV "ACLOCAL_PATH=${OUTPUT_PREFIX_DIR}/share/aclocal:$ENV{ACLOCAL_PATH}")

# Executes the appended program with MinGW compilers added to path
set(MINGW_COMMAND "${CMAKE_COMMAND}" -E env "PATH=${MINGW_SYSROOT_PATH}/bin:$ENV{PATH}")

function(add_autotools_project name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "BUILD_ALWAYS;LINK_CASSIAEXT" "MAKE_TARGET;INSTALL_TARGET" "PRE_CONFIGURE_COMMAND;DEPENDS;CONFIGURE_FLAGS")

  if (NOT DEFINED ARG_PRE_CONFIGURE_COMMAND)
    set(ARG_PRE_CONFIGURE_COMMAND "true")
  endif()
  if (NOT DEFINED ARG_INSTALL_TARGET)
    set(ARG_INSTALL_TARGET "install")
  endif()

  if (ARG_LINK_CASSIAEXT AND ANDROID)
    set(PROJ_COMPILER_SETTINGS ${COMPILER_SETTINGS_CASSIAEXT})
  else()
    set(PROJ_COMPILER_SETTINGS ${COMPILER_SETTINGS})
  endif()

  ExternalProject_Add(${name}
    INSTALL_DIR         ${OUTPUT_PREFIX_DIR}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/${name}
    DOWNLOAD_COMMAND    COMMAND cd <SOURCE_DIR> && ${ARG_PRE_CONFIGURE_COMMAND}
    CONFIGURE_COMMAND   ${MINGW_COMMAND} <SOURCE_DIR>/configure ${PROJ_COMPILER_SETTINGS} --prefix=<INSTALL_DIR> --host=${CMAKE_C_COMPILER_TARGET} ${ARG_CONFIGURE_FLAGS}
    BUILD_COMMAND       ${MINGW_COMMAND} make -j ${ARG_MAKE_TARGET}
    INSTALL_COMMAND     ${MINGW_COMMAND} make prefix=<INSTALL_DIR> -j ${ARG_INSTALL_TARGET}
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        ${ARG_BUILD_ALWAYS}
    DEPENDS             ${ARG_DEPENDS}
  )
endfunction()

function(add_dll_meson_project_intl name triple cross_file prefix build_always configure_flags)
  if (${configure_flags} STREQUAL " ")
    unset(configure_flags)
  endif()
  ExternalProject_Add(${name}_${triple}
    INSTALL_DIR         ${prefix}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/${name}
    CONFIGURE_COMMAND   ${MINGW_COMMAND} meson setup --strip --buildtype release ${configure_flags} --cross-file=${cross_file} --prefix=<INSTALL_DIR> . <SOURCE_DIR>
    BUILD_COMMAND       ${MINGW_COMMAND} meson compile
    INSTALL_COMMAND     ${MINGW_COMMAND} meson install --no-rebuild
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        ${build_always}
  )

endfunction()

function(add_dll_meson_project name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "BUILD_ALWAYS" "" "CONFIGURE_FLAGS")
  if (NOT DEFINED ARG_BUILD_ALWAYS)
    set(ARG_BUILD_ALWAYS FALSE)
  endif()

  if (NOT DEFINED ARG_CONFIGURE_FLAGS)
    set(ARG_CONFIGURE_FLAGS " ")
  endif()

  foreach(DLL_TRIPLE IN LISTS MINGW_DLL_BUILD_TRIPLES)
    set(DLL_TRIPLE_PREFIX "${OUTPUT_PREFIX_DIR}/dlls/${DLL_TRIPLE}")
    set(DLL_TRIPLE_CROSS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/meson_cross_files/${DLL_TRIPLE}.txt")
    add_dll_meson_project_intl(${name}
                               ${DLL_TRIPLE}
                               ${DLL_TRIPLE_CROSS_FILE}
                               ${DLL_TRIPLE_PREFIX}
                               ${ARG_BUILD_ALWAYS}
                               ${ARG_CONFIGURE_FLAGS})
  endforeach()
endfunction()

function(add_copy_command source destination)
  add_custom_command(OUTPUT ${destination}
                     COMMAND ${CMAKE_COMMAND} -E copy ${source} ${destination}
                     DEPENDS ${source})
endfunction()

function(apply_git_patches target base_hash)
  set(PATCHES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches/${target})
  file(GLOB PATCHES ${PATCHES_DIR}/*.patch)

  if (NOT PATCHES)
    message(FATAL_ERROR "No patches found for the repository ${target}")
  endif()

  # Calculate the patch string
  set(PATCH_STRING "")
  foreach(PATCH IN LISTS PATCHES)
    execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} hash-object -w ${PATCH}
                    OUTPUT_VARIABLE PATCH_HASH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(PATCH_STRING "${PATCH_STRING}${PATCH_HASH}\n")
  endforeach()

  # Get the latest commit message
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} log -1 --pretty=%B
                  OUTPUT_VARIABLE COMMIT_MESSAGE
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} log -1 --pretty=%an
                  OUTPUT_VARIABLE COMMIT_AUTHOR_NAME
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} log -1 --pretty=%ae
                  OUTPUT_VARIABLE COMMIT_AUTHOR_EMAIL
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (COMMIT_AUTHOR_NAME STREQUAL "CMake" AND COMMIT_AUTHOR_EMAIL STREQUAL "cmake@localhost")
    if (COMMIT_MESSAGE STREQUAL "CMake Patch\n${PATCH_STRING}")
      # The existing commit is the expected one
      return()
    endif()

    # The existing commit is not the expected one, reset the repository to the base hash and apply the patches again
    execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} reset --hard ${base_hash}
                    OUTPUT_VARIABLE RESET_RESULT
                    ERROR_VARIABLE RESET_RESULT
                    RESULT_VARIABLE RESET_RESULT_CODE)
    if (RESET_RESULT_CODE)
      message(FATAL_ERROR "Failed to reset the repository ${target} to the base hash ${base_hash}: ${RESET_RESULT}")
    endif()
  endif()

  # Ensure the repository is at the base hash
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} rev-parse HEAD
                  OUTPUT_VARIABLE HEAD_HASH
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (NOT HEAD_HASH STREQUAL base_hash)
    message(FATAL_ERROR "The repository ${target} is not at the expected base hash ${base_hash} but at ${HEAD_HASH}")
  endif()

  # Apply patches and commit the changes
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} apply --index ${PATCHES}
                  RESULT_VARIABLE APPLY_RESULT
                  OUTPUT_QUIET)

  if (APPLY_RESULT)
    message(FATAL_ERROR "Failed to apply the patches to the repository ${target}")
  endif()

  set(ENV{GIT_COMMITTER_NAME} "CMake")
  set(ENV{GIT_COMMITTER_EMAIL} "cmake@localhost")
  execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR}/${target} commit -m "CMake Patch\n${PATCH_STRING}" --author="CMake <cmake@localhost>"
                  RESULT_VARIABLE COMMIT_RESULT
                  OUTPUT_QUIET)

  if (COMMIT_RESULT)
    message(FATAL_ERROR "Failed to commit the patches to the repository ${target}")
  endif()
endfunction()

apply_git_patches(alsa-lib d8ce72f2561f23293ad0d98d30060c4a80a74c36)

add_autotools_project(freetype PRE_CONFIGURE_COMMAND test -e ./configure || ./autogen.sh)
add_autotools_project(libtasn1 PRE_CONFIGURE_COMMAND test -e ./configure || ./autogen.sh
                      CONFIGURE_FLAGS --disable-doc)
if (ANDROID)
  add_autotools_project(alsa-lib PRE_CONFIGURE_COMMAND test -e ./configure || ./autogen.sh)
endif()
add_autotools_project(p11-kit 
                      PRE_CONFIGURE_COMMAND test -e ./configure || NOCONFIGURE=1 ./autogen.sh
                      CONFIGURE_FLAGS --without-libffi
                      DEPENDS libtasn1)
add_autotools_project(libev PRE_CONFIGURE_COMMAND test -e ./configure || sh ./autogen.sh)
add_autotools_project(nettle 
                      PRE_CONFIGURE_COMMAND test -e ./configure || ./.bootstrap
                      CONFIGURE_FLAGS --disable-documentation --enable-mini-gmp
                      DEPENDS libev p11-kit libtasn1)
add_autotools_project(gnutls 
                      PRE_CONFIGURE_COMMAND test -e ./configure || ./bootstrap 
                      CONFIGURE_FLAGS --with-nettle-mini --with-included-unistring --disable-guile --disable-gost --enable-local-libopts --disable-tests --disable-doc
                      DEPENDS nettle libtasn1)
set(wine_DEPENDS gnutls freetype)
if (ANDROID)
  list(APPEND wine_DEPENDS alsa-lib)
endif()
add_autotools_project(wine
                      BUILD_ALWAYS
                      LINK_CASSIAEXT
                      PRE_CONFIGURE_COMMAND autoreconf -i
                      CONFIGURE_FLAGS --enable-archs=${DLL_BUILD_ARCHITECTURES} ${wine_EXTRA_CONFIGURE_FLAGS}
                      DEPENDS ${wine_DEPENDS})
add_dll_meson_project(mesa
                      CONFIGURE_FLAGS -Dgallium-drivers=zink
                      BUILD_ALWAYS)
add_dll_meson_project(dxvk
                      BUILD_ALWAYS)

if (ANDROID)
  ExternalProject_Add(libadrenotools
    INSTALL_DIR         ${OUTPUT_PREFIX_DIR}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/libadrenotools
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        TRUE
    CMAKE_ARGS          -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_SHARED_LIBS=ON -DGEN_INSTALL_TARGET=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${EXTPROJ_CMAKE_ARGS}
  )

  ExternalProject_Add(alsa-oboe
    INSTALL_DIR         ${OUTPUT_PREFIX_DIR}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/alsa-oboe
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        TRUE
    DEPENDS             alsa-lib
    CMAKE_ARGS          -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} -DCMAKE_PREFIX_PATH=<INSTALL_DIR> -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DGEN_INSTALL_TARGET=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${EXTPROJ_CMAKE_ARGS}
  )

  add_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/metadata.json ${OUTPUT_PREFIX_DIR}/metadata.json)
  add_copy_command(${CMAKE_CURRENT_SOURCE_DIR}/runtime.reg ${OUTPUT_PREFIX_DIR}/runtime.reg)
endif()

if ("aarch64-w64-mingw32" IN_LIST MINGW_DLL_BUILD_TRIPLES)
  set(DLL_TRIPLE "aarch64-w64-mingw32")
  set(DLL_TRIPLE_PREFIX "${OUTPUT_PREFIX_DIR}/dlls/${DLL_TRIPLE}")

  ExternalProject_Add(FEX_${DLL_TRIPLE}
    INSTALL_DIR         ${DLL_TRIPLE_PREFIX}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/FEX
    CMAKE_ARGS          -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/FEX/toolchain_mingw.cmake -DENABLE_JEMALLOC=0 -DENABLE_JEMALLOC_GLIBC_ALLOC=0 -DMINGW_TRIPLE=${DLL_TRIPLE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_TESTS=False -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    CMAKE_COMMAND       ${MINGW_COMMAND} cmake
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        TRUE
  )
endif()

# Merge the compilation databases into a single file
add_custom_target(compile_commands
  ALL
  COMMAND find ${CMAKE_BINARY_DIR} -iname "compile_commands.json" | xargs jq -s 'map(.[])' > compile_commands.json
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Merging compilation databases"
  BYPRODUCTS compile_commands.json
)
