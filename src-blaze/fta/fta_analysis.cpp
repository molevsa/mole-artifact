//
// Created by pro on 2024/9/27.
//

#include <cassert>
#include <map>
#include <unordered_set>

#include "glog/logging.h"
#include "istool/fta/fta.h"
#include "istool/fta/fta_value.h"

using namespace fta;

void analysis::analyzeMerge(Grammar *grammar, Env *env,
                            const IOExampleList &example_list, int size) {
    auto fta_value_sets = std::make_shared<value::FTAValueSets>();
    for (int n = 2; n <= 2; ++n) {
        LOG(INFO) << "\n\nTry merge " << n * 2 << " examples";
        {
            LOG(INFO) << "Forward: ";
            auto fta = buildFTA(grammar, env, example_list[0], size, true,
                                fta_value_sets);
            auto cost = 0;
            for (int i = 1; i < (n << 1); ++i) {
                auto next = buildFTA(grammar, env, example_list[i], size, true,
                                     fta_value_sets);
                fta = mergeFTA(fta.get(), next.get(), FORWARD, false);
                cost += fta->edgeCount();
                LOG(INFO) << "  raw " << i << ": " << fta->getSizeInfo();
                LOG(INFO) << "    "
                          << size::sizeInfo2String(
                                 size::getNodeCountVector(fta.get()));
                fta->cutBackward();
                LOG(INFO) << "  fin " << i << ": " << fta->getSizeInfo();
            }
            LOG(INFO) << "Total cost: " << cost;
        }
        /*{
            LOG(INFO) << "Backward: ";
            auto fta = buildFTA(grammar, env, example_list[0], size, true);
            auto cost = 0;
            for (int i = 1; i < (n << 1); ++i) {
                auto next = buildFTA(grammar, env, example_list[i], size, true);
                fta = mergeFTABackward(fta.get(), next.get(), false);
                cost += fta->edgeCount();
                LOG(INFO) << "  raw " << i << ": " << fta->getSizeInfo();
                LOG(INFO) << "    " <<
        size::sizeInfo2String(size::getNodeCountVector(fta.get()));
        fta->fullyCut(); LOG(INFO) << "  fin " << i << ": " <<
        fta->getSizeInfo();
            }
            LOG(INFO) << "Total cost: " << cost;
        }*/
        {
            LOG(INFO) << "Equal merge: ";
            auto fta_left = buildFTA(grammar, env, example_list[0], size, true,
                                     fta_value_sets);
            auto fta_right = buildFTA(grammar, env, example_list[n], size, true,
                                      fta_value_sets);
            int cost = 0;
            for (int i = 1; i < n; ++i) {
                auto next_left = buildFTA(grammar, env, example_list[i], size,
                                          true, fta_value_sets);
                fta_left =
                    mergeFTA(fta_left.get(), next_left.get(), FORWARD, false);
                cost += fta_left->edgeCount();
                fta_left->cutBackward();
                auto next_right = buildFTA(grammar, env, example_list[i + n],
                                           size, true, fta_value_sets);
                fta_right =
                    mergeFTA(fta_right.get(), next_right.get(), FORWARD, false);
                cost += fta_right->edgeCount();
                fta_right->cutBackward();
            }
            LOG(INFO) << "  left:  " << fta_left->getSizeInfo();
            LOG(INFO) << "  right: " << fta_right->getSizeInfo();
            auto forward_final =
                mergeFTA(fta_left.get(), fta_right.get(), FORWARD, false);
            LOG(INFO) << "  forward cost " << forward_final->edgeCount() + cost;
            LOG(INFO) << "    "
                      << size::sizeInfo2String(
                             size::getNodeCountVector(forward_final.get()));
            LOG(INFO) << "  raw: " << forward_final->getSizeInfo();
            forward_final->fullyCut();
            LOG(INFO) << "    "
                      << size::sizeInfo2String(
                             size::getNodeCountVector(forward_final.get()));
            LOG(INFO) << "  fin: " << forward_final->getSizeInfo();
            LOG(INFO) << "Total cost: " << cost;

            /*auto backward_final = mergeFTABackward(fta_left.get(),
            fta_right.get(), false); LOG(INFO) << "  backward cost " <<
            backward_final->edgeCount() + cost; LOG(INFO) << "    " <<
            size::sizeInfo2String(size::getNodeCountVector(backward_final.get()));
            LOG(INFO) << "  raw: " << backward_final->getSizeInfo();
            backward_final->fullyCut();
            LOG(INFO) << "  fin: " << backward_final->getSizeInfo();
            LOG(INFO) << "Total cost: " << cost;*/

            auto first = buildFTA(grammar, env, example_list[0], size, true,
                                  fta_value_sets);
            fta_right = mergeFTA(fta_right.get(), first.get(), FORWARD, false);
            cost += fta_right->edgeCount();
            fta_right->fullyCut();
            LOG(INFO) << "new right: " << fta_right->getSizeInfo();
            /*auto backward_overlap =
            mergeFTABackwardWithOverlapElimination(fta_left.get(),
            fta_right.get(), false); LOG(INFO) << "  backward overlap cost " <<
            backward_overlap->edgeCount() + cost; LOG(INFO) << "    " <<
            size::sizeInfo2String(size::getNodeCountVector(backward_overlap.get()));
            LOG(INFO) << "  raw: " << backward_overlap->getSizeInfo();
            backward_overlap->fullyCut();
            LOG(INFO) << "  fin: " << backward_overlap->getSizeInfo();
            LOG(INFO) << "Total cost: " << cost;*/
        }
    }
}