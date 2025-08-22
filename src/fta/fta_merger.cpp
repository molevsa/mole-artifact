//
// Created by pro on 2024/9/18.
//

#include <cassert>
#include <unordered_set>

#include "glog/logging.h"
#include "istool/fta/fta.h"
#include "istool/fta/fta_edge_trie.h"
#include "istool/fta/fta_merge.h"

using namespace fta;

namespace {
template <typename T>
std::vector<T> concat(const std::vector<T>& xs, const std::vector<T>& ys) {
    auto res = xs;
    for (auto& v : ys) res.push_back(v);
    return res;
}
}  // namespace

namespace {
struct MergeInfo {
    FTANode* merged;
    int y_index;
    MergeInfo(FTANode* _merged, int _y_index)
        : merged(_merged), y_index(_y_index) {}
};
}  // namespace

namespace {
void _mergeOnTrie(
    trie::TrieNode* node, int index, FixedVector<FTANode*>& tmp,
    const std::vector<std::vector<MergeInfo>*>& storage,
    std::vector<std::pair<FixedVector<FTANode*>, FTANode*>>& result) {
    if (index == storage.size()) {
        assert(node->y_node);
        result.emplace_back(tmp.clone(), node->y_node);
        return;
    }
    for (auto& choice : *storage[index]) {
        auto it = node->edges.find(choice.y_index);
        if (it != node->edges.end()) {
            tmp[index] = choice.merged;
            _mergeOnTrie(it->second, index + 1, tmp, storage, result);
        }
    }
}

namespace {
template <typename T>
void _cartesianProduct(int index, FixedVector<T>& tmp,
                       const std::vector<std::vector<T>>& storage,
                       std::vector<FixedVector<T>>& res) {
    if (index == tmp.size()) {
        res.emplace_back(tmp.clone());
        return;
    }
    for (auto& choice : storage[index]) {
        tmp[index] = choice;
        _cartesianProduct(index + 1, tmp, storage, res);
    }
}

template <typename T>
std::vector<FixedVector<T>> cartesianProduct(
    const std::vector<std::vector<T>>& storage) {
    std::vector<FixedVector<T>> res;
    FixedVector<T> tmp(storage.size());
    _cartesianProduct(0, tmp, storage, res);
    return res;
}
}  // namespace

std::vector<std::pair<FixedVector<FTANode*>, FTANode*>> mergeOnTrie(
    trie::TrieNode* root, const std::vector<std::vector<MergeInfo>*>& storage) {
    FixedVector<FTANode*> tmp(storage.size());
    std::vector<std::pair<FixedVector<FTANode*>, FTANode*>> result;
    _mergeOnTrie(root, 0, tmp, storage, result);
    return result;
}

#define INDEX(node) (node->getInfo())

void _clearInfo(const std::vector<std::vector<MergeInfo>>& merge_storage) {
    for (auto& merge_list : merge_storage) {
        for (auto& info : merge_list) delete info.merged;
    }
}
}  // namespace

using ::util::assignFTAIndex;

