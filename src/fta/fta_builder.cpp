//
// Created by pro on 2024/9/18.
//

#include "istool/fta/fta.h"
#include <unordered_set>
#include <cassert>
#include <queue>
#include "glog/logging.h"
#include "istool/basic/config.h"

using namespace fta;

DataList EmptyOutputInfo::getFullOutput() const {
    return {};
}

DataList SingleOutputInfo::getFullOutput() const {
    return {value};
}

SingleOutputInfo::SingleOutputInfo(const Data &_v): value(_v) {
}

DataList BinaryOutputInfo::getFullOutput() const {
    auto l_res = l->getFullOutput();
    auto r_res = r->getFullOutput();
    for (auto& v: r_res) {
        l_res.push_back(v);
    }
    return l_res;
}

BinaryOutputInfo::BinaryOutputInfo(const PInfo &_l, const PInfo &_r): l(_l), r(_r) {
}

MultiOutputInfo::MultiOutputInfo(FixedVector <PInfo> &&_info_list): info_list(std::move(_info_list)) {
}
DataList MultiOutputInfo::getFullOutput() const {
    DataList result;
    for (int i = 0; i < info_list.size(); ++i) {
        for (auto& v: info_list[i]->getFullOutput()) result.push_back(v);
    }
    return result;
}

std::string FTAEdge::toString() const {
    std::string res = semantics->getName();
    for (auto* child: node_list) res += " " + child->toString();
    return res;
}

FTAEdge::FTAEdge(const PSemantics &_sem, FixedVector<FTANode *> &&_node_list): semantics(_sem), node_list(std::move(_node_list)) {
}

FTA::FTA(Grammar* _grammar, int _size_limit, const SymbolNameFTANodeHolder &_node_map, const FTANodeList &_root_list,
         const std::vector<ExecuteInfo *> &_info_list):
         node_map(_node_map), root_list(_root_list), info_list(_info_list), grammar(_grammar), size_limit(_size_limit) {
}

bool FTA::isEmpty() const {return root_list.empty();}

FTA::~FTA() {
    root_list.clear();
    for (auto& [_, storage]: node_map) {
        for (auto& list: storage) {
            for (auto* node: list) delete node;
        }
    }
}

int FTA::nodeCount() const {
    int res = 0;
    for (auto& [_, storage]: node_map) {
        for (auto& list: storage) res += list.size();
    }
    return res;
}

int FTA::edgeCount() const {
    int res = 0;
    for (auto& [_, storage]: node_map) {
        for (auto& list: storage) {
            for (auto* node: list) res += node->edge_list.size();
        }
    }
    return res;
}

FTANode::FTANode(NonTerminal *_symbol, const PInfo &_info, int _size):
    symbol(_symbol), oup_info(_info), size(_size) {
}

fta::FTANode::~FTANode() {
    for (auto* edge: edge_list) delete edge;
}

void FTANode::setInfo(int info) {extra_info = info;}

int FTANode::getInfo() const {return extra_info;}

std::string FTANode::toString() const {
    return symbol->name + "@" + std::to_string(size) + "@" + data::dataList2String(oup_info->getFullOutput());
}

namespace {
    void _getAllSizeScheme(int pos, int rem, const std::vector<std::vector<int>> &pool, std::vector<int> &tmp,
                           std::vector<std::vector<int>> &res) {
        if (pos == pool.size()) {
            if (rem == 0) res.push_back(tmp);
            return;
        }
        for (auto size: pool[pos]) {
            if (size > rem) continue;
            tmp.push_back(size);
            _getAllSizeScheme(pos + 1, rem - size, pool, tmp, res);
            tmp.pop_back();
        }
    }

    std::vector<std::vector<int>> getAllSizeScheme(int size, const std::vector<std::vector<int>> &pool) {
        std::vector<int> tmp;
        std::vector<std::vector<int>> res;
        _getAllSizeScheme(0, size, pool, tmp, res);
        return res;
    }

