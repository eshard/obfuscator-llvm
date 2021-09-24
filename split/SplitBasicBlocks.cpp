//===- SplitBasicBlock.cpp - SplitBasicBlokc Obfuscation pass--------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the split basic block pass
//
//===----------------------------------------------------------------------===//

#include "SplitBasicBlocks.h"
#include "utils/CryptoUtils.h"
#include "utils/Utils.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#define DEBUG_TYPE "split"

using namespace std;

namespace llvm {

// Stats
STATISTIC(Split, "Basicblock splitted");

static cl::opt<int> SplitNum("split_num", cl::init(2),
                             cl::desc("Split <split_num> time each BB"));

namespace {
struct SplitBasicBlock {
  bool flag;

  SplitBasicBlock() {}

  bool runSplitBasicBlock(Function &F);
  void split(Function *f);

  bool containsPHI(BasicBlock *b);
  void shuffle(std::vector<int> &vec);
};

struct LegacySplitBasicBlock : public FunctionPass, public SplitBasicBlock {
  static char ID; // Pass identification, replacement for typeid

  LegacySplitBasicBlock() : FunctionPass(ID) {}
  LegacySplitBasicBlock(bool flag) : FunctionPass(ID) { this->flag = flag; }

  bool runOnFunction(Function &F);
};

struct SplitBasicBlockPass : public PassInfoMixin<SplitBasicBlockPass>,
                             public SplitBasicBlock {
  SplitBasicBlockPass() {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // namespace

char LegacySplitBasicBlock::ID = 0;
static RegisterPass<LegacySplitBasicBlock> X("splitbbl",
                                             "BasicBlock splitting");

Pass *createSplitBasicBlock(bool flag) {
  return new LegacySplitBasicBlock(flag);
}

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "SplitBasicBlockPass", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "split-basic-block-obfuscator-pass") {
                    FPM.addPass(SplitBasicBlockPass());
                    return true;
                  }
                  return false;
                });
          }};
}

bool LegacySplitBasicBlock::runOnFunction(Function &F) {
  return runSplitBasicBlock(F);
}

PreservedAnalyses SplitBasicBlockPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  return runSplitBasicBlock(F) ? PreservedAnalyses::none()
                               : PreservedAnalyses::all();
}

bool SplitBasicBlock::runSplitBasicBlock(Function &F) {
  // Check if the number of applications is correct
  if (!((SplitNum > 1) && (SplitNum <= 10))) {
    errs() << "Split application basic block percentage\
            -split_num=x must be 1 < x <= 10";
    return false;
  }

  Function *tmp = &F;

  // Do we obfuscate
  if (toObfuscate(flag, tmp, "split")) {
    split(tmp);
    ++Split;
  }

  return false;
}

void SplitBasicBlock::split(Function *f) {
  std::vector<BasicBlock *> origBB;
  int splitN = SplitNum;

  // Save all basic blocks
  for (Function::iterator I = f->begin(), IE = f->end(); I != IE; ++I) {
    origBB.push_back(&*I);
  }

  for (std::vector<BasicBlock *>::iterator I = origBB.begin(),
                                           IE = origBB.end();
       I != IE; ++I) {
    BasicBlock *curr = *I;

    // No need to split a 1 inst bb
    // Or ones containing a PHI node
    if (curr->size() < 2 || containsPHI(curr)) {
      continue;
    }

    // Check splitN and current BB size
    if ((size_t)splitN > curr->size()) {
      splitN = curr->size() - 1;
    }

    // Generate splits point
    std::vector<int> test;
    for (unsigned i = 1; i < curr->size(); ++i) {
      test.push_back(i);
    }

    // Shuffle
    if (test.size() != 1) {
      shuffle(test);
      std::sort(test.begin(), test.begin() + splitN);
    }

    // Split
    BasicBlock::iterator it = curr->begin();
    BasicBlock *toSplit = curr;
    int last = 0;
    for (int i = 0; i < splitN; ++i) {
      for (int j = 0; j < test[i] - last; ++j) {
        ++it;
      }
      last = test[i];
      if (toSplit->size() < 2)
        continue;
      toSplit = toSplit->splitBasicBlock(it, toSplit->getName() + ".split");
    }

    ++Split;
  }
}

bool SplitBasicBlock::containsPHI(BasicBlock *b) {
  for (BasicBlock::iterator I = b->begin(), IE = b->end(); I != IE; ++I) {
    if (isa<PHINode>(I)) {
      return true;
    }
  }
  return false;
}

void SplitBasicBlock::shuffle(std::vector<int> &vec) {
  int n = vec.size();
  for (int i = n - 1; i > 0; --i) {
    std::swap(vec[i], vec[cryptoutils->get_uint32_t() % (i + 1)]);
  }
}

} // namespace llvm