PFTA fta::merge::mergeFTAForward(FTA* x, FTA* y, bool is_cut,
                                 TimeGuard* guard) {
    int x_size = assignFTAIndex(x), y_size = assignFTAIndex(y);
    auto info_list = concat(x->info_list, y->info_list);
    assert(x->grammar == y->grammar && x->size_limit == y->size_limit);
    auto* grammar = x->grammar;
    grammar->indexSymbol();

    // auxiliary data structures
    std::vector<std::vector<MergeInfo>> merge_storage(
        x_size);  // get refined nodes from x
    trie::FTAEdgeHolder y_edge_holder = trie::buildEdgeHolder(y);

    // construct the merged FTA from the x-side
    std::vector<FTANode*> new_node_holder(y_size, nullptr);
    auto get_node = [&](FTANode* x_node, FTANode* y_node,
                        std::vector<MergeInfo>& info_list) {
        assert(x_node->size == y_node->size &&
               x_node->symbol == y_node->symbol);
        auto y_index = y_node->getInfo();
        auto* pre_node = new_node_holder[y_index];
        if (pre_node && pre_node->getInfo() == x_node->getInfo()) {
            return pre_node;
        }
        auto new_info = std::make_shared<BinaryOutputInfo>(x_node->oup_info,
                                                           y_node->oup_info);
        auto* new_node = new FTANode(x_node->symbol, new_info, x_node->size);
        new_node->setInfo(x_node->getInfo());
        info_list.emplace_back(new_node, y_index);
        return new_node_holder[y_index] = new_node;
    };

    for (int size = 1; size <= x->size_limit; ++size) {
        for (auto& [_, storage] : x->node_map) {
            for (auto* x_node : storage[size]) {
                // LOG(INFO) << "Merge for node " << x_node->toString();
                for (auto* x_edge : x_node->edge_list) {
                    // Check time out
                    if (guard && guard->isTimeout()) {
                        _clearInfo(merge_storage);
                        return {};
                    }
                    int child_num = x_edge->node_list.size();

                    // LOG(INFO) << "  Processing edge " << x_edge->toString();
                    std::vector<std::vector<MergeInfo>*> child_storage;
                    for (auto x_child : x_edge->node_list) {
                        child_storage.push_back(
                            &merge_storage[x_child->getInfo()]);
                    }
                    auto edge_feature = trie::getEdgeFeature(
                        x_node->symbol, x_edge->semantics.get());
                    auto it = y_edge_holder.find(edge_feature);
                    if (it == y_edge_holder.end()) continue;
                    // LOG(INFO) << edge_feature << " " << child_storage.size();
                    auto possible_edges =
                        mergeOnTrie(it->second.get(), child_storage);
                    for (auto& [children, y_node] : possible_edges) {
                        auto base_node = get_node(
                            x_node, y_node, merge_storage[x_node->getInfo()]);
                        auto* new_edge =
                            new FTAEdge(x_edge->semantics, std::move(children));
                        // util::verifyFTAEdge(base_node, new_edge, info_list);
                        base_node->edge_list.push_back(new_edge);
                    }
                }
            }
        }
    }

    SymbolNameFTANodeHolder holder;
    for (auto& [symbol_name, storage] : x->node_map) {
        holder[symbol_name] = {};
        for (auto& node_list : storage) {
            FTANodeList new_node_list;
            for (auto* node : node_list) {
                for (auto& merge_info : merge_storage[node->getInfo()]) {
                    new_node_list.push_back(merge_info.merged);
                }
            }
            holder[symbol_name].push_back(new_node_list);
        }
    }

    std::unordered_set<int> y_roots;
    for (auto* node : y->root_list) y_roots.insert(node->getInfo());
    FTANodeList root_list;
    for (auto x_root : x->root_list) {
        for (auto& merge_info : merge_storage[x_root->getInfo()]) {
            auto y_index = merge_info.y_index;
            if (y_roots.find(y_index) != y_roots.end()) {
                root_list.push_back(merge_info.merged);
            }
        }
    }

    auto res = std::make_shared<FTA>(x->grammar, x->size_limit, holder,
                                     root_list, info_list);
    if (is_cut) res->cutBackward();
    return res;
}

