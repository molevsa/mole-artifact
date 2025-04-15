//
// Created by pro on 2024/10/13.
//

#include "istool/fta/fta.h"
#include "istool/fta/fta_merge.h"
#include "istool/basic/config.h"

using namespace fta;
using fta::merge::util::SplitHolder;
using fta::merge::util::SplitHolderList;

namespace {
    struct GarbageInfo {
        FTANode* base;
        FixedVector<FTANode*> feature;
        FTANode* merge;
        GarbageInfo(FTANode* _base, FixedVector<FTANode*>&& _feature, FTANode* _merge):
            base(_base), feature(std::move(_feature)), merge(_merge) {
        }
    };

    void _bidForwardStep(FTANode *base, const std::vector<trie::FTAEdgeHolder> &edge_holders,
                         SplitHolderList &holder_list, std::vector<GarbageInfo> &garbage) {
        SplitHolder new_holder(base);
        fta::merge::util::forwardStep(base, edge_holders, holder_list, new_holder);
        auto pre_infos = holder_list[INDEX(base)].moveOutInfoList();
        auto new_infos = new_holder.moveOutInfoList();
        auto& result = holder_list[INDEX(base)].map;
        result.clear();
        FixedVectorCmp<FTANode *> cmp;
        auto x = pre_infos.begin(), y = new_infos.begin();
        while (x != pre_infos.end() && y != new_infos.end()) {
            auto sign = cmp.getSign(x->first, y->first);
            if (sign == -1) {
                garbage.emplace_back(base, std::move(x->first), x->second);
                x++;
            } else if (sign == 0) {
#ifdef DEBUG
                assert(x->second->edge_list.empty());
#endif
                x->second->edge_list = std::move(y->second->edge_list);
                result.insert(std::make_pair(std::move(x->first), x->second));
                delete y->second;
                x++; y++;
            } else {
                result.insert(std::make_pair(std::move(y->first), y->second));
                y++;
            }
        }
        for (; x != pre_infos.end(); ++x) garbage.emplace_back(base, std::move(x->first), x->second);
        for (; y != new_infos.end(); ++y) {
            result.insert(std::make_pair(std::move(y->first), y->second));
        }
    }

    using trie::PTrieNode;
    using trie::TrieNode;

    PTrieNode _buildSplitTrie(const SplitHolder& holder) {
        auto root = std::make_shared<TrieNode>();
        for (auto& [feature, node]: holder.map) {
            auto* now = root.get();
            for (auto* other: feature) {
                now = now->nextWithInsert(INDEX(other));
            }
#ifdef DEBUG
            assert(!now->y_node);
#endif
            now->y_node = node;
        }
        return root;
    }


    using fta::merge::util::backward::EdgeGroup;
    using fta::merge::util::backward::EdgeFeature;
    using fta::merge::util::backward::EdgeStorage;

    bool _bidStepOnTrie(const FixedVector<TrieNode*>& trie_list,
                     FixedVector<TrieNode*>& new_trie_list,
                     const FixedVector<FTANode*>& node_list) {
#ifdef DEBUG
        assert(trie_list.size() == new_trie_list.size() && trie_list.size() == node_list.size());
#endif
        for (int i = 0; i < trie_list.size(); ++i) {
            if (trie_list[i]) {
                auto* next = trie_list[i]->next(INDEX(node_list[i]));
                if (!next) return false;
                new_trie_list[i] = next;
            } else {
                new_trie_list[i] = nullptr;
            }
        }
        return true;
    }

    void _combineEdgeOnTrie(int index, const EdgeStorage& storage,
                            FixedVector<FTAEdge*>& tmp,
                            const FixedVector<SplitHolder*>& holder_list,
                            const FixedVector<TrieNode*>& trie_list,
                            SmallVector<FixedVector<FTANode*>>& result) {
        if (index == storage.size()) {
            FixedVector<FTANode*> children(holder_list.size());
            for (int child_index = 0; child_index < holder_list.size(); ++child_index) {
                if (trie_list[child_index]) {
#ifdef DEBUG
                    assert(trie_list[child_index]->y_node);
#endif
                    children[child_index] = trie_list[child_index]->y_node;
                } else {
                    FixedVector<FTANode*> node_list(tmp.size());
                    for (int edge_index = 0; edge_index < tmp.size(); ++edge_index) {
                        node_list[edge_index] = tmp[edge_index]->node_list[child_index];
                    }
                    children[child_index] = holder_list[child_index]->query(std::move(node_list));
                }
            }
            result.append(std::move(children));
            return;
        }
        FixedVector<TrieNode*> new_trie_list(trie_list.size());
        for (auto* edge: storage[index]) {
            if (!_bidStepOnTrie(trie_list, new_trie_list, edge->node_list)) continue;
            tmp[index - 1] = edge;
            _combineEdgeOnTrie(index + 1, storage, tmp, holder_list, new_trie_list, result);
        }
    }

