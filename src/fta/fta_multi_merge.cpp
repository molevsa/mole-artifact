//
// Created by pro on 2024/9/30.
//

#include "istool/fta/fta_edge_trie.h"
#include "istool/fta/fta_merge.h"
#include "istool/basic/small_vector.h"
#include "istool/basic/config.h"
#include "glog/logging.h"
#include <cassert>
#include <map>
#include <set>
#include "istool/basic/config.h"

using namespace fta;
using trie::PTrieNode;
using trie::TrieNode;
using namespace fta::merge::util;

struct CombineInfo {
    FixedVector<FTANode *> others;
    FixedVector<FTANode *> children;

    CombineInfo() = default;
    CombineInfo(FixedVector<FTANode *> &&_others, FixedVector<FTANode *> &&_children):
            others(std::move(_others)), children(std::move(_children)) {
    }
    CombineInfo(CombineInfo &&other): others(std::move(other.others)), children(std::move(other.children)) {
    }
    CombineInfo &operator=(CombineInfo &&other) {
        others = std::move(other.others);
        children = std::move(other.children);
        return *this;
    }
    RemoveCopy(CombineInfo);
};

PInfo merge::util::extractSharedInfo(FTANode *node) {
    auto* bi = dynamic_cast<BinaryOutputInfo*>(node->oup_info.get());
#ifdef DEBUG
    assert(bi);
#endif
    return bi->l;
}

PInfo merge::util::extractNewInfo(FTANode* node) {
    auto* bi = dynamic_cast<BinaryOutputInfo*>(node->oup_info.get());
#ifdef DEBUG
    assert(bi);
#endif
    return bi->r;
}

FTANode *merge::util::createNode(FTANode *base, const FixedVector<FTANode *> &others) {
    FixedVector<PInfo> info_list(others.size() + 2);
    info_list[0] = extractSharedInfo(base);
    info_list[1] = extractNewInfo(base);
    for (int i = 0; i < others.size(); ++i) {
        info_list[i + 2] = extractNewInfo(others[i]);
    }
    auto info = std::make_shared<MultiOutputInfo>(std::move(info_list));
    return new FTANode(base->symbol, info, base->size);
}

FTAEdge *merge::util::createEdge(const PSemantics &semantics, FixedVector<FTANode *> &&children) {
    return new FTAEdge(semantics, std::move(children));
}

SplitHolder::SplitHolder(FTANode *_base): base(_base) {}

FTANode *SplitHolder::query(FixedVector<FTANode*>&& feature) {
    auto it = map.find(feature);
    if (it == map.end()) {
        auto* new_node = createNode(base, feature);
        map.insert(std::make_pair(std::move(feature), new_node));
        return new_node;
    } else {
        return it->second;
    }
}

namespace {
    typedef std::pair<FixedVector<FTANode *>, FTANode *> SplitInfo;
}

FixedVector<SplitInfo> SplitHolder::moveOutInfoList() {
    FixedVector<SplitInfo> result(map.size());
    int counter = 0;
    for (auto it = map.begin(); it != map.end(); ++it, ++counter) {
        // TODO: remove this clone
        result[counter] = std::make_pair(it->first.clone(), it->second);
    }
    return result;
}

std::vector<ExecuteInfo *> merge::util::mergeInfoList(const FTAList &fta_list) {
    auto* base = fta_list[0].get();
    auto share_num = extractSharedInfo(base->root_list[0])->getFullOutput().size();
    std::vector<ExecuteInfo*> info_list = fta_list[0]->info_list;
    for (int i = 1; i < fta_list.size(); ++i) {
        for (int j = share_num; j < fta_list[i]->info_list.size(); ++j) {
            info_list.push_back(fta_list[i]->info_list[j]);
        }
    }
    return info_list;
}

namespace {
    bool _stepOnTrie(const FixedVector<FTANode*>& others, const FixedVector<TrieNode*>& node_list, FixedVector<TrieNode*>& result) {
#ifdef DEBUG
        assert(others.size() == node_list.size() && others.size() == result.size());
#endif
        for (int i = 0; i < others.size(); ++i) {
            auto next = node_list[i]->next(INDEX(others[i]));
            if (!next) return false;
            result[i] = next;
        }
        return true;
    }

