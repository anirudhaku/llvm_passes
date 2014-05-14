#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include "pretty_printer.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"

namespace cs565 {
    void print_value(const Value *v)
    {
        const ConstantInt *value = dyn_cast<ConstantInt>(v);
        if (value == nullptr) {
            if (v->getName() == "") {
                v->printAsOperand(errs(), false);
            } else {
                errs() << v->getName();
            }
        } else {
            errs() << value->getValue();
        }

    }

	bool pretty_printer::runOnFunction(Function &func) {
        int inst_count = 1;
		errs() << "FUNCTION ";
		errs().write_escaped(func.getName()) << "\n\n";
        
        for (BasicBlock &bb : func) {
            errs() << "BASIC BLOCK " << bb.getName() << ":\n";
            for (Instruction &inst : bb) {
                errs()
                    << '%' << inst_count ++ << '\t'
                    << inst.getOpcodeName() << '\t'
                ;

                for (const Value *v :
                        iterator_range<User::value_op_iterator>(
                            inst.value_op_begin(),
                            inst.value_op_end() - 1)) {
                    print_value(v);
                    errs() << ", ";
                }
                print_value(*(inst.op_end() - 1));

                errs() << '\n';
            }

            errs() << '\n';
        }
		
		return false;
	}
	
	void pretty_printer::getAnalysisUsage(AnalysisUsage &Info) const {
		Info.setPreservesAll();
	}
}

char cs565::pretty_printer::ID = 0;
static RegisterPass<cs565::pretty_printer> X("pretty-print",
    "(CS 565) - Pretty Print LLVM IR",
    false,
    false);
