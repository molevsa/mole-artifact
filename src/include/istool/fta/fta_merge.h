//
// Created by pro on 2024/10/13.
//

#ifndef ISTOOL_FTA_MERGE_H
#define ISTOOL_FTA_MERGE_H

#include "istool/fta/fta.h"
#include "istool/fta/fta_edge_trie.h"
#include <map>

namespace fta::merge {
    PFTA mergeFTAForward(FTA *x, FTA *y, bool is_cut, TimeGuard* guard);
    PFTA mergeFTABackward(FTA *x, FTA *y, bool is_cut, TimeGuard* guard);
    PFTA mergeFTABackwardMultiWithShare(const FTAList& fta_list, bool is_cut);

    // Assume that one fta is shared by all FTAs, as the left-side half
    PFTA mergeFTAForwardMulti(const FTAList& fta_list, bool is_cut);
    PFTA mergeFTABackwardMulti(const FTAList& fta_list, bool is_cut, int share_num);
    PFTA mergeFTACostBasedBidirectional(const FTAList& fta_list, bool is_cut);
    PFTA mergeFTAStagedBidirectional(const FTAList& fta_list, bool is_cut, double alpha);

#define RemoveCopy(type) \
    type(const type&) = delete; \
    type& operator = (const type&) = delete

    namespace util {
        struct SplitHolder {
            std::map<FixedVector<FTANode*>, FTANode*, FixedVectorCmp<FTANode*>> map;
            FTANode *base;

            SplitHolder(FTANode* _base);
            SplitHolder() = default;
            FixedVector<std::pair<FixedVector<FTANode*>, FTANode*>> moveOutInfoList();
            FTANode* query(FixedVector<FTANode*>&& feature);
            RemoveCopy(SplitHolder);
        };
        typedef std::vector<SplitHolder> SplitHolderList;

        PInfo extractSharedInfo(FTANode* node);
        PInfo extractNewInfo(FTANode* node);
        std::vector<ExecuteInfo*> mergeInfoList(const FTAList& fta_list);
        FTANode* createNode(FTANode* base, const FixedVector<FTANode*>& others);
        FTAEdge* createEdge(const PSemantics& semantics, FixedVector<FTANode*>&& children);
        SplitHolderList initSplitHolderList(FTA* base, int n);
        PFTA createRawFTA(FTA* base, const SplitHolderList& holders, const FTANodeList& root_list,
                          const std::vector<ExecuteInfo*>& info_list);

        void forwardStep(FTANode* base, const std::vector<trie::FTAEdgeHolder>& edge_holders,
                         SplitHolderList& info_holder, SplitHolder& result);

#ifdef DEBUG
        void checkShared(const FTAList& fta_list);
#endif

        namespace backward {
            struct EdgeFeature {
                PSemantics sem;
                FixedVector<OutputInfo*> info_list;
                RemoveCopy(EdgeFeature);
                EdgeFeature(EdgeFeature&& feature);
                EdgeFeature(FTAEdge* edge);
                std::string toString() const;
            };

            struct EdgeGroup {
                FTANode *node;
                std::vector<std::pair<EdgeFeature, FixedVector<FTAEdge*>>> edge_groups;
                EdgeGroup();
                RemoveCopy(EdgeGroup);
                void init(FTANode* node);
                void print() const;
            };

            typedef FixedVector<FixedVector<FTAEdge*>> EdgeStorage;

            // will assign node indices as a side effect
            FixedVector<EdgeGroup> buildEdgeGroup(FTA* fta);

            std::vector<std::pair<PSemantics, EdgeStorage>> mergeEdgeGroup(const FixedVector<EdgeGroup *> &group_list);
            FTANodeList initRoots(FTA* base, const FTAList& other, SplitHolderList& holder_list);
        }

        void backwardStep(FTANode* base,
                          const FixedVector<FixedVector<backward::EdgeGroup>>& edge_holders,
                          SplitHolderList& holder_list);
    }
}

#endif //ISTOOL_FTA_MERGE_H