    void _mergeForward(
            int index,
            const std::vector<SplitHolder*>& info_holder,
            const FixedVector<TrieNode*>& trie_nodes,
            SplitHolder& result,
            FixedVector<FTANode*>& children,
            const PSemantics& sem
            ) {
        if (index == info_holder.size()) {
            FixedVector<FTANode*> other_nodes(trie_nodes.size());
            for (int i = 0; i < trie_nodes.size(); ++i) {
                other_nodes[i] = trie_nodes[i]->y_node;
#ifdef DEBUG
                assert(other_nodes[i]);
#endif
            }
            auto* res_node = result.query(std::move(other_nodes));
            auto* new_edge = createEdge(sem, children.clone());
            res_node->edge_list.push_back(new_edge);
            return;
        }
        FixedVector<TrieNode*> current(trie_nodes.size());
        auto* info_list = info_holder[index];
        for (auto& [others, node]: info_list->map) {
            if (!_stepOnTrie(others, trie_nodes, current)) continue;
            children[index] = node;
            _mergeForward(index + 1, info_holder, current, result, children, sem);
        }
    }
    void _naiveStepOnTrie(const FixedVector<FTANode*>& others, const FixedVector<TrieNode*>& node_list, FixedVector<TrieNode*>& result) {
        #ifdef DEBUG
                assert(others.size() == node_list.size() && others.size() == result.size());
        #endif
                for (int i = 0; i < others.size(); ++i) {
                    if (!node_list[i]) result[i] = nullptr;
                    else {
                        result[i] = node_list[i]->next(INDEX(others[i]));
                    }
                }
            }
    /*void _mergeForward(
            int index,
            const std::vector<SplitHolder*>& info_holder,
            const FixedVector<TrieNode*>& trie_nodes,
            SplitHolder& result,
            FixedVector<FTANode*>& children,
            const PSemantics& sem
        ) {
        if (index == info_holder.size()) {
            for (auto* node: trie_nodes) {
                if (!node) return;
            }
            FixedVector<FTANode*> other_nodes(trie_nodes.size());
            for (int i = 0; i < trie_nodes.size(); ++i) {
                other_nodes[i] = trie_nodes[i]->y_node;
    #ifdef DEBUG
                    assert(other_nodes[i]);
    #endif
            }
            auto* res_node = result.query(std::move(other_nodes));
            auto* new_edge = createEdge(sem, children.clone());
            res_node->edge_list.push_back(new_edge);
            return;
        }
        FixedVector<TrieNode*> current(trie_nodes.size());
        auto* info_list = info_holder[index];
        for (auto& [others, node]: info_list->map) {
            _naiveStepOnTrie(others, trie_nodes, current);
            children[index] = node;
            _mergeForward(index + 1, info_holder, current, result, children, sem);
        }
    }*/

    void _mergeForward(const std::vector<SplitHolder*>& info_holder,
                       const FixedVector<TrieNode*>& roots,
                       SplitHolder& res, const PSemantics& sem) {
        FixedVector<FTANode*> children(info_holder.size());
        _mergeForward(0, info_holder, roots, res, children, sem);
    }

    std::set<int> _getRootSet(FTA* fta) {
        std::set<int> root_set;
        for (auto* root_node: fta->root_list) root_set.insert(INDEX(root_node));
        return root_set;
    }
}

using ::util::assignFTAIndex;

void fta::merge::util::forwardStep(FTANode *base, const std::vector<trie::FTAEdgeHolder> &edge_holders,
                                   SplitHolderList &holder_list, SplitHolder& result) {
    FixedVector<TrieNode*> trie_roots(edge_holders.size());

    auto get_roots = [&](const std::string& feature) -> bool {
        for (int i = 0; i < edge_holders.size(); ++i) {
            auto it = edge_holders[i].find(feature);
            if (it == edge_holders[i].end()) return false;
            trie_roots[i] = it->second.get();
        }
        return true;
    };

    for (auto* base_edge: base->edge_list) {
        auto feature = trie::getEdgeFeature(base->symbol, base_edge->semantics.get());
        bool is_root_available = true;
        for (int i = 0; i < edge_holders.size(); ++i) {
            auto it = edge_holders[i].find(feature);
            if (it == edge_holders[i].end()) {
                is_root_available = false; break;
            } else {
                trie_roots[i] = it->second.get();
            }
        }
        if (!is_root_available) continue;

        std::vector<SplitHolder*> info_storage;
        bool is_empty = false;
        for (auto* child: base_edge->node_list) {
            auto& info = holder_list[INDEX(child)];
            if (info.map.empty()) {
                is_empty = true; break;
            }
            info_storage.push_back(&info);
        }
        if (is_empty) continue;

        _mergeForward(info_storage, trie_roots, result, base_edge->semantics);
    }
}

