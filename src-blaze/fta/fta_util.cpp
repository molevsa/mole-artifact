//
// Created by pro on 2024/9/21.
//

#include <cassert>

#include "istool/fta/fta.h"

using namespace fta;

Data util::mergeOutput(Semantics *sem, DataList &&inp, ExecuteInfo *info) {
    auto *ns = dynamic_cast<NormalSemantics *>(sem);
    if (ns) {
        return ns->run(std::move(inp), info);
    }
    ProgramList programs;
    for (auto &value : inp) {
        programs.push_back(program::buildConst(value));
    }
    return sem->run(programs, info);
}

DataList util::mergeOutputMulti(Semantics *sem, DataStorage &&all_inp,
                                const std::vector<ExecuteInfo *> &info_list) {
    DataList res(info_list.size());
    for (int i = 0; i < info_list.size(); ++i) {
        DataList inp(all_inp.size());
        for (int j = 0; j < all_inp.size(); ++j) {
            inp[j] = all_inp[j][i];
        }
        res[i] = mergeOutput(sem, std::move(inp), info_list[i]);
    }
    return res;
}

int util::assignFTAIndex(FTA *fta) {
    int index = 0;
    for (int size = 0; size <= fta->size_limit; ++size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                node->setInfo(index++);
            }
        }
    }
    return index;
}

#include "glog/logging.h"

namespace {
void _printNode(FTANode *node) {
    auto oup_list = node->oup_info->getFullOutput();
    LOG(INFO) << "Node#" << node->getInfo() << " " << node->symbol->name << "@"
              << node->size << " " << data::dataList2String(oup_list);
    for (auto *edge : node->edge_list) {
        auto edge_string = edge->semantics->getName();
        for (auto *child : edge->node_list)
            edge_string += " (" + std::to_string(child->getInfo()) + ")";
        LOG(INFO) << "  " << edge_string;
    }
}
}  // namespace

void util::showFTA(FTA *fta) {
    int total_nodes = assignFTAIndex(fta);
    std::string info = "Roots [";
    for (int i = 0; i < fta->root_list.size(); ++i) {
        if (i) info += ", ";
        info += std::to_string(fta->root_list[i]->getInfo());
    }
    LOG(INFO) << info << "] with " << total_nodes << " nodes" << std::endl;

    for (int size = fta->size_limit; size >= 0; --size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                _printNode(node);
            }
        }
    }
}

void util::verifyFTAEdge(FTANode *node, FTAEdge *edge,
                         const std::vector<ExecuteInfo *> &info_list) {
    DataStorage child_outputs;
    for (auto *child : edge->node_list)
        child_outputs.push_back(child->oup_info->getFullOutput());
    auto expected_output = util::mergeOutputMulti(
        edge->semantics.get(), std::move(child_outputs), info_list);
    auto oup_list = node->oup_info->getFullOutput();
    if (expected_output.size() != oup_list.size()) {
        LOG(FATAL) << "Fail when verifying " << node->toString() << " -> "
                   << edge->toString() << ", example num not match";
    }
    for (int i = 0; i < expected_output.size(); i++) {
        if (!(oup_list[i].get()->isSubValue(expected_output[i].get()))) {
            LOG(FATAL) << "Fail when verifying " << node->toString() << " -> "
                       << edge->toString() << ", get unexpected output "
                       << data::dataList2String(expected_output);
        }
    }
}

bool util::isEmpty(FTA *fta) { return !fta || fta->isEmpty(); }

void util::verifyFTA(FTA *fta) {
    for (auto &[_, storage] : fta->node_map) {
        for (auto &list : storage) {
            for (auto *node : list) {
                for (auto *edge : node->edge_list) {
                    verifyFTAEdge(node, edge, fta->info_list);
                }
            }
        }
    }
}

using fta::size::FTASize;
using fta::size::FTASizeInfo;

