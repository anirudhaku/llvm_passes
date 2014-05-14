#include "naive_trimmer.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/CFG.h"

namespace cs565 {
	bool naive_trimmer::runOnFunction(Function &func) {
        using namespace std;

        bool modified = false;
        SmallPtrSet<BasicBlock*, 8> visited_bb_set, unreachable_bb_set;

        /*
         * Since we are using external storage, we need not do anything inside
         * the loop, all the visited BasicBlocks will be stored in visited_bb_set
         * by dfs itereator...
         */
        for (auto dfs_it = df_ext_begin(&func.getEntryBlock(), visited_bb_set),
                  dfs_end = df_ext_begin(&func.getEntryBlock(), visited_bb_set);
                dfs_it != dfs_end;
                ++ dfs_it) {}
        
        for (BasicBlock &bb : func) {
            /* check if this bb is visited... */
            if (!visited_bb_set.count(&bb)) {
                errs() << bb.getName() << " was not visited" << '\n';

                /* add this bb to unreachable set... */
                unreachable_bb_set.insert(&bb);
            }
        }

        for (BasicBlock *bb : unreachable_bb_set) {
            bb->eraseFromParent();
            modified = true;
        }
		
		return modified;
	}
}

char cs565::naive_trimmer::ID = 0;
static RegisterPass<cs565::naive_trimmer> X(
    "naive-trim",
    "Removes unreachable Basic Blocks just by looking at CFG",
    false,
    false);