PFTA fta::merge::mergeFTAForwardMulti(const FTAList &fta_list, bool is_cut) {
#ifdef DEBUG
    checkShared(fta_list);
#endif
    std::vector<std::pair<int, int>> index_list(fta_list.size());
    for (int i = 0; i < fta_list.size(); ++i) index_list[i] = {-fta_list[i]->nodeCount(), i};
    std::sort(index_list.begin(), index_list.end());

    auto base = fta_list[index_list[0].second].get();
    int n = assignFTAIndex(base);
    FTAList other_list;
    for (int i = 1; i < fta_list.size(); ++i) {
        auto &fta = fta_list[index_list[i].second];
        other_list.push_back(fta);
        assignFTAIndex(fta.get());
    }

    std::vector<trie::FTAEdgeHolder> edge_holders;
    for (const auto &fta: other_list) {
        edge_holders.push_back(trie::buildEdgeHolder(fta.get()));
    }
    auto holder_list = util::initSplitHolderList(base, n);


    for (int size = 1; size <= base->size_limit; ++size) {
        for (auto &[_, storage]: base->node_map) {
            for (auto *base_node: storage[size]) {
                // LOG(INFO) << "start forward";
                forwardStep(base_node, edge_holders, holder_list, holder_list[INDEX(base_node)]);
                // LOG(INFO) << "end forward " << holder_list[INDEX(base_node)].map.size();
            }
        }
    }

    FTANodeList root_list;
    std::vector<std::set<int>> root_set;
    for (auto &fta: other_list) root_set.emplace_back(_getRootSet(fta.get()));
    for (auto *base_root: base->root_list) {
        for (auto &[others, node]: holder_list[INDEX(base_root)].map) {
            bool is_root = true;
            for (int i = 0; i < others.size(); ++i) {
                auto index = INDEX(others[i]);
                if (root_set[i].find(index) == root_set[i].end()) {
                    is_root = false;
                    break;
                }
            }
            if (is_root) root_list.push_back(node);
        }
    }

    std::vector<ExecuteInfo*> info_list;
    for (auto* info: base->info_list) info_list.push_back(info);
    for (auto& other: other_list) {
        for (auto* info: other->info_list) info_list.push_back(info);
    }
    auto fta = util::createRawFTA(base, holder_list, root_list, info_list);
    if (is_cut) {
        fta->cutBackward();
    }
    return fta;
}

#ifdef DEBUG
namespace {
    void _checkShare(FTA* base, FTA* fta) {
        assert(base->grammar == fta->grammar && base->size_limit == fta->size_limit);
        assert(!base->root_list.empty() && !fta->root_list.empty());
        auto* root_x = base->root_list[0];
        auto* root_y = fta->root_list[0];
        assert(extractSharedInfo(root_x)->getFullOutput() == extractSharedInfo(root_y)->getFullOutput());
    }

    void _checkLeftSideUnique(const FTAList& fta_list) {
        auto* grammar = fta_list[0]->grammar;
        for (auto* symbol: grammar->symbol_list) {
            for (int size = 0; size < fta_list[0]->size_limit; ++size) {
                std::unordered_map<std::string, PInfo> info_map;
                for (auto& fta: fta_list) {
                    for (auto* node: fta->node_map[symbol->name][size]) {
                        auto shared_info = extractSharedInfo(node);
                        auto feature = data::dataList2String(shared_info->getFullOutput());
                        auto it = info_map.find(feature);
                        if (it == info_map.end()) {
                            info_map[feature] = shared_info;
                        } else {
                            assert(it->second == shared_info);
                        }
                    }
                }
            }
        }
    }
}
void fta::merge::util::checkShared(const FTAList &fta_list) {
    for (auto& fta: fta_list) _checkShare(fta_list[0].get(), fta.get());
    _checkLeftSideUnique(fta_list);
}
#endif

