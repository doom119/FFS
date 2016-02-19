echo "-----------开始x264------------"

make distclean

NDK_PATH=$NDK_HOME
NDK_ARM_PATH=${NDK_PATH}/platforms/android-9/arch-arm
NDK_ARM_BIN_PATH=${NDK_PATH}/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin

CC=${NDK_ARM_BIN_PATH}/arm-linux-androideabi-gcc

#chmod +x configure
#chmod +x config.guess
#chmod +x config.sub
#chmod +x version.sh
#chmod +x tools/cltostr.sh

echo "开始configure"
./configure \
--disable-cli \
--enable-static \
--disable-asm \
--enable-pic \
--enable-debug \
--enable-gpl \
--host=arm-linux \
--extra-cflags="-g" \
--extra-ldflags="-L${NDK_ARM_PATH}/usr/lib -lc -lm -ldl -llog -g" \
--cross-prefix=${NDK_ARM_BIN_PATH}/arm-linux-androideabi- \
--sysroot=${NDK_ARM_PATH}

echo "开始make"
make -j4

echo "开始install"
sudo make install

echo "-----------结束x264------------"
