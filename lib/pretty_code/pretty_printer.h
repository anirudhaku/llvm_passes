#pragma once
#include "llvm/Pass.h"

using namespace llvm;

namespace cs565 {
	class pretty_printer : public FunctionPass {
    public:
		static char ID;
		pretty_printer()
            : FunctionPass(ID)
        {}
		
		virtual bool runOnFunction(Function &F);
		
		virtual void getAnalysisUsage(AnalysisUsage &Info) const;

	};
}