using fta::merge::util::backward::EdgeFeature;
using fta::merge::util::backward::EdgeGroup;

EdgeFeature::EdgeFeature(EdgeFeature &&feature):
    sem(std::move(feature.sem)), info_list(std::move(feature.info_list)) {
}

EdgeFeature::EdgeFeature(FTAEdge *edge): sem(edge->semantics), info_list(edge->node_list.size()) {
    for (int i = 0; i < edge->node_list.size(); ++i) {
        info_list[i] = extractSharedInfo(edge->node_list[i]).get();
    }
}

std::string EdgeFeature::toString() const {
    if (!info_list.size()) return sem->name + "@[]";
    std::string res = sem->name + "@[";
    for (int i = 0; i < info_list.size(); ++i) {
        if (i) res += ", ";
        res += data::dataList2String(info_list[i]->getFullOutput());
    }
    return res + "]";
}

namespace {
    struct EdgeFeatureCmp {
        FixedVectorCmp<OutputInfo *> cmp;

        int operator()(const EdgeFeature &x, const EdgeFeature &y) const {
            return getSign(x, y) == -1;
        }

        bool isEqual(const EdgeFeature &x, const EdgeFeature &y) const {
            if (x.sem->name != y.sem->name) return false;
#ifdef DEBUG
            assert(x.info_list.size() == y.info_list.size());
#endif
            for (int i = 0; i < x.info_list.size(); ++i) {
                if (x.info_list[i] != y.info_list[i]) return false;
            }
            return true;
        }

        int getSign(const EdgeFeature &x, const EdgeFeature &y) const {
            if (x.sem->name < y.sem->name) return -1;
            if (x.sem->name > y.sem->name) return 1;
#ifdef DEBUG
            assert(x.info_list.size() == y.info_list.size());
#endif
            for (int i = 0; i < x.info_list.size(); ++i) {
                if (x.info_list[i] < y.info_list[i]) return -1;
                if (x.info_list[i] > y.info_list[i]) return 1;
            }
            return 0;
        }
    };
}

EdgeGroup::EdgeGroup(): node(nullptr) {}

void EdgeGroup::init(FTANode *_node) {
    node = _node;
    typedef std::pair<EdgeFeature, FTAEdge*> EdgeInfo;
    std::vector<EdgeInfo*> edge_infos;
    for (auto* edge: node->edge_list) {
        EdgeFeature feature(edge);
        edge_infos.push_back(new EdgeInfo(std::move(feature), edge));
    }
    EdgeFeatureCmp cmp;
    std::sort(edge_infos.begin(), edge_infos.end(), [&](EdgeInfo* x, EdgeInfo* y) {return cmp(x->first, y->first);});

    for (int now = 0, next = 0; now < edge_infos.size(); now = next) {
        next = now + 1;
        while (next < edge_infos.size() && cmp.isEqual(edge_infos[next]->first, edge_infos[next - 1]->first)) ++next;
        FixedVector<FTAEdge*> edges(next - now);
        for (int i = now; i < next; ++i) edges[i - now] = edge_infos[i]->second;
        edge_groups.emplace_back(std::move(edge_infos[now]->first), std::move(edges));
    }
}

void EdgeGroup::print() const {
    LOG(INFO) << "Group";
    for (auto& [feature, edge_list]: edge_groups) {
        LOG(INFO) << feature.toString();
        for (auto* edge: edge_list) {
            LOG(INFO) << "  " << edge->toString();
        }
    }
}

FixedVector<EdgeGroup> fta::merge::util::backward::buildEdgeGroup(FTA *fta) {
    auto n = assignFTAIndex(fta);
    FixedVector<EdgeGroup> group_list(n);
    for (int size = 0; size <= fta->size_limit; ++size) {
        for (auto& [_, storage]: fta->node_map) {
            for (auto* node: storage[size]) {
                group_list[INDEX(node)].init(node);
            }
        }
    }
    return group_list;
}

using fta::merge::util::backward::EdgeStorage;