    template<typename T>
    void _cartesianProduct(int index, FixedVector<T> &tmp, const std::vector<std::vector<T>> &storage, std::vector<FixedVector<T>> &res) {
        if (index == tmp.size()) {
            res.emplace_back(tmp.clone());
            return;
        }
        for (auto &choice: storage[index]) {
            tmp[index] = choice;
            _cartesianProduct(index + 1, tmp, storage, res);
        }
    }

    template<typename T>
    std::vector<FixedVector<T>> cartesianProduct(const std::vector<std::vector<T>> &storage) {
        std::vector<FixedVector<T>> res;
        FixedVector<T> tmp(storage.size());
        _cartesianProduct(0, tmp, storage, res);
        return res;
    }

    void _resetFTAInfo(FTA* fta, int info) {
        for (const auto& [_, storage]: fta->node_map) {
            for (const auto& list: storage) {
                for (auto* node: list) node->setInfo(info);
            }
        }
    }

    template<typename T>
    std::string _vec2String(const std::vector<T>& xs) {
        std::string res = "[";
        for (int i = 0; i < xs.size(); ++i) {
            if (i) res += ", "; res += std::to_string(xs[i]);
        }
        return res + "]";
    }

    Data _getSingleOutput(OutputInfo* info) {
        auto* single_info = dynamic_cast<SingleOutputInfo*>(info);
#ifdef DEBUF
        assert(single_info);
#endif
        return single_info->value;
    }
}

PFTA fta::buildFTA(Grammar *grammar, Env *env, const IOExample &example, int size_limit, bool is_strictly_equal) {
    auto* info = env->getExecuteInfoBuilder()->buildInfo(example.first, {});

    grammar->indexSymbol();
    std::vector<FTANodeStorage> node_holder(grammar->symbol_list.size());
    for (auto& storage: node_holder) storage.resize(size_limit + 1);

    for (int size = 1; size <= size_limit; ++size) {
        for (auto* symbol: grammar->symbol_list) {
            auto& current_holder = node_holder[symbol->id][size];
            std::unordered_map<std::string, FTANode*> output_node_map;

            for (auto* rule: symbol->rule_list) {
                auto* cr = dynamic_cast<ConcreteRule*>(rule); assert(cr);

                std::vector<std::vector<int>> size_pool;
                for (auto& nt: cr->param_list) {
                    auto& node_storage = node_holder[nt->id];
                    std::vector<int> possible_size;
                    for (int i = 0; i < size; ++i) {
                        if (!node_storage[i].empty()) {
                            possible_size.push_back(i);
                        }
                    }
                    size_pool.push_back(possible_size);
                }

                for (auto& size_scheme: getAllSizeScheme(size - 1, size_pool)) {
                    FTANodeStorage sub_nodes;
                    for (int i = 0; i < size_scheme.size(); ++i) {
                        sub_nodes.push_back(node_holder[rule->param_list[i]->id][size_scheme[i]]);
                    }
                    for (auto& combination: cartesianProduct(sub_nodes)) {
                        DataList sub_inps(combination.size());
                        for (int i = 0; i < combination.size(); ++i) {
                            sub_inps[i] = _getSingleOutput(combination[i]->oup_info.get());
                        }
                        // LOG(INFO) << "try merge " << cr->semantics->getName() << " " << data::dataList2String(sub_inps) << " " << data::dataList2String(info->param_value);
                        auto oup = util::mergeOutput(cr->semantics.get(), std::move(sub_inps), info);
                        auto feature = oup.toString();
                        auto it = output_node_map.find(feature);
                        FTANode* target = nullptr;
                        if (it == output_node_map.end()) {
                            target = new FTANode(symbol, std::make_shared<SingleOutputInfo>(oup), size);
                            current_holder.push_back(target);
                            output_node_map[feature] = target;
                        } else {
                            target = it->second;
                        }
                        auto* edge = new FTAEdge(cr->semantics, std::move(combination));
                        target->edge_list.push_back(edge);
                    }
                }
            }
        }
    }

    FTANodeList root_list;
    for (int size = (is_strictly_equal ? size_limit : 0); size <= size_limit; ++size) {
        auto root_index = grammar->start->id;
        for (auto* node: node_holder[root_index][size]) {
            if (_getSingleOutput(node->oup_info.get()) == example.second) {
                root_list.push_back(node);
            }
        }
    }

    SymbolNameFTANodeHolder name_holder;
    for (auto* symbol: grammar->symbol_list) {
        name_holder[symbol->name] = std::move(node_holder[symbol->id]);

        int node_num = 0;
        for (auto& list: name_holder[symbol->name]) node_num += list.size();
    }

    auto raw_fta = std::make_shared<FTA>(grammar, size_limit, name_holder, root_list, (std::vector<ExecuteInfo*>){info});
    raw_fta->cutBackward();
    return raw_fta;
}

