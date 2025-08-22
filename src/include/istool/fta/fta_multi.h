//
// Created by pro on 2024/10/6.
//

#ifndef ISTOOL_FTA_MULTI_H
#define ISTOOL_FTA_MULTI_H

#include "fta.h"
#include "istool/basic/time_guard.h"

namespace fta {
enum MultiFoldStage { PRE, MAIN, SINGLE_MERGE };

struct FoldInfo {
    Env* env;
    PFTA fta;
    int size, example_num;
    size::FTASize compress_rate, ref_size;
    PProgram candidate;
    FoldInfo() = default;
    FoldInfo(const PFTA& _fta, size::FTASize _ref_size, Env* _env);
    void updateInfo();
    bool addExample(Env* env, const IOExample& example, MergeType type);
};

class MultiMergeScheduler {
   public:
    virtual void start(FTA* fta) = 0;
    virtual void startStage(MultiFoldStage stage) = 0;
    virtual void endStage(MultiFoldStage stage) = 0;
    virtual bool isTerminate(const std::vector<FoldInfo>& info) = 0;
    virtual ~MultiMergeScheduler() = default;
};

class SizeExpectedScheduler : public MultiMergeScheduler {
   public:
    size::FTASize expected_size, ref_size, alpha;
    int example_limit;
    SizeExpectedScheduler(size::FTASize _alpha, int _example_limit = 1e9)
        : alpha(_alpha), example_limit(_example_limit) {}
    virtual void start(FTA* fta);
    virtual void startStage(MultiFoldStage stage) {};
    virtual void endStage(MultiFoldStage stage) {};
    virtual bool isTerminate(const std::vector<FoldInfo>& info);
    virtual ~SizeExpectedScheduler() = default;
};

class TimeAdaptiveScheduler : public MultiMergeScheduler {
   public:
    int example_num;
    TimeRecorder recorder;

    TimeAdaptiveScheduler(int fold_num) : example_num(fold_num) {}
    virtual void start(FTA* fta);
    virtual void startStage(MultiFoldStage stage);
    virtual void endStage(MultiFoldStage stage);
    virtual bool isTerminate(const std::vector<FoldInfo>& info);
    virtual ~TimeAdaptiveScheduler() = default;
};

class TimeLimitScheduler : public MultiMergeScheduler {
   public:
    double time_limit;
    TimeGuard guard;
    bool is_exceed;
    TimeLimitScheduler(double _time_limit)
        : time_limit(_time_limit), guard(_time_limit), is_exceed(false) {}
    virtual void start(FTA* fta) {};
    virtual void startStage(MultiFoldStage stage);
    virtual void endStage(MultiFoldStage stage);
    virtual bool isTerminate(const std::vector<FoldInfo>& info);
    virtual ~TimeLimitScheduler() = default;
};

class SimpleScheduler : public MultiMergeScheduler {
   public:
    int merge_count;
    SimpleScheduler(int _merge_count) : merge_count(_merge_count) {}
    virtual void start(FTA* fta) {};
    virtual void startStage(MultiFoldStage stage);
    virtual void endStage(MultiFoldStage stage);
    virtual bool isTerminate(const std::vector<FoldInfo>& info);
    virtual ~SimpleScheduler() = default;
};

namespace verify {
bool verifyAfterExcludingExamples(const PProgram& program, Specification* spec,
                                  IOExample& counter_example,
                                  const IOExampleList& used_examples);
bool verifyMultiAfterExcludingExamples(const ProgramList& program,
                                       Specification* spec,
                                       IOExample& counter_example,
                                       const IOExampleList& used_examples);
}  // namespace verify

namespace kfold {
namespace variants {
FTAList adaptiveKFold(Specification* spec, int fold_num,
                      MultiMergeScheduler* scheduler, MergeType type, int size);
FTAList adaptiveKFoldMulti(Specification* spec, int fold_num,
                           MultiMergeScheduler* scheduler, int size);
FTAList prepareSharedUnitsMulti(Specification* spec,
                                const IOExampleList& examples, int shared_num,
                                int size);
FTAList prepareSharedUnits(Specification* spec, const IOExampleList& examples,
                           int shared_num, int size);
}  // namespace variants
FTAList prepareUnits(Specification* spec, const IOExampleList& examples,
                     int shared_num, int size);
FTAList kFold(Specification* spec, int fold_num, MultiMergeScheduler* scheduler,
              MergeType type, int size);
}  // namespace kfold

namespace synthesis {
PProgram rawCEGIS(Specification* spec, MergeType type, int start_size = 1);
PFTA rawMerge(Specification* spec, const IOExampleList& example_list,
              MergeType type, int size);
PFTA allMerge(Specification* spec, const IOExampleList& example_list,
              MergeType type, int size);
PFTA multiMerge(Specification* spec, const IOExampleList& example_list,
                int fold_num, MergeType type, int size);
PFTA rawMergeFast(Specification* spec, const IOExampleList& example_list,
                  MergeType type, int size);
PProgram kFoldCEGIS(Specification* spec, int fold_num,
                    MultiMergeScheduler* scheduler, MergeType type,
                    int start_size = 1);
PFTA kFoldMerge(Specification* spec, const IOExampleList& example_list,
                int fold_num, MultiMergeScheduler* scheduler, MergeType type,
                int size);
PFTA kFoldMergeCart(Specification* spec, const IOExampleList& example_list,
                    int fold_num, MultiMergeScheduler* scheduler,
                    MergeType type, int size);
namespace stun {
PFTA simpleMerge(Specification* spec, const IOExampleList& example_list,
                 int size, int fold_num);
ProgramList rawMakeIfTerms(Specification* spec,
                           const IOExampleList& example_list, MergeType type);
ProgramList kFoldMakeIfTerms(Specification* spec,
                             const IOExampleList& example_list);
PProgram rawMakeProgram(Specification* spec, const IOExampleList& example_list,
                        MergeType type, ProgramList if_terms);
PProgram kFoldMakeProgram(Specification* spec,
                          const IOExampleList& example_list,
                          ProgramList if_terms);
PProgram rawMergeSTUN(Specification* spec, const IOExampleList& example_list,
                      MergeType type);
PProgram kFoldMergeSTUN(Specification* spec, const IOExampleList& example_list);
PProgram kFoldCEGISSTUN(Specification* spec, IOExampleList& examples,
                        Verifier* verifier);
}  // namespace stun
struct BidConfig {
    int init_example_num;
    int shared_num;
    double alpha;
    BidConfig(int _init, int _shared_num, double _alpha);
};
PProgram bidCEGIS(Specification* spec, BidConfig config);
PFTA bidMerge(Specification* spec, const IOExampleList& example_list,
              BidConfig config, int size_limit);
}  // namespace synthesis
}  // namespace fta

#endif  // ISTOOL_FTA_MULTI_H
