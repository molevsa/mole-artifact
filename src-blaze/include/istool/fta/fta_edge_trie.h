//
// Created by pro on 2024/9/22.
//

#ifndef ISTOOL_FTA_EDGE_TRIE_H
#define ISTOOL_FTA_EDGE_TRIE_H

#include <unordered_map>
#include <memory>
#include "fta.h"

namespace fta::trie {
    struct TrieNode {
        std::unordered_map<int, TrieNode*> edges;
        FTANode* y_node = nullptr;

        TrieNode() = default;
        ~TrieNode();

        TrieNode* next(int index);
        TrieNode* nextWithInsert(int index);
    };

    typedef std::shared_ptr<TrieNode> PTrieNode;
    typedef std::unordered_map<std::string, PTrieNode> FTAEdgeHolder;

    std::string getEdgeFeature(NonTerminal* symbol, Semantics* semantics);
    FTAEdgeHolder buildEdgeHolder(FTA* fta);
}

#endif //ISTOOL_FTA_EDGE_TRIE_H
