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
--enable-encoder=libx264 \
--enable-encoder=libfaac \
--enable-libx264 \
--enable-libfaac \
--disable-decoders \
--enable-decoder=h264 \
--enable-decoder=aac \
--enable-decoder=mpeg4 \
--enable-decoder=mpegvideo \
--disable-parsers \
--enable-parser=h264 \
--enable-parser=ac3 \
--enable-parser=mpeg4video \
--enable-parser=mpegvideo \
--enable-parser=mpegvideo \
--enable-parser=ac3 \
--enable-parser=h261 \
--enable-parser=vc1 \
--disable-muxers \
--enable-muxer=h264 \
--enable-muxer=aac \
--enable-muxer=mpeg2video \
--enable-muxer=mov \
--disable-demuxers \
--enable-demuxer=h264 \
--enable-demuxer=aac \
--enable-demuxer=mpegvideo \
--enable-demuxer=mov \
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
--extra-cflags="-g -DANDROID -I/usr/local/include" \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--sysroot=$SYSROOT \
--extra-ldflags="-g -L/usr/local/lib"

make clean
make
make install
}
CPU=arm
PREFIX=$(pwd)/android/$CPU
build_one
