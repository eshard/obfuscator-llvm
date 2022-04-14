#include "llvm/IR/PassManager.h"
#include "llvm/Passes/OptimizationLevel.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace llvm {

struct Dummy {
  Dummy(const std::string c) : caller_(c) {}

  bool runDummyFunction(Function &F);
  bool runDummyModule(Module &M);

  const std::string caller_;
};

struct DummyFunctionPass : public PassInfoMixin<DummyFunctionPass>,
                           public Dummy {
  DummyFunctionPass(const std::string c) : Dummy(c) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

struct DummyModulePass : public PassInfoMixin<DummyModulePass>, public Dummy {
  DummyModulePass(const std::string c) : Dummy(c) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

PreservedAnalyses DummyFunctionPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  return runDummyFunction(F) ? PreservedAnalyses::none()
                             : PreservedAnalyses::all();
}

PreservedAnalyses DummyModulePass::run(Module &M, ModuleAnalysisManager &AM) {
  return runDummyModule(M) ? PreservedAnalyses::none()
                           : PreservedAnalyses::all();
}

bool Dummy::runDummyFunction(Function &F) {
  outs() << " - "
         << "caller=" << caller_ << "\n";
  return false;
}

bool Dummy::runDummyModule(Module &M) {
  outs() << " - "
         << "caller=" << caller_ << "\n";
  return false;
}

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION, "Dummy plugin", "v1.0", [](PassBuilder &PB) {
        // Add passes that perform peephole optimizations similar to the
        // instruction combiner. These passes will be inserted after each
        // instance of the instruction combiner pass.
        PB.registerPeepholeEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              outs() << "dummy: registerPeepholeEPCallback callback\n";
              FPM.addPass(DummyFunctionPass("PeepholeEPCallback"));
            });

        // Add loop passes to the end of the loop optimizer.
        PB.registerLateLoopOptimizationsEPCallback([](LoopPassManager &LPM,
                                                      OptimizationLevel O) {
          outs() << "dummy: registerLateLoopOptimizationsEPCallback callback\n";
        });

        // Add optimization passes after most of the main optimizations, but
        // before the last cleanup-ish optimizations.
        PB.registerScalarOptimizerLateEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              outs()
                  << "dummy: registerScalarOptimizerLateEPCallback callback\n";
              FPM.addPass(DummyFunctionPass("ScalarOptimizerLateEPCallback"));
            });

        // Add CallGraphSCC passes at the end of the main CallGraphSCC passes
        // and before any function simplification passes run by CGPassManager
        PB.registerCGSCCOptimizerLateEPCallback(
            [](CGSCCPassManager &CGPM, OptimizationLevel O) {
              outs()
                  << "dummy: registerCGSCCOptimizerLateEPCallback callback\n";
            });

        // Add optimization passes before the vectorizer and other highly target
        // specific optimization passes are executed.
        PB.registerVectorizerStartEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              outs() << "dummy: registerVectorizerStartEPCallback callback\n";
              FPM.addPass(DummyFunctionPass("VectorizerStartEPCallback"));
            });

        // Add optimization once at the start of the pipeline. This does not
        // apply to 'backend' compiles (LTO and ThinLTO link-time pipelines).
        PB.registerPipelineStartEPCallback([](ModulePassManager &MPM
#if LLVM_VERSION_MAJOR >= 12
                                              ,
                                              OptimizationLevel O
#endif
                                           ) {
          outs() << "dummy: registerPipelineStartEPCallback callback\n";
          MPM.addPass(DummyModulePass("PipelineStartEPCallback"));
        });

#if LLVM_VERSION_MAJOR >= 13
        // Add optimization right after passes that do basic simplification of
        // the input IR.
        PB.registerPipelineEarlySimplificationEPCallback(
            [](ModulePassManager &MPM, OptimizationLevel O) {
              outs() << "dummy: registerPipelineEarlySimplificationEPCallback "
                        "callback\n";
              MPM.addPass(
                  DummyModulePass("PipelineEarlySimplificationEPCallback"));
            });
#endif

#if LLVM_VERSION_MAJOR >= 11
        // Add optimizations at the very end of the function optimization
        // pipeline.
        PB.registerOptimizerLastEPCallback(
            [](ModulePassManager &MPM, OptimizationLevel O) {
              outs() << "dummy: registerOptimizerLastEPCallback callback\n";
              MPM.addPass(DummyModulePass("OptimizerLastEPCallback"));
            });
#else
        // Add optimizations at the very end of the function optimization
        // pipeline. A key difference between this and the legacy PassManager's
        // OptimizerLast callback is that this extension point is not triggered
        // at O0. Extensions to the O0 pipeline should append their passes to
        // the end of the overall pipeline.
        PB.registerOptimizerLastEPCallback(
            [](FunctionPassManager &FPM, OptimizationLevel O) {
              outs() << "dummy: registerOptimizerLastEPCallback callback\n";
              FPM.addPass(DummyFunctionPass("OptimizerLastEPCallback"));
            });
#endif

        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              FPM.addPass(DummyFunctionPass("PipelineParsingCallback"));
              return true;
            });

        PB.registerPipelineParsingCallback(
            [](StringRef Name, ModulePassManager &MPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              MPM.addPass(DummyModulePass("PipelineParsingCallback"));
              return true;
            });
      }};
}
} // namespace llvm
