{pkgs ? import <nixpkgs> {}}:

let
  llvmMingw = pkgs.stdenv.mkDerivation rec {
    name = "llvm-mingw";
    version = "20230517";

    src = pkgs.fetchurl {
      url = "https://github.com/mstorsjo/llvm-mingw/releases/download/${version}/llvm-mingw-${version}-ucrt-ubuntu-20.04-x86_64.tar.xz";
      hash = "sha256-AykvzWa5/blLOQ9IWZ/y65wCTQU1c7qQiDR6nE4WHs0=";
    };

    dontStrip = true;
    dontPatchELF = true;

    nativeBuildInputs = with pkgs; [
    ];

    buildInputs = with pkgs; [
      stdenv.cc.cc.lib
      zlib
    ];

    installPhase = ''
      runHook preInstall
      mkdir -p $out
      cp -r ./* $out
      runHook postInstall
    '';
  };
in
  pkgs.mkShell /*.override {stdenv = pkgs.llvmPackages_latest.stdenv;}*/ {
    NIX_LD = pkgs.lib.fileContents "${pkgs.stdenv.cc}/nix-support/dynamic-linker";
    LLVM_MINGW_SYSROOT = llvmMingw; /* Use as -DMINGW_SYSROOT_PATH=$LLVM_MINGW_SYSROOT */
    NIX_HARDENING_ENABLE = "0"; /* Disable hardening */
    nativeBuildInputs = with pkgs; [
      autoconf
      automake
      autogen
      flex
      bison
      libtool
      gtk-doc
      gettext
      pkg-config
      gperf
      glslang
      libtasn1
      python3Packages.python
      python3Packages.mako
      python3Packages.ply
      python3Packages.setuptools
      libev
      freetype
      xorg.libX11
      vulkan-headers
      jq
      perl
    ];
    depsBuildBuild = with pkgs; [
      ninja
      cmake
      meson
    ];
  }


