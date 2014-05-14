#pragma once
#include "llvm/Pass.h"

using namespace llvm;

namespace cs565 {
	class naive_trimmer : public FunctionPass {
    public:
		static char ID;
		naive_trimmer()
            : FunctionPass(ID)
        {}
		
		virtual bool runOnFunction(Function &F);
	};
}