PFTA fta::buildFTAFromExamples(Grammar *grammar, Env *env, const IOExampleList &example_list, int size_limit,
                               bool is_strictly_equal) {
    assert(!example_list.empty());
    auto fta = buildFTA(grammar, env, example_list[0], size_limit, is_strictly_equal);
    for (int i = 1; i < example_list.size() && !util::isEmpty(fta.get()); ++i) {
        auto next_single = buildFTA(grammar, env, example_list[i], size_limit, is_strictly_equal);
        if (util::isEmpty(next_single.get())) return nullptr;
        fta = mergeFTA(fta.get(), next_single.get(), FORWARD);
    }
    if (util::isEmpty(fta.get())) return nullptr;
    return fta;
}

std::string FTA::getSizeInfo() const {
    return std::to_string(nodeCount()) + " nodes and " + std::to_string(edgeCount()) + " edges";
}

namespace {
    const int VISITED = 1, UNVISITED = 0;

    bool _isEdgeVisited(FTAEdge* edge) {
        for (auto* node: edge->node_list) {
            if (node->getInfo() == UNVISITED) return false;
        }
        return true;
    }

    void _clearNodeMap(SymbolNameFTANodeHolder& node_map) {
        for (auto& [_, node_storage]: node_map) {
            for (auto& node_list: node_storage) {
                int index = 0;
                for (auto* node: node_list) {
                    if (node->getInfo() == UNVISITED) {
                        delete node;
                    } else {
                        node_list[index++] = node;
                    }
                }
                node_list.resize(index);
            }
        }
    }
}

void FTA::cutForward() {
    _resetFTAInfo(this, UNVISITED);

    for (int size = 1; size <= this->size_limit; ++size) {
        for (auto& [_, storage]: node_map) {
            for (auto* node: storage[size]) {
                int num = 0;
                for (auto* edge: node->edge_list) {
                    if (_isEdgeVisited(edge)) {
                        node->edge_list[num++] = edge;
                    } else {
                        delete edge;
                    }
                }
                node->edge_list.resize(num);
                if (num) node->setInfo(VISITED);
            }
        }
    }
    int num = 0;
    for (auto* root: root_list) {
        if (root->getInfo() == VISITED) root_list[num++] = root;
    }
    root_list.resize(num);
    _clearNodeMap(node_map);
}

void FTA::cutBackward() {
    std::queue<FTANode*> Q;
    _resetFTAInfo(this, UNVISITED);

    auto insert = [&](FTANode* node) {
        if (node->getInfo() == UNVISITED) {
            node->setInfo(VISITED);
            Q.push(node);
        }
    };

    for (auto* root: root_list) insert(root);
    while (!Q.empty()) {
        auto* node = Q.front(); Q.pop();
        for (auto* edge: node->edge_list) {
            for (auto* child: edge->node_list) {
                insert(child);
            }
        }
    }
    _clearNodeMap(node_map);
}

