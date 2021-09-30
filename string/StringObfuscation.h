#ifndef _STRING_OBFUSCATION_INCLUDES_
#define _STRING_OBFUSCATION_INCLUDES_

// LLVM include
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h" // For DemoteRegToStack and DemotePHIToStack

struct GlobalStringVariable {
  llvm::GlobalVariable *var;
  size_t size;
  unsigned int index;
  bool isStruct;
  uint8_t key;

  GlobalStringVariable(llvm::GlobalVariable *var, size_t size,
                       unsigned int index, bool isStruct, uint8_t key) {
    this->var = var;
    this->size = size;
    this->index = index;
    this->isStruct = isStruct;
    this->key = key;
  }
};

namespace llvm {
struct StringObfuscatorPass : public PassInfoMixin<StringObfuscatorPass> {
  std::vector<GlobalStringVariable> globalStrings;

  StringObfuscatorPass();
  ConstantDataArray *encodeStringDataArray(LLVMContext &ctx, const char *str,
                                           size_t size, uint8_t key);
  void encodeStructString(LLVMContext &ctx, GlobalVariable *gv,
                          ConstantStruct *cs, ConstantDataArray *array,
                          unsigned int index);
  void encodeGlobalString(LLVMContext &ctx, GlobalVariable *gv,
                          ConstantDataArray *array);
  bool encodeAllStrings(Module &M);
  std::string generateRandomName();
  Function *addDecodeFunction(Module &M);
  void addDecodeAllStringsFunction(Module &M, Function *decodeFunction);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

} // namespace llvm

#endif
