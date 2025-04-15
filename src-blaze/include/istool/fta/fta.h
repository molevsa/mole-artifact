//
// Created by pro on 2024/9/18.
//

#ifndef ISTOOL_FTA_H
#define ISTOOL_FTA_H

#include <variant>

#include "istool/basic/example_space.h"
#include "istool/basic/program.h"
#include "istool/basic/small_vector.h"
#include "istool/basic/specification.h"
#include "istool/basic/time_guard.h"
#include "istool/basic/verifier.h"
#include "istool/fta/fta_value.h"

namespace fta {
    class OutputInfo {
    public:
        virtual DataList getFullOutput() const = 0;
        virtual ~OutputInfo() = default;
    };
    typedef std::shared_ptr<OutputInfo> PInfo;

    class EmptyOutputInfo : public OutputInfo {
    public:
        virtual DataList getFullOutput() const;
        virtual ~EmptyOutputInfo() = default;
    };

    class SingleOutputInfo : public OutputInfo {
    public:
        Data value;
        SingleOutputInfo(const Data &_v);
        virtual DataList getFullOutput() const;
        virtual ~SingleOutputInfo() = default;
    };

    class BinaryOutputInfo : public OutputInfo {
    public:
        PInfo l, r;
        BinaryOutputInfo(const PInfo &_l, const PInfo &_r);
        virtual DataList getFullOutput() const;
        virtual ~BinaryOutputInfo() = default;
    };

    class MultiOutputInfo : public OutputInfo {
    public:
        FixedVector<PInfo> info_list;
        MultiOutputInfo(FixedVector<PInfo> &&_info_list);
        virtual DataList getFullOutput() const;
        virtual ~MultiOutputInfo() = default;
    };

    class FTANode;

    class FTAEdge {
    public:
        PSemantics semantics;
        FixedVector<FTANode *> node_list;
        std::string toString() const;
        FTAEdge(const PSemantics &_sem, FixedVector<FTANode *> &&_node_list);
    };

    class FTANode {
        int extra_info;

    public:
        NonTerminal *symbol;
        PInfo oup_info;
        std::vector<FTAEdge *> edge_list;
        int size;

        FTANode(NonTerminal *_symbol, const PInfo &_info, int _size);
        ~FTANode();
        void setInfo(int info);
        std::string toString() const;
        int getInfo() const;
    };

    typedef std::vector<FTANode *> FTANodeList;
    typedef std::vector<FTANodeList> FTANodeStorage;
    typedef std::unordered_map<std::string, FTANodeStorage> SymbolNameFTANodeHolder;

    class FTA {
    public:
        SymbolNameFTANodeHolder node_map;
        FTANodeList root_list;
        std::vector<ExecuteInfo *> info_list;
        Grammar *grammar;
        int size_limit;
        IOExampleList examples;

        int nodeCount() const;
        int edgeCount() const;
        bool isEmpty() const;
        void cutBackward();
        void cutForward();
        void fullyCut();
        std::string getSizeInfo() const;

        FTA(Grammar *_grammar, int _size_limit,
            const SymbolNameFTANodeHolder &_node_map, const FTANodeList &_root_list,
            const std::vector<ExecuteInfo *> &_info_list,
            const IOExampleList &examples);
        ~FTA();
    };

    typedef std::shared_ptr<FTA> PFTA;
    typedef std::vector<PFTA> FTAList;

    namespace util {
        Data mergeOutput(Semantics *sem, DataList &&inp, ExecuteInfo *info);
        DataList mergeOutputMulti(Semantics *sem, DataStorage &&inp,
                                  const std::vector<ExecuteInfo *> &info_list);
        PProgram extractMinimalProgram(FTA *x);
        PProgram extractRandomMinimalProgram(FTA *x, Env *env);
        PProgram extractMinimalProgramFromNode(FTANode *node);
        void showFTA(FTA *fta);
        int assignFTAIndex(FTA *fta);
        void verifyFTAEdge(FTANode *node, FTAEdge *edge,
                           const std::vector<ExecuteInfo *> &info_list);
        void verifyFTA(FTA *fta);
        bool isEmpty(FTA *x);
        PFTA cloneFTA(FTA *fta);
        std::pair<long long, long long> getTotalSize(FTA *x);
        std::pair<long long, long long> getTotalSize();
        void addExampleUsed(IOExample example);
        int countExampleUsed();
    } // namespace util

    namespace size {
        typedef double FTASize;
        typedef std::vector<FTASize> FTASizeInfo;
        FTASize getFTASize(FTA *fta);
        FTASizeInfo getFTASizeVector(FTA *fta);
        FTASizeInfo getNodeCountVector(FTA *fta);
        FTASizeInfo getCompressVector(FTA *fta, FTA *base);
        std::string sizeInfo2String(const FTASizeInfo &info);
    } // namespace size

    enum MergeType { FORWARD,
                     BACKWARD };

    PFTA buildFTA(Grammar *grammar, Env *env, const IOExample &example,
                  int size_limit, bool is_strictly_equal,
                  std::shared_ptr<value::FTAValueSets> fta_value_sets);
    PFTA rebuildFTA(FTA *x, Env *env, const IOExample &example, int size_limit,
                    bool is_strictly_equal,
                    std::shared_ptr<value::FTAValueSets> fta_value_sets);
    PFTA buildFTAFromExamples(Grammar *grammar, Env *env,
                              const IOExampleList &example_list, int size_limit,
                              bool is_strictly_equal,
                              std::shared_ptr<value::FTAValueSets> fta_value_sets);
    PFTA grammar2FTA(Grammar *grammar, int size_limit, bool is_strictly_equal);

    PFTA mergeFTA(FTA *x, FTA *y, MergeType type, bool is_cut = true,
                  TimeGuard *guard = nullptr);
    PFTA mergeIndependentFTAForward(const FTAList &fta_list, bool is_cut);

    void refineAbstractValue(FTA *x, Env *env, const IOExample &example,
                             std::shared_ptr<value::FTAValueSets> fta_value_sets);

    namespace analysis {
        void analyzeMerge(Grammar *grammar, Env *env, const IOExampleList &example_list,
                          int merge_size);
    }
} // namespace fta

#define INDEX(node) (node->getInfo())

#endif // ISTOOL_FTA_H
