//
// Created by pro on 2024/9/22.
//

#include "istool/fta/fta_edge_trie.h"
#include <cassert>

using namespace fta;
using fta::trie::TrieNode;

TrieNode::~TrieNode() {
    for (auto& [_, child]: edges) delete child;
}

TrieNode *TrieNode::next(int index) {
    auto it = edges.find(index);
    if (it == edges.end()) return nullptr; else return it->second;
}

TrieNode *TrieNode::nextWithInsert(int index) {
    auto* node = next(index);
    if (!node) return edges[index] = new TrieNode(); else return node;
}

std::string fta::trie::getEdgeFeature(NonTerminal *symbol, Semantics *semantics) {
    return symbol->name + "@" + semantics->getName();
}

namespace {
    void insert(TrieNode* node, FTAEdge* edge, FTANode* source) {
        for (auto* fta_node: edge->node_list) node = node->nextWithInsert(fta_node->getInfo());
        assert(!node->y_node);
        node->y_node = source;
    }
}

#include "glog/logging.h"

trie::FTAEdgeHolder fta::trie::buildEdgeHolder(FTA *fta) {
    FTAEdgeHolder res;
    //LOG(INFO) << "start build";
    auto insert_edge = [&](FTANode* source, FTAEdge* edge) {
        auto feature = getEdgeFeature(source->symbol, edge->semantics.get());
        if (res.find(feature) == res.end()) {
            res[feature] = std::make_shared<TrieNode>();
        }
        insert(res[feature].get(), edge, source);
    };
    for (auto& [_, storage]: fta->node_map) {
        for (auto& list: storage) {
            for (auto* node: list) {
                for (auto* edge: node->edge_list) {
                    insert_edge(node, edge);
                }
            }
        }
    }
    return res;
}