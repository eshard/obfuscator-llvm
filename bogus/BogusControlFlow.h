//===- BogusControlFlow.h - BogusControlFlow Obfuscation pass--------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===-----------------------------------------------------------------------===//
//
// This file contains includes and defines for the bogusControlFlow pass
//
//===-----------------------------------------------------------------------===//

#ifndef _BOGUSCONTROLFLOW_H_
#define _BOGUSCONTROLFLOW_H_

// LLVM include
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <list>

namespace llvm {
struct BogusControlFlow {
  BogusControlFlow();
  BogusControlFlow(bool flag);
  bool flag;

  bool runBogusControlFlow(Function &F);
  void bogus(Function &F);

  /* addBogusFlow
   *
   * Add bogus flow to a given basic block, according to the header's
   * description
   */
  void addBogusFlow(BasicBlock *basicBlock, Function &F);

  /* createAlteredBasicBlock
   *
   * This function return a basic block similar to a given one.
   * It's inserted just after the given basic block.
   * The instructions are similar but junk instructions are added between
   * the cloned one. The cloned instructions' phi nodes, metadatas, uses and
   * debug locations are adjusted to fit in the cloned basic block and
   * behave nicely.
   */
  BasicBlock *createAlteredBasicBlock(BasicBlock *basicBlock,
                                      const Twine &Name = "gen",
                                      Function *F = 0);

  /* doFinalization
   *
   * Overwrite FunctionPass method to apply the transformations to the whole
   * module. This part obfuscate all the always true predicates of the module.
   * More precisely, the condition which predicate is FCMP_TRUE.
   * It also remove all the functions' basic blocks' and instructions' names.
   */
  bool doF(Module &M);
};

struct LegacyBogusControlFlow : public FunctionPass, public BogusControlFlow {
  static char ID; // Pass identification
  LegacyBogusControlFlow();
  LegacyBogusControlFlow(bool flag);

  /* runOnFunction
   *
   * Overwrite FunctionPass method to apply the transformation
   * to the function. See header for more details.
   */
  bool runOnFunction(Function &F);

}; // end of struct BogusControlFlow : public FunctionPass

struct BogusControlFlowPass : public PassInfoMixin<BogusControlFlowPass>,
                              public BogusControlFlow {
  BogusControlFlowPass();
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace llvm
#endif
