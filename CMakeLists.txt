cmake_minimum_required(VERSION 3.10.2)
project(proj LANGUAGES C CXX ASM VERSION 1.0)

include(ExternalProject)


set(OUTPUT_PREFIX_DIR ${CMAKE_BINARY_DIR}/prefix)
set(WINETOOLS_PATH "" CACHE PATH "Path to wine tools built for the host (when cross compiling)")
set(MINGW_SYSROOT_PATH "" CACHE PATH "Path to a mingw compiler sysroot built for the host")
set(DLL_BUILD_ARCHITECTURES "" CACHE STRING "List of DLL architectures to build")

string(REPLACE "-fstack-protector-strong" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
string(REPLACE "-D_FORTIFY_SOURCE=2" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_ASM_FLAGS ${CMAKE_CXX_FLAGS})

set(MINGW_COMMAND "${CMAKE_COMMAND}" -E env "PATH=$ENV{PATH}:${MINGW_SYSROOT_PATH}/bin")
if (CMAKE_CROSSCOMPILING)
  set(CROSS_COMPILER_TARGET_ARG "--target=${CMAKE_CXX_COMPILER_TARGET}")
  set(CROSS_COMPILER_SYSROOT_ARG "--sysroot=${CMAKE_SYSROOT}")

  if (WINETOOLS_PATH STREQUAL "")
    message(FATAL_ERROR "WINETOOLS_PATH is undefined!")
  endif()

  
  set(wine_EXTRA_CONFIGURE_FLAGS "--with-wine-tools=${WINETOOLS_PATH}")
endif()

set(COMPILER_SETTINGS
  # Autotools strips target from CFLAGS so need to CC instead
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


function(add_autotools_project name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "BUILD_ALWAYS" "MAKE_TARGET;INSTALL_TARGET" "PRE_CONFIGURE_COMMAND;DEPENDS;CONFIGURE_FLAGS")

  if (NOT DEFINED ARG_PRE_CONFIGURE_COMMAND)
    set(ARG_PRE_CONFIGURE_COMMAND "true")
  endif()
  if (NOT DEFINED ARG_INSTALL_TARGET)
    set(ARG_INSTALL_TARGET "install")
  endif()

  ExternalProject_Add(${name}
    INSTALL_DIR         ${OUTPUT_PREFIX_DIR}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/${name}
    DOWNLOAD_COMMAND    COMMAND cd <SOURCE_DIR> && ${ARG_PRE_CONFIGURE_COMMAND}
    CONFIGURE_COMMAND   <SOURCE_DIR>/configure ${COMPILER_SETTINGS} --prefix=<INSTALL_DIR> --host=${CMAKE_C_COMPILER_TARGET} ${ARG_CONFIGURE_FLAGS}
    BUILD_COMMAND       make -j ${ARG_MAKE_TARGET}
    INSTALL_COMMAND     make prefix=<INSTALL_DIR> ${ARG_INSTALL_TARGET}
    LOG_CONFIGURE       TRUE
    LOG_BUILD           TRUE
    LOG_INSTALL         TRUE
    BUILD_ALWAYS        ${ARG_BUILD_ALWAYS}
    DEPENDS             ${ARG_DEPENDS}
  )
endfunction()

function(add_dll_meson_project_intl name arch cross_file prefix build_always configure_flags)
  if (${configure_flags} STREQUAL " ")
    unset(configure_flags)
  endif()
  ExternalProject_Add(${name}_${arch}
    INSTALL_DIR         ${prefix}
    SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/${name}
    CONFIGURE_COMMAND   ${MINGW_COMMAND} meson setup ${configure_flags} --cross-file=${cross_file} --prefix=<INSTALL_DIR> . <SOURCE_DIR>
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

  foreach(DLL_ARCH IN LISTS DLL_BUILD_ARCHITECTURES)
    set(DLL_ARCH_PREFIX "${OUTPUT_PREFIX_DIR}/dlls/${DLL_ARCH}")
    set(DLL_ARCH_CROSS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/meson_cross_files/${DLL_ARCH}.txt")
    add_dll_meson_project_intl(${name}
                               ${DLL_ARCH}
                               ${DLL_ARCH_CROSS_FILE}
                               ${DLL_ARCH_PREFIX}
                               ${ARG_BUILD_ALWAYS} 
                               ${ARG_CONFIGURE_FLAGS})
  endforeach()
endfunction()
      

add_autotools_project(freetype PRE_CONFIGURE_COMMAND test -e ./configure || ./autogen.sh)
add_autotools_project(libtasn1 PRE_CONFIGURE_COMMAND test -e ./configure || ./autogen.sh
                      CONFIGURE_FLAGS --disable-doc)
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
add_autotools_project(wine
                      BUILD_ALWAYS
                      PRE_CONFIGURE_COMMAND autoreconf -i
                      CONFIGURE_FLAGS --enable-win64 --enable-archs=aarch64,i386  --enable-wineandroid-drv ${wine_EXTRA_CONFIGURE_FLAGS}
                      DEPENDS gnutls freetype)
add_dll_meson_project(mesa
                      CONFIGURE_FLAGS -Dgallium-drivers=zink
                      BUILD_ALWAYS)
add_dll_meson_project(dxvk
                      BUILD_ALWAYS)
add_library(proj SHARED test.c)


