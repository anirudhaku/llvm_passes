#include <string>
#include <map>
#include <algorithm>

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "live_var_analyzer.h"

namespace cs565 {

    void print_value(const Value &v)
    {
        const ConstantInt *value = dyn_cast<ConstantInt>(&v);
        if (value == nullptr) {
                v.printAsOperand(errs(), false);
        } else {
            errs() << value->getValue();
        }
    }

    /*
     * Creates a BitVector of size sz and initializes multiple indices given by
     * idxs to given value...
     */
    BitVector createBitVector(std::vector<unsigned> idxs, bool val)
    {
        auto max_it = std::max_element(idxs.begin(), idxs.end());
        BitVector b(max_it == idxs.end() ? 0 : *max_it + 1, !val);

        for (unsigned i : idxs) {
            b[i] = val;
        }

        return b;
    }

    /*
     * Set subtraction operator...
     */
    BitVector operator-(const BitVector &lhs, BitVector rhs)
    {
        /* A - B = !Bits(B) & Bits(A) */
        rhs.resize(std::max(lhs.size(), rhs.size()));
        return rhs.flip() &= lhs;
    }

    /*
     * Set union operator...
     */
    BitVector operator|(BitVector lhs, const BitVector &rhs)
    {
        return lhs |= rhs;
    }

    /*
     * prints a BitVector to errs()...
     */
    void print_bit_vector(const BitVector &bv)
    {
        for (size_t i = 0; i < bv.size(); ++ i) {
            errs() << (bv[i] ? '1' : '0');
        }
    }

    class live_variable_analyzer {
    private:
        /* 
         * holds a pair of sets, first of which represents def and second
         * represents use...
         */
        typedef std::pair<BitVector, BitVector> set_pair;
        typedef std::map<const Value *, set_pair> set_pair_map;
        typedef std::map<const Value *, unsigned> value_bit_index_map;

    public:
        live_variable_analyzer(const Function &f)
            : func(f)
        {
            initialize();
        }

        void run()
        {
            errs() << "+++++++++++++++ USE-DEF values ++++++++++++++++" << '\n';
            print_bb_set_pair(use_def_map, std::make_pair("USE", "DEF"));
            errs() << "\n";
            
            data_flow();

            errs() << "+++++++++++++++ IN-OUT values ++++++++++++++++" << '\n';
            print_bb_set_pair(in_out_map, std::make_pair("IN", "OUT"));
        }

    private:
        /* reference to the function object... */
        const Function &func;

        /* value object to bit index map... */
        value_bit_index_map value_idx_map;

        /* instruction and basic block to <use set, def set> pair  map... */
        set_pair_map use_def_map;

        /* basic block to <in set, out set> pair map... */
        set_pair_map in_out_map;

        /*
         * returns the bit index of 'v'. If 'v' doesn't already has an index, it
         * is assigned one...
         */
        unsigned get_bit_index(const Value &v)
        {
            unsigned idx = 0;
            auto it = value_idx_map.find(&v);
            if (it == value_idx_map.end()) {
                idx = value_idx_map.size();
                value_idx_map[&v] = idx;
            } else {
                idx = it->second;
            }

            return idx;
        }

        /*
         * returns a vector containing indices of all the defs (which is never
         * more than one) of given instruction...
         */
        std::vector<unsigned> get_def_indices(const Instruction &inst)
        {
            std::vector<unsigned> def_idxs;
            
            /*
             * Following are not considered definitions for this analysis:
             * 1) Void types (this is the type assigned to insts without lhs).
             */
            if (!inst.getType()->isVoidTy()) {
                /*
                 * assign a bit to this def if not already assigned and add the
                 * bit index to def_idxs...
                 */
                def_idxs.push_back(get_bit_index(inst));
            }

            return def_idxs;
        }

        /*
         * returns a vector containing indices of all the uses of given
         * instruction...
         */
        std::vector<unsigned> get_use_indices(const Instruction &inst)
        {
            std::vector<unsigned> use_idxs;

            /*
             * Iterate through the operands of inst and add them to its use
             * set...
             */
            for (const Use &u : inst.operands()) {
                const Value &v = *u;
                /*
                 * Following are not considered as usable variables for this
                 * analysis:
                 * 1) Labels
                 * 2) Constants
                 */
                if (!dyn_cast<Constant>(&v) && !v.getType()->isLabelTy()) {
                    /*
                     * assign a bit to this def if not already assigned and add
                     * the bit index to use_idxs...
                     */
                    use_idxs.push_back(get_bit_index(v));
                }
            }

            return use_idxs;
        }

        void init_inst_use(const Instruction &inst, set_pair &sp)
        {
            use_def_map[&inst].first =
                createBitVector(get_use_indices(inst), true);

            /* Use(bb) = Use(bb) U (Use(inst) - Def(bb)) */
            sp.first |= (use_def_map[&inst].first - sp.second);

            //print_value_set(sp->first);
            //errs() << '\n';
        }
        
        void init_inst_def(const Instruction &inst, set_pair &sp)
        {
            use_def_map[&inst].second =
                createBitVector(get_def_indices(inst), true);

            /* Def(bb) = Def(bb) U Def(inst) */
            sp.second |= use_def_map[&inst].second;

            //print_value_set(sp->second);
            //errs() << '\n';
        }

