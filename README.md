# About

LLVM obfuscation is a LLVM pass plugin, used to apply transformation on [LLVM assembly language](https://llvm.org/docs/LangRef.html) to provide obfuscation.

## Compilation

To build this llvm plugin you need an llvm already built and define the path to llvm headers with
`LLVM_INCLUDE_DIRS`.

Example if llvm is installed at `/opt/llvm` set `-D LLVM_INCLUDE_DIRS="/opt/llvm/include/"`

Then

```
git clone https://github.com/eshard/llvm-obfuscation.git
cd llvm-obfuscation

cmake -D LLVM_INCLUDE_DIRS="/opt/llvm/include/"

cmake --build .
```

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
