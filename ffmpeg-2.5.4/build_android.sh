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
./configure \
--disable-doc \
--disable-ffmpeg \
--disable-ffprobe \
--disable-ffplay \
--disable-ffserver \
--disable-avdevice \
--disable-postproc \
--disable-network \
--disable-pthreads \
--disable-encoders \
--disable-decoders \
--disable-parsers \
--disable-muxers \
--disable-demuxers \
--disable-protocols \
--enable-protocol=file \
--disable-indevs \
--enable-indev=v4l \
--enable-indev=v4l2 \
--disable-bsfs \
--enable-nonfree \
--enable-shared \
--disable-static \
--enable-debug \
--disable-stripping \
--disable-optimizations \
--disable-yasm \
--disable-asm \
--enable-gpl \
--enable-pic \
--enable-cross-compile \
--enable-thumb \
--enable-hwaccels \
--arch=arm \
--cpu=armv7-a \
--disable-neon \
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
