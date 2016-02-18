#!/bin/bash
#NDK_HOME=$HOME/Desktop/Android/android-ndk-r10c
echo $NDK_HOME
SYSROOT=${NDK_HOME}/platforms/android-9/arch-arm/
TOOLCHAIN=${NDK_HOME}/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
CC=${TOOLCHAIN}/bin/arm-linux-androideabi-gcc
echo ${SYSROOT}
echo ${TOOLCHAIN}
echo ${CC}
function build_one
{
./configure --enable-cross-compile --cross-prefix=arm-linux-androideabi- --target-os=linux \
--arch=armv7 --cpu=armv7-a \
--disable-doc --disable-ffplay --disable-ffprobe --disable-ffserver --disable-symver --enable-debug --disable-stripping --disable-everything \
--enable-static \
--enable-protocol=pipe \
--enable-filter=scale --enable-filter=crop --enable-filter=transpose \
--enable-demuxer=rawvideo --enable-decoder=rawvideo \
--enable-muxer=image2 --enable-muxer=image2pipe --enable-muxer=mjpeg --enable-encoder=mjpeg \
--target-os=linux \
--extra-cflags="-g -DANDROID" \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--sysroot=$SYSROOT \
--extra-ldflags="-g"

make clean
make
make install
}
CPU=arm
PREFIX=$(pwd)/android/$CPU
build_one
