find_program(XXD xxd)

# If defined, use the target compiler.
# For cross-compilation the LLVM-IR generated is still arch specific.
if (NOT ${TARGET_C_COMPILER})
    set(CLANG ${TARGET_C_COMPILER})
else()
    set(CLANG "${LLVM_TOOLS_BINARY_DIR}/clang")
endif()

# Compile the string decode function into IR bitcode and embed it as a c array
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/decode.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generate_ir_header.sh
                ${CLANG} ${XXD}
                ${CMAKE_CURRENT_SOURCE_DIR}/decode.c
                ${CMAKE_CURRENT_BINARY_DIR} decode.h
    DEPENDS decode.c
)

add_custom_target(generateDecodeIRHeader ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/decode.h
)

add_dependencies(LLVMObfuscator generateDecodeIRHeader)
target_sources(LLVMObfuscator PRIVATE StringObfuscation.cpp)
