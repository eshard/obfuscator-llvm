#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/FormatVariadic.h"

#include "bogus/BogusControlFlow.h"
#include "flattening/Flattening.h"
#include "split/SplitBasicBlocks.h"
#include "substitution/Substitution.h"
#include "utils/CryptoUtils.h"

static const char PassesDelimiter = ',';
static const std::string EnvVarPrefix = "LLVM_OBF_";

namespace llvm {

StringRef getEnvVar(StringRef const &key) {
  char const *value = getenv(key.data());
  return value == NULL ? StringRef() : StringRef(value);
}

bool addPassWithName(FunctionPassManager &FPM, StringRef &passName) {
  if (passName == "substitution") {
    FPM.addPass(SubstitutionPass());
  } else if (passName == "split-basic-blocks") {
    FPM.addPass(SplitBasicBlockPass());
  } else if (passName == "flattening") {
    FPM.addPass(FlatteningObfuscatorPass());
  } else if (passName == "bogus") {
    FPM.addPass(BogusControlFlowPass());
  } else {
    return false;
  }

  return true;
}

bool addPassWithName(ModulePassManager &MPM, StringRef &passName) {
  return false;
}

template <class T>
void addPassesFromEnvVar(PassManager<T> &M, const StringRef &var) {
  auto passesStr = getEnvVar(var);

  SmallVector<StringRef> passes;
  passesStr.split(passes, PassesDelimiter, -1, false);
  for (auto passName : passes) {
    addPassWithName(M, passName);
  }
}

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  /* Fixed seed for cryptoutils */
  StringRef seed = getEnvVar(EnvVarPrefix + "SEED");
  if (!seed.empty()) {
    llvm::cryptoutils->prng_seed(seed.data());
  }

  /* Print the seed */
  if (getEnvVar(EnvVarPrefix + "DEBUG_SEED") == "y") {
    llvm::cryptoutils->get_uint8_t();
    const char *used_seed = llvm::cryptoutils->get_seed();
    outs() << "SEED = 0x";
    for (int i = 0; i < 16; i++) {
      outs() << formatv("{0:X-2}", (uint8_t)used_seed[i]);
    }
    outs() << "\n";
  }

  /* Register LLVM passes */
  return {
      LLVM_PLUGIN_API_VERSION, "Obfuscator plugin", "v0.1",
      [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              return addPassWithName(FPM, Name);
            });

        PB.registerPipelineParsingCallback(
            [](StringRef Name, ModulePassManager &MPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              return addPassWithName(MPM, Name);
            });

        // Add passes that perform peephole optimizations similar to the
        // instruction combiner. These passes will be inserted after each
        // instance of the instruction combiner pass.
        PB.registerPeepholeEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              addPassesFromEnvVar(FPM, EnvVarPrefix + "PEEPHOLE_PASSES");
            });

        // Add optimization passes after most of the main optimizations, but
        // before the last cleanup-ish optimizations.
        PB.registerScalarOptimizerLateEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              addPassesFromEnvVar(FPM,
                                  EnvVarPrefix + "SCALAROPTIMIZERLATE_PASSES");
            });

        // Add optimization passes before the vectorizer and other highly target
        // specific optimization passes are executed.
        PB.registerVectorizerStartEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              addPassesFromEnvVar(FPM, EnvVarPrefix + "VECTORIZERSTART_PASSES");
            });

        // Add optimization once at the start of the pipeline. This does not
        // apply to 'backend' compiles (LTO and ThinLTO link-time pipelines).
        PB.registerPipelineStartEPCallback([&](ModulePassManager &MPM
#if LLVM_VERSION_MAJOR >= 12
                                               ,
                                               OptimizationLevel O
#endif
                                           ) {
          addPassesFromEnvVar(MPM, EnvVarPrefix + "PIPELINESTART_PASSES");
        });

#if LLVM_VERSION_MAJOR >= 13
        // Add optimization right after passes that do basic simplification of
        // the input IR.
        PB.registerPipelineEarlySimplificationEPCallback(
            [](ModulePassManager &MPM, OptimizationLevel O) {
              addPassesFromEnvVar(
                  MPM, EnvVarPrefix + "PIPELINEEARLYSIMPLIFICATION_PASSES");
            });
#endif

#if LLVM_VERSION_MAJOR >= 11
        // Add optimizations at the very end of the function optimization
        // pipeline.
        PB.registerOptimizerLastEPCallback(
            [](ModulePassManager &MPM, OptimizationLevel O) {
              addPassesFromEnvVar(MPM, EnvVarPrefix + "OPTIMIZERLASTEP_PASSES");
            });
#else
        // Add optimizations at the very end of the function optimization
        // pipeline. A key difference between this and the legacy PassManager's
        // OptimizerLast callback is that this extension point is not triggered
        // at O0. Extensions to the O0 pipeline should append their passes to
        // the end of the overall pipeline.
        PB.registerOptimizerLastEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              addPassesFromEnvVar(FPM, EnvVarPrefix + "OPTIMIZERLASTEP_PASSES");
            });
#endif
      }};
}
} // namespace llvm
