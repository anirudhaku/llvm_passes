#pragma once
#include "llvm/Pass.h"

using namespace llvm;

namespace cs565 {
	class live_var_analyzer_pass : public FunctionPass {
    public:
		static char ID;
		live_var_analyzer_pass()
            : FunctionPass(ID)
        {}
		
		virtual bool runOnFunction(Function &F);
	};
}
