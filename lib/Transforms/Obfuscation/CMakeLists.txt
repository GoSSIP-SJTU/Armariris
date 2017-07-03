add_llvm_library(LLVMObfuscation
  CryptoUtils.cpp
  StringObfuscation.cpp
  Substitution.cpp
  Flattening.cpp
  Utils.cpp
  )

add_dependencies(LLVMObfuscation intrinsics_gen)
