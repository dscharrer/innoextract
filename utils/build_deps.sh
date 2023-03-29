#!/bin/bash

source ./env
mkdir -p $DEPDIR && cd $DEPDIR


function info() {
    echo -e "\e[32m$@\e[0m"
}
function err() {
    echo -e "\e[32m$@\e[0m"
}

function die() {
    echo -e "\e[32m$@\e[0m" && exit 1
}

function test_lzma() {
    [ -f $DEPDIR/xz-5.2.10/src/liblzma/.libs/liblzma.a ]
}

function test_boost() {
    [ -d $DEPDIR/boost_1_74_0/stage/lib ] && [[ $(ls $DEPDIR/boost_1_74_0/stage/lib | grep bc | wc -l) -eq 7 ]]
}

function test_libzip() {
    [ -f $DEPDIR/libzip-1.9.2/lib/libzip.a  ]
}

function test_json() {
    [ -f $DEPDIR/json-3.11.2/CMakeLists.txt  ]
}

function test_deps_or_build() {
    test_lzma && info "libLZMA already built, skipping" || make_lzma
    test_boost && info "libboost already built, skipping" || make_boost
    test_libzip && info "libzip already built, skipping" || make_zip
    test_json && info "nlohmann::json already built, skipping" || make_json
    # TODO: add other libs
}

if ! env | grep -q EMSDK; then die "EMSDK not found in env, exiting" ; fi

# liblzma
function make_lzma() {
    cd $DEPDIR
    info "Downloading LZMA..."
    wget -nv https://tukaani.org/xz/xz-5.2.10.tar.gz || die "Building LZMA failed"

    info "Unpacking LZMA..."
    tar -xaf xz-5.2.10.tar.gz && cd xz-5.2.10 || die "Unpacking LZMA failed"

    info "Configuring LZMA, see lzma.log for details..."
    ( emconfigure ./configure --disable-xz -disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc --enable-symbol-versions=no --enable-shared=no) 2>&1 > $DEPDIR/lzma.log || die "LZMA configure failed"
    cd src/liblzma

    info "Building LZMA, see lzma.log for details..."
    ( emmake make -j${MAKE_J} ) >> $DEPDIR/lzma.log 2>&1 || die "Building LZMA failed"
    test_lzma && info "Done building LZMA" || err "libLZMA was not built successfully, check log for details."
}

function make_boost() {
    cd $DEPDIR
    info "Downloading boost, zlib, bzip2..."
    wget -nv https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.bz2 -O boost_1_74_0.tar.bz2
    wget -nv https://zlib.net/zlib-1.2.13.tar.gz -O zlib-1.2.13.tar.gz
    wget -nv https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz -O bzip2-1.0.8.tar.gz

    info "Unpacking..."
    tar -xaf zlib-1.2.13.tar.gz
    tar -xaf bzip2-1.0.8.tar.gz
    tar -xaf boost_1_74_0.tar.bz2

    info "Configuring zlib..."
    cd $DEPDIR/zlib-1.2.13/
    emconfigure ./configure

    info "Patching boost..."
    cd $DEPDIR/boost_1_74_0
    patch -p0 < $DEPDIR/../utils/boost_p0.patch

    info "Building Boost, see boost.log for details..."
    ( ./bootstrap.sh &&
      ./b2 -q link=static toolset=emscripten variant=release threading=single archiveflags="-r" --with-filesystem --with-system --with-date_time --with-iostreams --with-program_options -sZLIB_SOURCE=../zlib-1.2.13 -sBZIP2_SOURCE=../bzip2-1.0.8
    ) > $DEPDIR/boost.log 2>&1 || die "Building Boost failed"
    test_boost && info "Done building Boost" || err "Boost was not built successfully, check log for details."
}

function make_zip() {
    cd $DEPDIR
    info "Downloading libzip..."
    wget -nv https://libzip.org/download/libzip-1.9.2.tar.gz || die "Downloading libzip failed"

    info "Unpacking libzip..."
    tar -xaf libzip-1.9.2.tar.gz && cd libzip-1.9.2 || die "Unpacking libzip failed"

    info "Building libzip, see libzip.log for details..."
    ( emcmake cmake . ) > $DEPDIR/libzip.log 2>&1 || die "Building libzip failed"
    ( emmake make -j$MAKE_J ) >> $DEPDIR/libzip.log 2>&1 || die "Building libzip failed"
    cp zipconf.h lib/
    test_libzip && info "Done building libzip" || err "libzip lib was not built successfully, check log for details."
}

function make_json() {
    cd $DEPDIR
    info "Downloading nlohmann::json..."
    wget -nv https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz || die "Downloading nlohmann::json failed"

    info "Unpacking nlohmann::json..."
    tar -xaf v3.11.2.tar.gz && cd libzip-1.9.2 || die "Unpacking nlohmann::json failed"
}

test_deps_or_build
