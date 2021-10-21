//===- Substitution.h - Substitution Obfuscation pass----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains includes and defines for the substitution pass
//
//===----------------------------------------------------------------------===//

#ifndef _SUBSTITUTIONS_H_
#define _SUBSTITUTIONS_H_

// LLVM include
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"

#define NUMBER_ADD_SUBST 4
#define NUMBER_SUB_SUBST 3
#define NUMBER_AND_SUBST 2
#define NUMBER_OR_SUBST 2
#define NUMBER_XOR_SUBST 2

namespace llvm {
struct Substitution {
  Substitution();
  Substitution(bool flag);

  void registerFuncs();

  bool runSubstitution(Function &F);
  bool substitute(Function *f);

  void addNeg(BinaryOperator *bo);
  void addDoubleNeg(BinaryOperator *bo);
  void addRand(BinaryOperator *bo);
  void addRand2(BinaryOperator *bo);

  void subNeg(BinaryOperator *bo);
  void subRand(BinaryOperator *bo);
  void subRand2(BinaryOperator *bo);

  void andSubstitution(BinaryOperator *bo);
  void andSubstitutionRand(BinaryOperator *bo);

  void orSubstitution(BinaryOperator *bo);
  void orSubstitutionRand(BinaryOperator *bo);

  void xorSubstitution(BinaryOperator *bo);
  void xorSubstitutionRand(BinaryOperator *bo);

  void (Substitution::*funcAdd[NUMBER_ADD_SUBST])(BinaryOperator *bo);
  void (Substitution::*funcSub[NUMBER_SUB_SUBST])(BinaryOperator *bo);
  void (Substitution::*funcAnd[NUMBER_AND_SUBST])(BinaryOperator *bo);
  void (Substitution::*funcOr[NUMBER_OR_SUBST])(BinaryOperator *bo);
  void (Substitution::*funcXor[NUMBER_XOR_SUBST])(BinaryOperator *bo);
  bool flag;
};

struct SubstitutionPass : public PassInfoMixin<SubstitutionPass>,
                          public Substitution {
  SubstitutionPass();
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace llvm

#endif