PFTA fta::merge::mergeFTAForwardCart(FTA* x, FTA* y, bool is_cut,
                                     TimeGuard* guard) {
    int x_size = assignFTAIndex(x), y_size = assignFTAIndex(y);
    auto info_list = concat(x->info_list, y->info_list);
    assert(x->grammar == y->grammar && x->size_limit == y->size_limit);
    auto* grammar = x->grammar;
    grammar->indexSymbol();

    std::map<std::pair<int, int>, FTANode*> merged_nodes;
    auto get_node = [&](FTANode* x_node, FTANode* y_node) -> fta::FTANode* {
        assert(x_node->size == y_node->size &&
               x_node->symbol == y_node->symbol);
        int x_index = x_node->getInfo(), y_index = y_node->getInfo();
        if (merged_nodes.count(std::make_pair(x_index, y_index))) {
            return merged_nodes[std::make_pair(x_index, y_index)];
        } else {
            return nullptr;
        }
    };

    SymbolNameFTANodeHolder holder;
    for (auto [name, _] : x->node_map)
        holder[name] = FTANodeStorage(x->size_limit + 1);
    for (int size = 1; size <= x->size_limit; ++size) {
        for (auto& [x_symbol, x_storage] : x->node_map) {
            for (auto* x_node : x_storage[size]) {
                for (auto& [y_symbol, y_storage] : y->node_map) {
                    if (x_symbol != y_symbol) {
                        continue;
                    }
                    for (auto* y_node : y_storage[size]) {
                        std::vector<FTAEdge*> edge_list;
                        for (auto* x_edge : x_node->edge_list) {
                            int child_num = x_edge->node_list.size();
                            for (auto* y_edge : y_node->edge_list) {
                                if (x_edge->semantics != y_edge->semantics) {
                                    continue;
                                }
                                auto children =
                                    FixedVector<FTANode*>(child_num);
                                bool can_merge = true;
                                for (int i = 0; i < child_num; i++) {
                                    auto child_x = x_edge->node_list[i],
                                         child_y = y_edge->node_list[i];
                                    if (child_x->size != child_y->size ||
                                        child_x->symbol != child_y->symbol) {
                                        can_merge = false;
                                        break;
                                    }
                                    children[i] = get_node(child_x, child_y);
                                    if (!children[i]) {
                                        can_merge = false;
                                        break;
                                    }
                                }
                                if (can_merge) {
                                    auto* new_edge = new FTAEdge(
                                        x_edge->semantics, std::move(children));
                                    edge_list.push_back(new_edge);
                                }
                            }
                        }
                        if (!edge_list.empty()) {
                            FTANode* node = new FTANode(
                                x_node->symbol,
                                std::make_shared<BinaryOutputInfo>(
                                    x_node->oup_info, y_node->oup_info),
                                x_node->size);
                            node->setInfo(merged_nodes.size());
                            node->edge_list = edge_list;
                            holder[x_node->symbol->name][size].push_back(node);
                            merged_nodes[std::make_pair(
                                x_node->getInfo(), y_node->getInfo())] = node;
                        }
                    }
                }
            }
        }
    }

    FTANodeList root_list;
    for (auto x_root : x->root_list) {
        for (auto y_root : y->root_list) {
            if (merged_nodes.count(
                    std::make_pair(x_root->getInfo(), y_root->getInfo()))) {
                root_list.push_back(merged_nodes[std::make_pair(
                    x_root->getInfo(), y_root->getInfo())]);
            }
        }
    }

    auto res = std::make_shared<FTA>(x->grammar, x->size_limit, holder,
                                     root_list, info_list);
    if (is_cut) res->cutBackward();
    return res;
}

namespace {
struct SplitInfo {
    FTANode *res, *y_node;
    SplitInfo() : res(nullptr), y_node(nullptr) {}
    SplitInfo(FTANode* _res, FTANode* _y_node) : res(_res), y_node(_y_node) {}
};

typedef std::unordered_map<
    std::string, std::pair<std::vector<FTAEdge*>, std::vector<FTAEdge*>>>
    PairEdgeInfo;
PairEdgeInfo _buildEdgeInfo(FTANode* x, FTANode* y) {
    PairEdgeInfo info;
    for (auto* edge : x->edge_list)
        info[edge->semantics->getName()].first.push_back(edge);
    for (auto* edge : y->edge_list)
        info[edge->semantics->getName()].second.push_back(edge);
    return info;
}

void _clearInfo(
    const std::vector<std::unordered_map<int, SplitInfo>>& info_holder) {
    for (auto& map : info_holder) {
        for (auto& [_, info] : map) {
            delete info.res;
        }
    }
}
}  // namespace

