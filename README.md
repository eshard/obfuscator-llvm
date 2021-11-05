# About

LLVM obfuscation is a LLVM pass plugin, used to apply transformation on [LLVM assembly language](https://llvm.org/docs/LangRef.html) to provide obfuscation.

## Compilation

To build this plugin you need an llvm already built and installed version of llvm.
Please refer to [llvm documentation](https://llvm.org/docs/CMake.html) to build and install llvm to the prefix of your choice.

For example if you installed llvm at `/opt/llvm` with `-D CMAKE_INSTALL_PREFIX=/opt/llvm`.

Then, to compile the plugin

```
git clone https://github.com/eshard/llvm-obfuscation.git
cd llvm-obfuscation

export LLVM_DIR=/opt/llvm/lib/cmake
cmake -S . -B /tmp/llvm-obfuscation
cmake --build /tmp/llvm-obfuscation
```

If the compilation is successful the plugin is `libLLVMObfuscator.so` and can be used with **clang** (`-fpass-plugin=`) or **opt** (`-load-pass-plugin`).

## Usage

### With clang

This plugin can be used with clang's switch `-fpass-plugin` and depending on your llvm version in addition
you may need to specify `-fno-legacy-pass-manager`.

`clang -fno-legacy-pass-manager -fpass-plugin=<path/to/llvm/obfuscation>/libLLVMObfuscator.so hello_world.c -o hello_world`

### With opt

[`opt`](https://llvm.org/docs/CommandGuide/opt.html) can be used to apply specific passes from LLRM-IR you
would pre-generate using `clang`.

```
# pre-generate llvm bytecode
clang -01 -S -emit-llvm hello_world.c -o hello_world.bc

# apply the pass
opt --relocation-model=pic -load-pass-plugin <path/to/llvm/obfuscation>/libLLVMObfuscator.so
-passes="<my-pass-name>" hello_world.bc -o hello_world_obfuscated.bc

# generate an object file with llc
llc --relocation-model=pic -filetype=obj hello_world_obfuscated.bc -o hello_world_obfuscated.o

# generate the binary file with clang
clang hello_world_obfuscated.o -o hello_world_obfuscated
```

# Acknowledgement

This project started from [`Obfuscator-LLVM`](https://github.com/obfuscator-llvm/obfuscator) by the information security group of the University of Applied Sciences and Arts Western Switzerland of Yverdon-les-Bains
([HEIG-VD](https://heig-vd.ch/international)).
