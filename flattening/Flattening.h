//===- FlatteningIncludes.h - Flattening Obfuscation pass------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains includes and defines for the flattening pass
//
//===----------------------------------------------------------------------===//

#ifndef _FLATTENING_INCLUDES_
#define _FLATTENING_INCLUDES_

// LLVM include
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Local.h" // For DemoteRegToStack and DemotePHIToStack

namespace llvm {
struct Flattening {
  bool flag;

  Flattening() {}

  bool runFlattening(Function &F);
  bool flatten(Function *f);
};

struct FlatteningObfuscatorPass
    : public PassInfoMixin<FlatteningObfuscatorPass>,
      public Flattening {
  FlatteningObfuscatorPass();
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &);
};
} // namespace llvm

#endif
