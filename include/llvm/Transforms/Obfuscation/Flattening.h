#ifndef _FLATTENING_INCLUDES_
#define _FLATTENING_INCLUDES_


// LLVM include
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Transforms/Utils/Local.h" // For DemoteRegToStack and DemotePHIToStack
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/CryptoUtils.h"
#include "llvm/Transforms/Obfuscation/Utils.h"

// Namespace
using namespace std;

namespace llvm {
	Pass *createFlattening();
	Pass *createFlattening(bool flag);
}

#endif

