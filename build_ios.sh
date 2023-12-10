#!/bin/sh
#项目环境配置
export CMAKE=/Applications/CMake.app/Contents/bin
export PATH=$CMAKE:$PATH
touch ~/.bash_profile
source ~/.bash_profile

#构建脚本目录位置
cd "$(dirname "$0")" || exit
buildDir="$(pwd)"/build
cmakeToolchainFile=${buildDir}/../ios.toolchain.cmake
cmakeListsPath=${buildDir}/../

rm -f -r "$buildDir/ios" && mkdir -p "$buildDir/ios" && cd "$buildDir/ios" || exit 1

#mkdir -p "Release-os64combined" && cd "Release-os64combined" || exit 1
#Build for arm64 x86_64 iphoneOS + iphoneOS Simulator
#cmake ../../src/main/cpp -G Xcode  -DCMAKE_CXX_FLAGS=-std=c++14 -DCMAKE_TOOLCHAIN_FILE=../../../../ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED
#
#cmake --build . --config Release
#
#cmake --install . --config Release # Necessary to build combined library

#cd ..
mkdir -p "Release-arm64" && cd "Release-arm64" || exit 1

cmake \
  ${cmakeListsPath} -G Xcode -DCMAKE_CXX_FLAGS=-std=c++14 \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DPLATFORM=OS64
cmake --build . --config Release

cd ..
mkdir -p "Release-simulator-x86_64" && cd "Release-simulator-x86_64" || exit 1

cmake \
  ${cmakeListsPath} -G Xcode -DCMAKE_CXX_FLAGS=-std=c++14 \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DPLATFORM=SIMULATOR64
cmake --build . --config Release

cd ..
mkdir -p "Release-simulator-arm64" && cd "Release-simulator-arm64" || exit 1

cmake \
  ${cmakeListsPath} -G Xcode -DCMAKE_CXX_FLAGS=-std=c++14 \
  -DCMAKE_TOOLCHAIN_FILE=${cmakeToolchainFile} \
  -DPLATFORM=SIMULATORARM64
cmake --build . --config Release

cd ..
mkdir -p "Release-simulator-combined" && cd "Release-simulator-combined" || exit 1
lipo -create \
  ../Release-simulator-arm64/Release-iphonesimulator/liblocalCTP.a \
  ../Release-simulator-x86_64/Release-iphonesimulator/liblocalCTP.a \
  -output ./liblocalCTP.a

cd ..

mkdir -p public && cp -r $buildDir/../LocalCTP/ctp_file/current/ public/

mkdir -p "Release-xcframework" && cd "Release-xcframework" || exit 1
xcodebuild -create-xcframework \
  -library ../Release-simulator-combined/liblocalCTP.a -headers ../public \
  -library ../Release-arm64/Release-iphoneos/liblocalCTP.a -headers ../public \
  -output ./localCTP.xcframework