PFTA fta::merge::mergeFTABackward(FTA* x, FTA* y, bool is_cut,
                                  TimeGuard* guard) {
    int x_size = assignFTAIndex(x), y_size = assignFTAIndex(y);
    if (x_size < y_size) {
        std::swap(x_size, y_size);
        std::swap(x, y);
    }
#ifdef DEBUG
    assert(x->size_limit == y->size_limit && x->grammar == y->grammar);
#endif

    std::vector<std::unordered_map<int, SplitInfo>> split_holder(x_size);
    FTANodeList root_list;

    auto get_merge_node = [&](FTANode* x_node, FTANode* y_node) -> FTANode* {
        auto& holder = split_holder[INDEX(x_node)];
        auto it = holder.find(INDEX(y_node));
        if (it == holder.end()) {
            auto oup_info = std::make_shared<BinaryOutputInfo>(
                x_node->oup_info, y_node->oup_info);
            auto* new_node =
                new FTANode(x_node->symbol, oup_info, y_node->size);
            holder.insert({INDEX(y_node), SplitInfo(new_node, y_node)});
            return new_node;
        } else {
            return it->second.res;
        }
    };

    for (auto* x_root : x->root_list) {
        for (auto* y_root : y->root_list) {
            if (x_root->size == y_root->size &&
                x_root->symbol->id == y_root->symbol->id) {
                auto* new_root = get_merge_node(x_root, y_root);
                root_list.push_back(new_root);
            }
        }
    }

    for (int size = x->size_limit; size; --size) {
        for (auto& [_symbol, storage] : x->node_map) {
            for (auto* x_node : storage[size]) {
                for (auto& [_y_id, info] : split_holder[INDEX(x_node)]) {
                    auto* y_node = info.y_node;
                    auto* new_node = info.res;
                    auto edge_info = _buildEdgeInfo(x_node, y_node);
                    for (auto& [_rule_name, rules] : edge_info) {
                        // check timeout
                        if (guard && guard->isTimeout()) {
                            _clearInfo(split_holder);
                            return {};
                        }
                        for (auto* x_edge : rules.first) {
                            for (auto* y_edge : rules.second) {
#ifdef DEBUG
                                assert(x_edge->semantics->getName() ==
                                       y_edge->semantics->getName());
                                assert(x_edge->node_list.size() ==
                                       y_edge->node_list.size());
#endif
                                FixedVector<FTANode*> child_list(
                                    x_edge->node_list.size());
                                for (int i = 0; i < x_edge->node_list.size();
                                     ++i) {
                                    child_list[i] =
                                        get_merge_node(x_edge->node_list[i],
                                                       y_edge->node_list[i]);
                                }
                                auto* new_edge = new FTAEdge(
                                    x_edge->semantics, std::move(child_list));
                                new_node->edge_list.push_back(new_edge);
                            }
                        }
                    }
                }
            }
        }
    }

    SymbolNameFTANodeHolder holder;
    for (int size = 0; size <= x->size_limit; ++size) {
        for (auto& [name, storage] : x->node_map) {
            holder[name].emplace_back();
            auto& new_list = holder[name][size];
            for (auto* x_node : storage[size]) {
                for (auto& [_, info] : split_holder[INDEX(x_node)]) {
                    new_list.push_back(info.res);
                }
            }
        }
    }

    auto res =
        std::make_shared<FTA>(x->grammar, x->size_limit, holder, root_list,
                              concat(x->info_list, y->info_list));

    if (is_cut) res->fullyCut();
    return res;
}

namespace {
void _swapMerge(FTA* fta, int left_size) {
    std::vector<ExecuteInfo*> info_list;
    for (int i = left_size; i < fta->info_list.size(); ++i) {
        info_list.push_back(fta->info_list[i]);
    }
    for (int i = 0; i < left_size; ++i) {
        info_list.push_back(fta->info_list[i]);
    }
    fta->info_list = info_list;
    for (auto& [_, holder] : fta->node_map) {
        for (auto& list : holder) {
            for (auto* node : list) {
                auto* info =
                    dynamic_cast<BinaryOutputInfo*>(node->oup_info.get());
                assert(info);
                auto new_info =
                    std::make_shared<BinaryOutputInfo>(info->r, info->l);
                node->oup_info = new_info;
            }
        }
    }
}
}  // namespace

PFTA fta::mergeFTA(FTA* x, FTA* y, MergeType type, bool is_cut,
                   TimeGuard* guard) {
    PFTA result;
    bool is_swapped = false;
    if (x->nodeCount() < y->nodeCount()) {
        std::swap(x, y);
        is_swapped = true;
    }
    switch (type) {
        case FORWARD: {
            result = merge::mergeFTAForward(x, y, is_cut, guard);
            break;
        }
        case BACKWARD: {
            result = merge::mergeFTABackward(x, y, is_cut, guard);
            break;
        }
    }
    if (is_swapped && result) {
        _swapMerge(result.get(), x->info_list.size());
    }
    return result;
}

PFTA fta::mergeFTACart(FTA* x, FTA* y, MergeType type, bool is_cut,
                       TimeGuard* guard) {
    PFTA result;
    bool is_swapped = false;
    if (x->nodeCount() < y->nodeCount()) {
        std::swap(x, y);
        is_swapped = true;
    }
    switch (type) {
        case FORWARD: {
            result = merge::mergeFTAForwardCart(x, y, is_cut, guard);
            break;
        }
        case BACKWARD: {
            // not implemented
            assert(false);
        }
    }
    if (is_swapped && result) {
        _swapMerge(result.get(), x->info_list.size());
    }
    return result;
}

PFTA fta::mergeIndependentFTAForward(const FTAList& fta_list, bool is_cut) {
    auto init =
        fta::grammar2FTA(fta_list[0]->grammar, fta_list[0]->size_limit, true);
    FTAList unit_list;
    for (auto& fta : fta_list)
        unit_list.push_back(
            fta::mergeFTA(init.get(), fta.get(), FORWARD, true));
    return fta::merge::mergeFTAForwardMulti(unit_list, is_cut);
}