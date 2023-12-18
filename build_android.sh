#!/bin/sh
cmake=/Applications/CMake.app/Contents/bin/cmake

#构建脚本目录位置
cd "$(dirname "$0")" || exit
buildDir="$(pwd)"/build

if [ ! $ANDROID_NDK_HOME ]; then
   echo "Error: Must set ANDROID_NDK_HOME in this script with your actual NDK path!"
   exit -1;
fi
cmakeToolchainFile=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
cmakeListsPath=${buildDir}/../

# Android 5.0 above
MIN_SDK_VERSION=21
# cross compile on MacOS
HOST_TAG=darwin-x86_64
TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_TAG

rm -rf ${buildDir}/android && mkdir -p ${buildDir}/android

cd ${buildDir}
# build for Android arm64-v8a
ABI=arm64-v8a
TARGET_HOST=aarch64-linux-android

mkdir -p ${buildDir}/android/${ABI} && cd ${buildDir}/android/${ABI}

export AR=$TOOLCHAIN/bin/$TARGET_HOST-ar
export AS=$TOOLCHAIN/bin/$TARGET_HOST-as
export CC=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang
export CXX=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang++
export LD=$TOOLCHAIN/bin/$TARGET_HOST-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET_HOST-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET_HOST-strip

$cmake \
  ${cmakeListsPath} \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=ON \
  -DANDROID_ABI=$ABI \
  -DANDROID_STL=c++_shared \
  -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION
make -j 10

cd ${buildDir}
# build for Android armeabi-v7a
ABI=armeabi-v7a
TARGET_HOST=arm-linux-android

mkdir -p ${buildDir}/android/${ABI} && cd ${buildDir}/android/${ABI}

export AR=$TOOLCHAIN/bin/$TARGET_HOST-ar
export AS=$TOOLCHAIN/bin/$TARGET_HOST-as
export CC=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang
export CXX=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang++
export LD=$TOOLCHAIN/bin/$TARGET_HOST-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET_HOST-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET_HOST-strip

$cmake \
  ${cmakeListsPath} \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=ON \
  -DANDROID_ABI=$ABI \
  -DANDROID_STL=c++_shared \
  -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION
make -j 10

cd ${buildDir}
# build for Android x86_64
ABI=x86_64
TARGET_HOST=x86_64-linux-android

mkdir -p ${buildDir}/android/${ABI} && cd ${buildDir}/android/${ABI}

export AR=$TOOLCHAIN/bin/$TARGET_HOST-ar
export AS=$TOOLCHAIN/bin/$TARGET_HOST-as
export CC=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang
export CXX=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang++
export LD=$TOOLCHAIN/bin/$TARGET_HOST-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET_HOST-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET_HOST-strip

$cmake \
  ${cmakeListsPath} \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=ON \
  -DANDROID_ABI=$ABI \
  -DANDROID_STL=c++_shared \
  -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION
make -j 10

cd ${buildDir}
# build for Android x86
ABI=x86
TARGET_HOST=i686-linux-android

mkdir -p ${buildDir}/android/${ABI} && cd ${buildDir}/android/${ABI}

export AR=$TOOLCHAIN/bin/$TARGET_HOST-ar
export AS=$TOOLCHAIN/bin/$TARGET_HOST-as
export CC=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang
export CXX=$TOOLCHAIN/bin/$TARGET_HOST$MIN_SDK_VERSION-clang++
export LD=$TOOLCHAIN/bin/$TARGET_HOST-ld
export RANLIB=$TOOLCHAIN/bin/$TARGET_HOST-ranlib
export STRIP=$TOOLCHAIN/bin/$TARGET_HOST-strip

$cmake \
  ${cmakeListsPath} \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=ON \
  -DANDROID_ABI=$ABI \
  -DANDROID_STL=c++_shared \
  -DANDROID_NATIVE_API_LEVEL=$MIN_SDK_VERSION
make -j 10

cd ${buildDir}/android
mkdir -p public && cp -r $buildDir/../LocalCTP/ctp_file/current/ public/
