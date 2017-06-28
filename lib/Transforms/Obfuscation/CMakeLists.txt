add_llvm_library(LLVMObfuscation
  CryptoUtils.cpp
  StringObfuscation.cpp
  )

add_dependencies(LLVMObfuscation intrinsics_gen)