        void resolve_phi_to_out(const PHINode *phi)
        {
            for (unsigned i = 0; i < phi->getNumIncomingValues(); ++ i) {
                auto inc_bb = phi->getIncomingBlock(i);
                auto value = phi->getIncomingValue(i);

                /*
                 * Constant arguments to phi block are not considered (beacuse,
                 * well, they are constanst and, hence, not variables)...
                 */
                if (!dyn_cast<Constant>(value)) {
                    unsigned idx = get_bit_index(*value);

                    if (idx >= in_out_map[inc_bb].second.size()) {
                        in_out_map[inc_bb].second.resize(idx + 1);
                    }

                    in_out_map[inc_bb].second.set(idx);
                }
            }
        }

        void initialize()
        {
            /* 
             * Go through all basic blocks to compute their def and use sets...
             */
            for (const BasicBlock &bb : func) {
                auto bb_use_def = std::make_pair(BitVector(), BitVector());

                /*
                 * Go through all the instruction in the basic block to compute
                 * use and def sets...
                 */
                for (const Instruction &inst : bb) {
                    //errs() << inst << '\n';

                    if (const PHINode *phi = dyn_cast<PHINode>(&inst)) {
                        /*
                         * In case of phi instruction, we still need to
                         * initialize def, but don't need to initilize use...
                         */
                        init_inst_def(inst, bb_use_def);
                        resolve_phi_to_out(phi);
                    } else {
                        /* First initialize use and then initialize def... */
                        init_inst_use(inst, bb_use_def);
                        init_inst_def(inst, bb_use_def);
                    }
                }

                use_def_map[&bb] = std::move(bb_use_def);
            }
        }

        void data_flow()
        {
            bool changing = true;

            while (changing) {
                changing = false;

                for (auto po_it = po_begin(&func.getEntryBlock()),
                          po_it_end = po_end(&func.getEntryBlock());
                        po_it != po_it_end;
                        ++ po_it) {
                    auto bb_in_out = in_out_map[*po_it];
                    auto bb_use_def = use_def_map[*po_it];
                    auto new_in = bb_in_out.first;
                    auto new_out = bb_in_out.second;

                    /* Out = U(succ s) In(s) */
                    for (auto succ_it = succ_begin(*po_it),
                              succ_it_end = succ_end(*po_it);
                            succ_it != succ_it_end;
                            ++ succ_it) {
                        new_out |= in_out_map[*succ_it].first;
                    }

                    if (new_out != bb_in_out.second) {
                        changing = true;
                        bb_in_out.second = std::move(new_out);
                    }

                    /* In = Use U (Out - Def) */
                    new_in = bb_use_def.first |
                        (bb_in_out.second - bb_use_def.second);

                    if (new_in != bb_in_out.first) {
                        changing = true;
                        bb_in_out.first = std::move(new_in);
                    }

                    in_out_map[*po_it] = std::move(bb_in_out);
                }
            }
        }

        void print_value_set(const BitVector &set)
        {
            for (size_t i = 0; i < set.size(); ++ i) {
                if (set[i]) {
                    auto value_it = std::find_if(
                        value_idx_map.begin(),
                        value_idx_map.end(),
                        [&] (const value_bit_index_map::value_type &val) {
                            return val.second == i;
                        });
                    print_value(*value_it->first);
                    errs() << ' ';
                }
            }
        }

        void print_bb_inst_use_def(const BasicBlock &bb)
        {
            for (const Instruction &inst: bb) {
                errs() << inst << '\n';

                errs() << "Use: ";
                print_bit_vector(use_def_map[&inst].first);
                errs() << '\n';

                errs() << "Def: ";
                print_bit_vector(use_def_map[&inst].second);
                errs() << '\n';
            }
        }

        void print_bb_set_pair(
              const set_pair_map &m
            , std::pair<std::string, std::string> labels
            , bool print_bb_body = false
            , bool print_inst_use_def = false)
        {
            for (const BasicBlock &bb : func) {
                if (print_inst_use_def) {
                    print_bb_inst_use_def(bb);
                }

                if (print_bb_body) {
                    errs() << bb << '\n';
                } else {
                    errs() << bb.getName() << ":\n";
                }

                if (m.find(&bb) != m.end()) {
                    auto bb_set_pair = m.at(&bb);

                    errs() << '\t' << labels.first << " { ";
                    print_value_set(bb_set_pair.first);
                    errs() << "}\n";

                    errs() << '\t' << labels.second << " { ";
                    print_value_set(bb_set_pair.second);
                    errs() << "}\n\n";
                } else {
                    errs() << "\t<empty>\n";
                }
            }
        }
    };


	bool live_var_analyzer_pass::runOnFunction(Function &func) {
        using namespace std;

        live_variable_analyzer analyzer(func);
        analyzer.run();

        return false;
	}
}

char cs565::live_var_analyzer_pass::ID = 0;
static RegisterPass<cs565::live_var_analyzer_pass> X(
    "analyze-live-var",
    "Prints Live Variable Analysis results on functions.",
    false,
    false);