std::vector<std::pair<PSemantics, EdgeStorage>>
fta::merge::util::backward::mergeEdgeGroup(const FixedVector<EdgeGroup *> &group_list) {
    std::vector<std::pair<PSemantics, EdgeStorage>> result;
#ifdef DEBUG
    for (auto *group: group_list) assert(!group->edge_groups.empty());
#endif

    typedef std::vector<std::pair<EdgeFeature, FixedVector<FTAEdge *>>>::iterator Iterator;
    int n = group_list.size();
    FixedVector<Iterator> pointers(n);
    for (int i = 0; i < group_list.size(); ++i) pointers[i] = group_list[i]->edge_groups.begin();
    EdgeFeatureCmp cmp;

    bool is_finished = false;
    int index = 0;
    int same_count = 0;
    FixedVector<int> sign_list(n);
    for (int i = 0; i < n; ++i) sign_list[i] = 0;
    while (!is_finished) {
        index = 0;
        same_count = 1;
        for (int i = 1; i < n; ++i) {
            auto &sign = (sign_list[i] = cmp.getSign(pointers[index]->first, pointers[i]->first));
            if (sign == 1) {
                sign = 0;
                index = i;
                same_count = 1;
            } else if (sign == 0) ++same_count;
        }
        if (same_count == n) {
            EdgeStorage storage(n);
            for (int i = 0; i < n; ++i) storage[i] = pointers[i]->second.clone();
            result.emplace_back(pointers[0]->first.sem, std::move(storage));
        }
        for (int i = index; i < n; ++i) {
            if (!sign_list[i]) {
                if ((++pointers[i]) == group_list[i]->edge_groups.end()) {
                    is_finished = true;
                    break;
                }
            }
        }
    }
    return result;
}

namespace {
    void
    _initMergedRoots(int index, const FTANodeStorage &storage, FixedVector<FTANode *> &tmp, SplitHolder &holder,
                     FTANodeList &roots) {
        if (index == storage.size()) {
            auto *root = holder.query(tmp.clone());
            roots.push_back(root);
            return;
        }
        for (auto *node: storage[index]) {
            tmp[index] = node;
            _initMergedRoots(index + 1, storage, tmp, holder, roots);
        }
    }
}

FTANodeList fta::merge::util::backward::initRoots(FTA* base, const FTAList &others, SplitHolderList &holder_list) {
    FTANodeList root_list;
    for (auto *base_root: base->root_list) {
        FTANodeStorage storage;
        for (auto &fta: others) {
            auto &root_list = storage.emplace_back();
            for (auto *root: fta->root_list) {
                if (root->symbol == base_root->symbol && root->size == base_root->size) {
                    root_list.push_back(root);
                }
            }
        }
        FixedVector<FTANode *> tmp(others.size());
        _initMergedRoots(0, storage, tmp, holder_list[INDEX(base_root)], root_list);
    }
    return root_list;
}

namespace {
    void _combineEdge(int index, const EdgeStorage& storage, FixedVector<FTAEdge*>& tmp,
                      const FixedVector<SplitHolder*>& holder_list, SmallVector<FixedVector<FTANode*>>& result) {
        if (index == storage.size()) {
            FixedVector<FTANode*> children(holder_list.size());
            for (int child_index = 0; child_index < holder_list.size(); ++child_index) {
                FixedVector<FTANode*> node_list(tmp.size());
                for (int edge_index = 0; edge_index < tmp.size(); ++edge_index) {
                    node_list[edge_index] = tmp[edge_index]->node_list[child_index];
                }
                children[child_index] = holder_list[child_index]->query(std::move(node_list));
            }
            result.append(std::move(children));
            return;
        }
        for (auto* edge: storage[index]) {
            tmp[index - 1] = edge;
            _combineEdge(index + 1, storage, tmp, holder_list, result);
        }
    }

    SmallVector<FixedVector<FTANode*>> _combineEdge(const EdgeStorage& storage, SplitHolderList& holder_list) {
#ifdef DEBUG
        assert(storage.size());
#endif
        SmallVector<FixedVector<FTANode*>> result(config::KDefaultSmallVecSize);
        for (auto* first_edge: storage[0]) {
            FixedVector<SplitHolder*> base_holder_list(first_edge->node_list.size());
            for (int i = 0; i < first_edge->node_list.size(); ++i) {
                base_holder_list[i] = &holder_list[INDEX(first_edge->node_list[i])];
            }
            FixedVector<FTAEdge*> tmp_list(storage.size() - 1);
            _combineEdge(1, storage, tmp_list, base_holder_list, result);
        }
        return result;
    }
}