namespace {
FTASizeInfo prepareNodeSize(FTA *fta) {
    int n = util::assignFTAIndex(fta);
    FTASizeInfo counter(n, 0.0);
    for (int size = 1; size <= fta->size_limit; ++size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                auto &node_size = counter[node->getInfo()];
                for (auto *edge : node->edge_list) {
                    auto edge_size = 1.0;
                    for (auto *child : edge->node_list) {
                        edge_size *= counter[child->getInfo()];
                    }
                    node_size += edge_size;
                }
            }
        }
    }
    return counter;
}
}  // namespace

FTASize size::getFTASize(FTA *fta) {
    auto counter = prepareNodeSize(fta);
    FTASize result = 0.0;
    for (auto *root : fta->root_list) {
        result += counter[root->getInfo()];
    }
    return result;
}

FTASizeInfo size::getFTASizeVector(FTA *fta) {
    auto counter = prepareNodeSize(fta);

    FTASizeInfo res(fta->size_limit + 1, 0.0);
    for (int size = 1; size <= fta->size_limit; ++size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                res[size] += counter[node->getInfo()];
            }
        }
    }
    return res;
}

FTASizeInfo size::getCompressVector(FTA *fta, FTA *base) {
    assert(fta->size_limit == base->size_limit &&
           fta->grammar == base->grammar);

    auto size_info = getFTASizeVector(fta);
    auto ref_info = getFTASizeVector(base);

    FTASizeInfo res;
    for (int i = 0; i < ref_info.size(); ++i) {
        if (ref_info[i] == 0) {
            assert(size_info[i] == 0);
            res.push_back(NAN);
        } else {
            res.push_back(size_info[i] / ref_info[i]);
        }
    }
    return res;
}

std::string size::sizeInfo2String(const FTASizeInfo &info) {
    std::string res = "[";
    for (int i = 0; i < info.size(); ++i) {
        if (i) res += ", ";
        if (fabs(info[i] - int(info[i])) < 1e-8 && info[i] >= 0 &&
            info[i] < 1e9) {
            res += std::to_string(int(info[i]));
        } else {
            res += std::to_string(info[i]);
        }
    }
    return res + "]";
}

FTASizeInfo size::getNodeCountVector(FTA *fta) {
    FTASizeInfo res(fta->size_limit + 1, 0.0);
    for (auto &[_, storage] : fta->node_map) {
        for (int i = 0; i <= fta->size_limit; ++i) {
            for (auto *node : storage[i]) {
                res[i] += node->edge_list.size();
            }
        }
    }
    return res;
}

PFTA util::cloneFTA(FTA *fta) {
    int n = assignFTAIndex(fta);
    FTANodeList new_node_list(n, nullptr);
    for (int size = 1; size <= fta->size_limit; ++size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                new_node_list[node->getInfo()] =
                    new FTANode(node->symbol, node->oup_info, node->size);
            }
        }
    }
    auto get_new_node = [&](FTANode *node) {
        return new_node_list[node->getInfo()];
    };

    for (int size = 1; size <= fta->size_limit; ++size) {
        for (auto &[_, storage] : fta->node_map) {
            for (auto *node : storage[size]) {
                auto *new_node = get_new_node(node);
                for (auto *edge : node->edge_list) {
                    FixedVector<FTANode *> new_children(edge->node_list.size());
                    for (int i = 0; i < edge->node_list.size(); ++i) {
                        new_children[i] = get_new_node(edge->node_list[i]);
                    }
                    auto *new_edge =
                        new FTAEdge(edge->semantics, std::move(new_children));
                    new_node->edge_list.push_back(new_edge);
                }
            }
        }
    }

    SymbolNameFTANodeHolder holder;
    for (auto &[name, storage] : fta->node_map) {
        FTANodeStorage new_storage;
        for (auto &list : storage) {
            FTANodeList new_list;
            for (auto *node : list) new_list.push_back(get_new_node(node));
            new_storage.push_back(new_list);
        }
        holder[name] = new_storage;
    }

    FTANodeList root_list;
    for (auto *root : fta->root_list) root_list.push_back(get_new_node(root));
    return std::make_shared<FTA>(fta->grammar, fta->size_limit, holder,
                                 root_list, fta->info_list, fta->examples);
}