void FTA::fullyCut() {
    if (!isEmpty()) cutForward();
    if (!isEmpty()) cutBackward();
}

namespace {
    PProgram _extractProgramFrom(FTANode* node) {
        auto* edge = node->edge_list[0];
        ProgramList sub_programs;
        for (auto* child: edge->node_list) {
            sub_programs.push_back(_extractProgramFrom(child));
        }
        return std::make_shared<Program>(edge->semantics, sub_programs);
    }
};

PProgram util::extractMinimalProgram(FTA *x) {
    FTANode* start = nullptr;
    int best_size = 1e9;
    for (auto* node: x->root_list) {
        if (node->size < best_size) {
            best_size = node->size; start = node;
        }
    }
    return _extractProgramFrom(start);
}

namespace {
    PProgram _extractRandomFrom(FTANode* node, Env* env) {
        auto dist = std::uniform_int_distribution<int>(0, int(node->edge_list.size()) - 1);
        auto* edge = node->edge_list[dist(env->random_engine)];
        ProgramList sub_programs;
        for (auto* child: edge->node_list) {
            sub_programs.push_back(_extractRandomFrom(child, env));
        }
        return std::make_shared<Program>(edge->semantics, sub_programs);
    }
};

PProgram util::extractRandomMinimalProgram(FTA *x, Env* env) {
    FTANode* start = nullptr;
    int best_size = 1e9;
    for (auto* node: x->root_list) {
        if (node->size < best_size) {
            best_size = node->size; start = node;
        }
    }
    return _extractRandomFrom(start, env);
}

PFTA fta::grammar2FTA(Grammar *grammar, int size_limit, bool is_strictly_equal) {
    grammar->indexSymbol();
    FTANodeStorage node_storage(grammar->symbol_list.size(), FTANodeList(size_limit + 1, nullptr));

    for (int size = 1; size <= size_limit; ++size) {
        for (auto* symbol: grammar->symbol_list) {
            std::vector<FTAEdge*> edge_list;
            for (auto* rule: symbol->rule_list) {
                auto* cr = dynamic_cast<ConcreteRule*>(rule);
                assert(cr);
                std::vector<std::vector<int>> choices;
                for (auto* param: cr->param_list) {
                    std::vector<int> choice_list;
                    for (int i = 0; i < size; ++i) {
                        if (node_storage[param->id][i]) choice_list.push_back(i);
                    }
                    choices.push_back(choice_list);
                }

                for (auto scheme: getAllSizeScheme(size - 1, choices)) {
                    FixedVector<FTANode*> children(scheme.size());
                    for (int i = 0; i < scheme.size(); ++i) {
                        auto* param_symbol = cr->param_list[i];
                        children[i] = node_storage[param_symbol->id][scheme[i]];
                    }
                    edge_list.push_back(new FTAEdge(cr->semantics, std::move(children)));
                }
            }
            if (!edge_list.empty()) {
                auto* new_node = new FTANode(symbol, std::make_shared<EmptyOutputInfo>(), size);
                new_node->edge_list = edge_list;
                node_storage[symbol->id][size] = new_node;
            }
        }
    }

    SymbolNameFTANodeHolder holder;
    for (auto* symbol: grammar->symbol_list) {
        FTANodeStorage storage(size_limit + 1);
        for (int i = 0; i <= size_limit; ++i) {
            if (node_storage[symbol->id][i]) {
                storage[i].push_back(node_storage[symbol->id][i]);
            }
        }
        holder[symbol->name] = storage;
    }

    FTANodeList root_list;
    for (int size = (is_strictly_equal ? size_limit : 0); size <= size_limit; ++size) {
        if (node_storage[grammar->start->id][size]) {
            root_list.push_back(node_storage[grammar->start->id][size]);
        }
    }

    auto raw_fta = std::make_shared<FTA>(grammar, size_limit, holder, root_list, std::vector<ExecuteInfo*>());
    raw_fta->cutBackward();
    return raw_fta;
}