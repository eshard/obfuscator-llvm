# Android NDK

## Build Environment

Steps

 - [Download latest NDK](https://developer.android.com/ndk/downloads)
 - Recompile the same version of llvm-toolchain present in the NDK  
   [See llvm-android documentation](https://android.googlesource.com/toolchain/llvm_android/+/master/README.md)
 - Compile the plugin with the NDK host toolchain
 - Cross compile your project with the NDK target toolchain

#### Recompile the same version of llvm-toolchain of the NDK

Example for version `r32b`

```
cd ~/tmp
unzip android-ndk-r23b-linux.zip
./android-ndk-r23b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang --version

# Android (7714059, based on r416183c1) clang version 12.0.8 (https://android.googlesource.com/toolchain/llvm-project c935d99d7cf2016289302412d708641d52d2f7ee)
```

Following [llvm-android documentation](https://android.googlesource.com/toolchain/llvm_android/+/master/README.md)

```
mkdir ~/tmp/llvm-toolchain && cd ~/tmp/llvm-toolchain
repo init -u https://android.googlesource.com/platform/manifest -b llvm-toolchain
cp ~/tmp/android-ndk-r23b/toolchains/llvm/prebuilt/linux-x86_64/manifest_7714059.xml .repo/manifests
repo init -m manifest_7714059.xml
repo sync -c
```

Then rebuild the toolchain

```
python toolchain/llvm_android/build.py --no-build windows --no-build lldb --no-strip
```

`--no-strip` is important to avoid removing symbols not used by llvm/clang but needed for our plugin.

Locate the *stage2* build of llvm-android at `out/stage2-install/`

For example we consider the install path to be `~/tmp/llvm-toolchain/out/stage2-install/` later refered as `LLVM_HOME`

#### Compile the plugin with NDK's host toolchain

Compile the plugin with  `-D LLVM_DIR=~/tmp/llvm-toolchain/out/stage2-install/lib64/cmake` along with a [NDK compile environment](https://developer.android.com/ndk/guides/other_build_systems#non-autoconf_make_projects).

```
# See https://developer.android.com/ndk/guides/other_build_systems

# From Android NDK r23b
# # # ./toolchains/llvm/prebuilt/linux-x86_64/bin/clang-12 --version
# # Android (7714059, based on r416183c1) clang version 12.0.8
# # (https://android.googlesource.com/toolchain/llvm-project c935d99d7cf2016289302412d708641d52d2f7ee)
VERSION="r23b"
REVISION="r416183"

# Change this path to your li
NDK=${HOME}/tmp/android-ndk-$VERSION
LLVM_NDK=${HOME}/tmp/llvm-toolchain

# Only choose one of these, depending on your build machine
#export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64

# Tools
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/clang++
export LD=$TOOLCHAIN/bin/lld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

# LLVM
export LLVM_HOME=${LLVM_NDK}/prebuilts/clang/host/linux-x86/clang-${REVISION}/
export LLVM_DIR=${LLVM_HOME}/lib64/cmake

# Set NDK sysroot, includes and libc++ headers
export HOST_SYSROOT_NDK="$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/"
export HOST_INCLUDES="$HOST_SYSROOT_NDK/usr/include/"
export CXX_HEADERS="$HOST_SYSROOT_NDK/usr/include/c++/v1/"


export CFLAGS="-isystem $HOST_INCLUDES -I$CXX_HEADERS"
export CXXFLAGS="-isystem ${HOST_INCLUDES} -I${CXX_HEADERS}"
export LDFLAGS="-L${HOST_NDK_LIBS}"

export PATH="$TOOLCHAIN/bin:$PATH"
```

And compile the plugin with,

```
cmake -S <PATH/TO/LLVM-OBFUSCATION> -B ~/tmp/llvm-obfuscation-ndk -D LLVM_DIR="$LLVM_DIR" -D CMAKE_CXX_COMPILER=$CXX -D CMAKE_CXX_FLAGS="$CXXFLAGS"

cmake --build ~/tmp/llvm-obfuscation-ndk
```

#### Cross compile and obfuscate with the NDK for your target device

```
# Only choose one of these, depending on your build machine...
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
# Only choose one of these, depending on your device...
export TARGET=aarch64-linux-android
export TARGET=armv7a-linux-androideabi
export TARGET=i686-linux-android
export TARGET=x86_64-linux-android
# Set this to your minSdkVersion.
export API=21
# Configure and build.
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

# NDK headers and libs
export NDK_INCLUDE=$(readlink -f $TOOLCHAIN/sysroot/usr/include)
export NDK_SYSROOT=$TOOLCHAIN/sysroot/usr/lib/$TARGET/$API/
export NDK_GCC=$TOOLCHAIN/lib/gcc/$TARGET/4.9.x/

# CFLAGS/LDFLAGS
export LDFLAGS="-L${NDK_GCC}"
export CFLAGS="-I${NDK_INCLUDE} -I${NDK_INCLUDE}/$TARGET"
export CXXFLAGS="-I${NDK_INCLUDE} -I${NDK_INCLUDE}/$TARGET"
```

And to obfuscate your project just define `CFLAGS` to use the plugin

```
export CFLAGS="$CFLAGS -O1 -fno-legacy-pass-manager -fpass-plugin=~/llvm-obfuscation-ndk/libLLVMObfuscator.so"
```
