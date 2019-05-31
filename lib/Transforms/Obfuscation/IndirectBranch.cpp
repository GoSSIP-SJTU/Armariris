/*
    LLVM Indirect Branching Pass
    Copyright (C) 2017 Zhang(https://github.com/Naville/)
    For the latest version of this license, please refer to [Hikari/wiki/License](https://github.com/HikariObfuscator/Hikari/wiki/License)

    // Hikari is relicensed from Obfuscator-LLVM and LLVM upstream's permissive NCSA license
    // to GNU Affero General Public License Version 3 with exceptions listed below.
    // tl;dr: The obfuscated LLVM IR and/or obfuscated binary is not restricted in anyway,
    // however any other project containing code from Hikari needs to be open source and licensed under AGPLV3 as well, even for web-based obfuscation services.
    //
    // Exceptions:
    // Anyone who has associated with ByteDance in anyway at any past, current, future time point is prohibited from direct using this piece of software or create any derivative from it
    //
    //===----------------------------------------------------------------------===//
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/CryptoUtils.h"
#include "llvm/Transforms/Obfuscation/Utils.h"
#include "llvm/Transforms/Obfuscation/IndirectBranch.h"
using namespace llvm;

using namespace std;
namespace llvm{
struct IndirectBranch : public FunctionPass {
  static char ID;
  bool flag;
  bool initialized;
  map<BasicBlock *, unsigned long long> indexmap;
  IndirectBranch() : FunctionPass(ID) {
    this->flag = true;
    this->initialized = false;
  }
  IndirectBranch(bool flag) : FunctionPass(ID) {
    this->flag = flag;
    this->initialized = false;
  }
  //StringRef getPassName() const override { return StringRef("IndirectBranch"); }
   bool initialize(Module &M) {
    vector<Constant *> BBs;
    unsigned long long i = 0;
    for (auto F = M.begin(); F != M.end(); F++) {
      for (auto BB = F->begin(); BB != F->end(); BB++) {
        BasicBlock *BBPtr = &*BB;
        if (BBPtr != &(BBPtr->getParent()->getEntryBlock())) {
          indexmap[BBPtr] = i++;
          BBs.push_back(BlockAddress::get(BBPtr));
        }
      }
    }
    ArrayType *AT =
        ArrayType::get(Type::getInt8PtrTy(M.getContext()), BBs.size());
    Constant *BlockAddressArray =
        ConstantArray::get(AT, ArrayRef<Constant *>(BBs));
    /*
    GlobalVariable *Table = new GlobalVariable(
        M, AT, false, GlobalValue::LinkageTypes::PrivateLinkage,
        BlockAddressArray, "IndirectBranchingGlobalTable");
    appendToCompilerUsed(M, {Table});
    */
    // std::string global_tab_name = "IndirectBranchingGlobalTable";
    // M.getOrInsertGlobal(global_tab_name,AT);
    // llvm::GlobalVariable* tmp = M.getNamedGlobal(global_tab_name);
    // tmp->setInitializer(BlockAddressArray);
    // tmp->setSection("");
    return true;
  }
  bool runOnFunction(Function &Func) override {
    if (!toObfuscate(flag, &Func, "indibr")) {
      return false;
    }
    // if (this->initialized == false) {
    //   initialize(*Func.getParent());
    //   this->initialized = true;
    // }
    //errs() << "Running IndirectBranch On " << Func.getName() << "\n";
    vector<BranchInst *> BIs;
    for (inst_iterator I = inst_begin(Func); I != inst_end(Func); I++) {
      Instruction *Inst = &(*I);
      BranchInst *BI = dyn_cast<BranchInst>(Inst);
      if ( BI
      ) {
        BIs.push_back(BI);
      }
    } // Finish collecting branching conditions
    Value *zero =
        ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()), 0);
    Value *one =
        ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()), 1);
    for (BranchInst *BI : BIs) {
      IRBuilder<> IRB(BI);
      IRBuilder<> INIT_F(&Func.front().front());
      vector<BasicBlock *> BBs;
      // We use the condition's evaluation result to generate the GEP
      // instruction  False evaluates to 0 while true evaluates to 1.  So here
      // we insert the false block first
      if (BI->isConditional()) {
        BBs.push_back(BI->getSuccessor(1));
      }
      BBs.push_back(BI->getSuccessor(0));
      ArrayType *AT = ArrayType::get(
          Type::getInt8PtrTy(Func.getParent()->getContext()), BBs.size());
      vector<Constant *> BlockAddresses;
      for (unsigned i = 0; i < BBs.size(); i++) {
        BlockAddresses.push_back(BlockAddress::get(BBs[i]));
      }
      Value *LoadFrom = nullptr;
      Value *index = nullptr;
      Value *br_addr = nullptr;
      if (BI->isConditional()) {
        // Create a new GV
        // Constant *BlockAddressArray =
        //     ConstantArray::get(AT, ArrayRef<Constant *>(BlockAddresses));

        // LoadFrom = new GlobalVariable(
        //     *Func.getParent(), AT, false,
        //     GlobalValue::LinkageTypes::InternalLinkage, BlockAddressArray,
        //     "HikariConditionalLocalIndirectBranchingTable_"+getRandomName());
        // LoadFrom->setSection("");
        // appendToCompilerUsed(*Func.getParent(), {LoadFrom});

       /*
        std::string local_name = "local_name."+getRandomName();
        Func.getParent().getOrInsertGlobal(local_name,AT);
        LoadFrom = M.getNamedGlobal(local_name);
        tmp->setInitializer(BlockAddressArray);
        tmp->setSection("");
        */
        LoadFrom = new AllocaInst(AT,0,"",&Func.front().front());
        Value *local_gep0 = INIT_F.CreateGEP(LoadFrom, {zero, zero});
        INIT_F.CreateStore(BlockAddresses[0],local_gep0);
        Value *local_gep1 = INIT_F.CreateGEP(LoadFrom, {zero, one});
        INIT_F.CreateStore(BlockAddresses[1],local_gep1);

        Value *condition = BI->getCondition();
        index = IRB.CreateZExt(condition, Type::getInt32Ty(Func.getParent()->getContext()));
        Value *GEP = IRB.CreateGEP(LoadFrom, {zero, index});
        br_addr = IRB.CreateLoad(GEP);
        } else {
            //int random_index = cryptoutils->get_uint64_t() % 2;
            //  index =  ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()),
            //                  indexmap[BI->getSuccessor(0)]);
            LoadFrom = new AllocaInst(Type::getInt8PtrTy(Func.getParent()->getContext()),0,"",&Func.front().front());
            INIT_F.CreateStore(BlockAddress::get(BI->getSuccessor(0)),LoadFrom);
            br_addr = IRB.CreateLoad(LoadFrom);
            //ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()), 1);
      }

        IndirectBrInst *indirBr = IndirectBrInst::Create(br_addr, BBs.size(),BI);
        for (BasicBlock *BB : BBs) {
           indirBr->addDestination(BB);
         }
        // ReplaceInstWithInst(BI, indirBr);
        BI->eraseFromParent();
      // Value *index = NULL;
      // if (BI->isConditional()||
      //     indexmap.find(BI->getSuccessor(0)) == indexmap.end()) {
      //     Value *condition = BI->getCondition();
      //     index = IRB.CreateZExt(condition, Type::getInt32Ty(Func.getParent()->getContext()));
      // } else {
      //   index =
      //       ConstantInt::get(Type::getInt32Ty(Func.getParent()->getContext()),
      //                        indexmap[BI->getSuccessor(0)]);
      // }
      // Value *GEP = IRB.CreateGEP(LoadFrom, {zero, index});
      // LoadInst *LI = IRB.CreateLoad(GEP, "IndirectBranchingTargetAddress");
      // IndirectBrInst *indirBr = IndirectBrInst::Create(LI, BBs.size());
      // for (BasicBlock *BB : BBs) {
      //   indirBr->addDestination(BB);
      // }
      // // ReplaceInstWithInst(BI, indirBr);
    }
    return true;
  }
  virtual bool doFinalization(Module &M) override {
    indexmap.clear();
    initialized = false;
    return false;
  }
};
} // namespace llvm
char IndirectBranch::ID = 0;
static RegisterPass<IndirectBranch> X("indirect branch", "indirect branch");
//INITIALIZE_PASS(IndirectBranch, "indibran", "IndirectBranching", true, true)
FunctionPass *llvm::createIndirectBranchPass() { return new IndirectBranch(); }
FunctionPass *llvm::createIndirectBranchPass(bool flag) {
  return new IndirectBranch(flag);
}