    SmallVector<FixedVector<FTANode*>> _combineEdgeOnTrie(
            const EdgeStorage & storage,
            SplitHolderList& holder_list,
            const FixedVector<PTrieNode>& root_list
            ) {
        SmallVector<FixedVector<FTANode*>> result(config::KDefaultSmallVecSize);
        for (auto* first_edge: storage[0]) {
            FixedVector<SplitHolder*> base_holder_list(first_edge->node_list.size());
            FixedVector<TrieNode*> trie_list(first_edge->node_list.size());
            for (int i = 0; i < first_edge->node_list.size(); ++i) {
                auto index = INDEX(first_edge->node_list[i]);
                base_holder_list[i] = &holder_list[index];
                trie_list[i] = root_list[index].get();
            }
            FixedVector<FTAEdge*> tmp_list(storage.size() - 1);
            _combineEdgeOnTrie(1, storage, tmp_list, base_holder_list, trie_list, result);
        }
        return result;
    }

    void _bidBackwardStep(FTANode* base,
                          const FixedVector<FixedVector<EdgeGroup>>& edge_holders,
                          SplitHolderList &holder_list,
                          FixedVector<PTrieNode>& root_list) {
        auto& start = holder_list[INDEX(base)];

        for (auto& [others, merged_node]: start.map) {
            FixedVector<EdgeGroup*> group_list(edge_holders.size());
            group_list[0] = &edge_holders[0][INDEX(base)];
            for (int i = 0; i < others.size(); ++i) {
                auto node_index = INDEX(others[i]);
                group_list[i + 1] = &edge_holders[i + 1][node_index];
            }
            for (auto& [sem, edge_storage]: merge::util::backward::mergeEdgeGroup(group_list)) {
                for (auto& children: _combineEdgeOnTrie(edge_storage, holder_list, root_list)) {
                    auto* edge = new FTAEdge(sem, std::move(children));
                    merged_node->edge_list.push_back(edge);
                }
            }
        }
    }
}