void fta::merge::util::backwardStep(FTANode *base,
                                    const FixedVector<FixedVector<backward::EdgeGroup>> &edge_holders,
                                    SplitHolderList &holder_list) {
    auto& start = holder_list[INDEX(base)];

    for (auto& [others, merged_node]: start.map) {
        FixedVector<EdgeGroup*> group_list(edge_holders.size());
        group_list[0] = &edge_holders[0][INDEX(base)];
        for (int i = 0; i < others.size(); ++i) {
            auto node_index = INDEX(others[i]);
            group_list[i + 1] = &edge_holders[i + 1][node_index];
        }
        for (auto& [sem, edge_storage]: util::backward::mergeEdgeGroup(group_list)) {
            for (auto& children: _combineEdge(edge_storage, holder_list)) {
                auto* edge = new FTAEdge(sem, std::move(children));
                merged_node->edge_list.push_back(edge);
            }
        }
    }
}

SplitHolderList fta::merge::util::initSplitHolderList(FTA *base, int n) {
    SplitHolderList holder_list(n);
    for (auto& [_, storage]: base->node_map) {
        for (auto& list: storage) {
            for (auto* node: list) {
                holder_list[INDEX(node)].base = node;
            }
        }
    }
    return holder_list;
}

PFTA fta::merge::util::createRawFTA(FTA *base, const SplitHolderList &holder_list, const FTANodeList &root_list,
                                    const std::vector<ExecuteInfo *> &info_list) {
    SymbolNameFTANodeHolder node_holder;
    for (int size = 0; size <= base->size_limit; ++size) {
        for (auto& [name, storage]: base->node_map) {
            auto& node_list = node_holder[name].emplace_back();
            for (auto* node: storage[size]) {
                for (auto& [_, merged_node]: holder_list[INDEX(node)].map) {
                    node_list.push_back(merged_node);
                }
            }
        }
    }
    return std::make_shared<FTA>(base->grammar, base->size_limit, node_holder, root_list, info_list);
}

PFTA fta::merge::mergeFTABackwardMultiWithShare(const FTAList &fta_list, bool is_cut) {
#ifdef DEBUG
    checkShared(fta_list);
#endif

    FixedVector<FixedVector<EdgeGroup>> edge_group_list(fta_list.size());
    for (int i = 0; i < fta_list.size(); ++i) {
        edge_group_list[i] = backward::buildEdgeGroup(fta_list[i].get());
    }
    auto* base = fta_list[0].get();
    int n = edge_group_list[0].size();

    FTAList others;
    for (int i = 1; i < fta_list.size(); ++i) others.push_back(fta_list[i]);

    auto holder_list = util::initSplitHolderList(base, n);
    auto roots = backward::initRoots(base, others, holder_list);

    // Full merge
    for (int size = base->size_limit; size >= 0; --size) {
        for (auto& [_, storage]: base->node_map) {
            for (auto* base_node: storage[size]) {
                backwardStep(base_node, edge_group_list, holder_list);
            }
        }
    }

    auto fta = util::createRawFTA(base, holder_list, roots, mergeInfoList(fta_list));
    if (is_cut) fta->fullyCut();
    return fta;
}

PFTA fta::merge::mergeFTABackwardMulti(const FTAList &fta_list, bool is_cut, int share_num) {
#ifdef DEBUG
    assert(share_num < fta_list.size());
#endif
    auto base = fta_list[0];
    for (int i = 1; i < share_num; ++i) {
        base = mergeFTA(base.get(), fta_list[i].get(), FORWARD);
    }
    FTAList unit_list;
    for (int i = share_num; i < fta_list.size(); ++i) {
        auto merged = mergeFTA(base.get(),fta_list[i].get(), FORWARD);
        unit_list.push_back(merged);
    }
    for (int i = 0; i < unit_list.size(); ++i) {
        LOG(INFO) << "Unit #" << i << ": " << unit_list[i]->getSizeInfo();
    }
    return merge::mergeFTABackwardMultiWithShare(unit_list, is_cut);
}