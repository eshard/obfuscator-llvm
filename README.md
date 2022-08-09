# About

LLVM obfuscation is a LLVM pass plugin, used to apply transformation on [LLVM assembly language](https://llvm.org/docs/LangRef.html) to provide obfuscation.

## Compilation

To build this plugin you need to build LLVM. Refer to the [documentation](https://llvm.org/docs/CMake.html) for more information. 

Here's how you can download, build and install LLVM:
```
git clone --depth 1 --branch llvmorg-14.0.6 https://github.com/llvm/llvm-project.git
cd llvm-project

mkdir -p build && cd build
cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=/opt/llvm ..
ninja -j$(nproc)
ninja install
```

Then, to compile the plugin:

```
git clone https://github.com/eshard/obfuscator-llvm.git
cd obfuscator-llvm

mkdir -p build && cd build
cmake -G "Ninja" -DLLVM_DIR=/opt/llvm/lib/cmake ..
ninja -j$(nproc)
```

If the compilation is successful the plugin is `libLLVMObfuscator.so` and can be used with **clang** (`-fpass-plugin=`) or **opt** (`-load-pass-plugin`).

## Usage

### With clang

This plugin can be used with clang's switch `-fpass-plugin` and depending on your llvm version in addition
you may need to specify `-fno-legacy-pass-manager`.

`clang -fno-legacy-pass-manager -fpass-plugin=<path/to/llvm/obfuscation>/libLLVMObfuscator.so hello_world.c -o hello_world`

You can chose to insert passes in the optimization pipeline by setting the following environment variables before running clang:
- LLVM_OBF_PEEPHOLE_PASSES
- LLVM_OBF_SCALAROPTIMIZERLATE_PASSES
- LLVM_OBF_VECTORIZERSTART_PASSES
- LLVM_OBF_PIPELINESTART_PASSES
- LLVM_OBF_PIPELINEEARLYSIMPLIFICATION_PASSES
- LLVM_OBF_OPTIMIZERLASTEP_PASSES

For instance if you want to run the flattening, bogus and substitution passes in that order, you can do:
`export LLVM_OBF_SCALAROPTIMIZERLATE_PASSES="flattening, bogus, substitution, split-basic-blocks"`

Refer to the llvm::PassBuilder documentation for more information on each insertion point.

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

### Debugging

To allow debugging passes in a deterministic way, the environment variable `LLVM_OBF_SEED` can be set to fix the CryptoUtils seed (used to for all random number generation and
encryption).

The variable should contain a hex string of 32 characters or 34 characters if prefixed with "0x", for example:
`export LLVM_OBF_SEED="0xA04252B187478C00A40BC6D81D1A8A52"`

The environement variable `LLVM_OBF_DEBUG_SEED` can be set to "y" to enable printing the seed everytime the plugin is loaded.

## Cross compilation

 - [With Android NDK](docs/ANDROID_NDK.md)

# Acknowledgement

This project started from [`Obfuscator-LLVM`](https://github.com/obfuscator-llvm/obfuscator) by the information security group of the University of Applied Sciences and Arts Western Switzerland of Yverdon-les-Bains
([HEIG-VD](https://heig-vd.ch/international)).