PFTA fta::merge::mergeFTACostBasedBidirectional(const FTAList &fta_list, bool is_cut) {
#ifdef DEBUG
    util::checkShared(fta_list);
#endif

    auto* base = fta_list[0].get(); int n = ::util::assignFTAIndex(base);
    FTAList others;
    for (int i = 1; i < fta_list.size(); ++i) {
        others.push_back(fta_list[i]);
    }
    auto holder_list = util::initSplitHolderList(base, n);

    // build data structures for forward merge
    std::vector<trie::FTAEdgeHolder> edge_holders;
    for (const auto& fta: others) {
        edge_holders.push_back(trie::buildEdgeHolder(fta.get()));
    }

    // build data structures for backward merge
    FixedVector<FixedVector<util::backward::EdgeGroup>> edge_group_list(fta_list.size());
    for (int i = 0; i < fta_list.size(); ++i) {
        edge_group_list[i] = util::backward::buildEdgeGroup(fta_list[i].get());
    }
    FixedVector<trie::PTrieNode> split_trie_list(n);
#ifdef DEBUG
    for (auto& trie_root: split_trie_list) assert(!trie_root);
#endif
    auto roots = util::backward::initRoots(base, others, holder_list);

    auto get_forward_cost = [&](int size) -> double {
        double cost = 0;
        for (auto& [_, storage]: base->node_map) {
            for (auto* node: storage[size]) {
                for (auto* edge: node->edge_list) {
                    double edge_cost = 1.0;
                    for (auto* child: edge->node_list) {
                        edge_cost *= holder_list[INDEX(child)].map.size();
                    }
                    cost += edge_cost;
                }
            }
        }
        return cost;
    };
    auto get_backward_cost = [&](int size) -> double {
        double cost = 0;
        for (auto& [_, storage]: base->node_map) {
            for (auto* base_node: storage[size]) {
                for (auto& [others, _]: holder_list[INDEX(base_node)].map) {
                    cost += base_node->edge_list.size();
                    for (auto* other: others) {
                        cost += other->edge_list.size();
                    }
                }
            }
        }
        return cost;
    };

    int forward_size = 1, backward_size = base->size_limit;
    double forward_cost = get_forward_cost(forward_size);
    double backward_cost = get_backward_cost(backward_size);
    std::vector<GarbageInfo> garbage;
    while (forward_size < backward_size) {
        // LOG(INFO) << "forward: " << forward_size << "@" << forward_cost << ", backward: " << backward_size << "@" << backward_cost;
        if (forward_cost < backward_cost) {
            // LOG(INFO) << "go forward";
            for (auto& [_, storage]: base->node_map) {
                for (auto* base_node: storage[forward_size]) {
                    _bidForwardStep(base_node, edge_holders, holder_list, garbage);
                    split_trie_list[INDEX(base_node)] = _buildSplitTrie(holder_list[INDEX(base_node)]);
                }
            }
            forward_size++;
            if (forward_size < backward_size) {
                forward_cost = get_forward_cost(forward_size);
            }
        } else {
            // LOG(INFO) << "go backward";
            for (auto& [_, storage]: base->node_map) {
                for (auto* base_node: storage[backward_size]) {
                    _bidBackwardStep(base_node, edge_group_list, holder_list, split_trie_list);
                }
            }
            backward_size--;
            if (forward_size < backward_size) {
                backward_cost = get_backward_cost(backward_size);
            }
        }
    }

    for (auto& info: garbage) {
#ifdef DEBUG
        auto& map = holder_list[INDEX(info.base)].map;
        assert(map.find(info.feature) == map.end());
#endif
        holder_list[INDEX(info.base)].map.insert(std::make_pair(std::move(info.feature), info.merge));
    }
    auto fta = util::createRawFTA(base, holder_list, roots, util::mergeInfoList(fta_list));
    if (is_cut) fta->fullyCut();
    return fta;
}

PFTA fta::merge::mergeFTAStagedBidirectional(const FTAList &fta_list, bool is_cut, double alpha) {
#ifdef DEBUG
    util::checkShared(fta_list);
#endif

    auto* base = fta_list[0].get(); int n = ::util::assignFTAIndex(base);
    FTAList others;
    for (int i = 1; i < fta_list.size(); ++i) {
        fta::util::assignFTAIndex(fta_list[i].get());
        others.push_back(fta_list[i]);
    }
    auto holder_list = util::initSplitHolderList(base, n);

    // build data structures for forward merge
    std::vector<trie::FTAEdgeHolder> edge_holders;
    for (const auto& fta: others) {
        edge_holders.push_back(trie::buildEdgeHolder(fta.get()));
    }

    // build data structures for backward merge
    FixedVector<FixedVector<util::backward::EdgeGroup>> edge_group_list(fta_list.size());
    for (int i = 0; i < fta_list.size(); ++i) {
        edge_group_list[i] = util::backward::buildEdgeGroup(fta_list[i].get());
    }
    FixedVector<trie::PTrieNode> split_trie_list(n);
#ifdef DEBUG
    for (auto& trie_root: split_trie_list) assert(!trie_root);
#endif
    auto roots = util::backward::initRoots(base, others, holder_list);

    int mid_size = std::max(1, int(base->size_limit * alpha));
    for (int size = 1; size <= mid_size; ++size) {
        LOG(INFO) << "Forward " << size;
        for (auto& [_, storage]: base->node_map) {
            for (auto* base_node: storage[size]) {
                util::forwardStep(base_node, edge_holders, holder_list, holder_list[INDEX(base_node)]);
                split_trie_list[INDEX(base_node)] = _buildSplitTrie(holder_list[INDEX(base_node)]);
            }
        }
    }

    for (int size = base->size_limit; size > mid_size; --size) {
        LOG(INFO) << "Backward " << size;
        for (auto& [_, storage]: base->node_map) {
            for (auto* base_node: storage[size]) {
                _bidBackwardStep(base_node, edge_group_list, holder_list, split_trie_list);
            }
        }
    }

    auto fta = util::createRawFTA(base, holder_list, roots, util::mergeInfoList(fta_list));
    if (is_cut) fta->fullyCut();
    return fta;
}