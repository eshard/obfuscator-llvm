#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "bogus/BogusControlFlow.h"
#include "flattening/Flattening.h"
#include "split/SplitBasicBlocks.h"
#include "substitution/Substitution.h"

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "SubstitutionObfuscatorPass", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "substitution-obfuscator-pass") {
                    FPM.addPass(SubstitutionPass());
                    return true;
                  } else if (Name == "split-basic-block-obfuscator-pass") {
                    FPM.addPass(SplitBasicBlockPass());
                    return true;
                  } else if (Name == "flattening-obfuscator-pass") {
                    FPM.addPass(FlatteningObfuscatorPass());
                    return true;
                  } else if (Name == "bogus-control-flow-obfuscator-pass") {
                    FPM.addPass(BogusControlFlowPass());
                    return true;
                  } else {
                    return false;
                  }
                });
          }};
}
