#include "istool/sygus/parser/parser.h"
#include "istool/basic/config.h"
#include <string>
#include <iostream>
#include <cstdio>
#include "glog/logging.h"
#include <cassert>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <stdarg.h>
#include "istool/ext/z3/z3_verifier.h"
#include "istool/sygus/theory/basic/clia/clia.h"

// Declarations of data structures related to an FTA. You can change them on your demand.


//#define TEST
class FTANode;
void printf2(const char* format, ...){
    #ifdef TEST
    va_list arglist;
    va_start( arglist, format );
    vprintf( format, arglist );
    va_end( arglist );
    #endif
}
class FTAEdge {
public:
    Rule* rule; // The corresponding grammar rule.
    std::vector<FTANode*> node_list;
    FTAEdge(Rule* _rule, const std::vector<FTANode*> _node_list): rule(_rule), node_list(_node_list){};
};
class FTANode {
public:
    NonTerminal* symbol; // The corresponding non-terminal.
    IOExampleList oup_list; // The corresponding outputs of this node on all considered examples.
    std::string all_oup;
    PProgram example_program;
    std::vector<FTAEdge*> edge_list;
    int size;
    FTANode(NonTerminal* _symbol, const IOExampleList& _oup, int _size): symbol(_symbol), oup_list(_oup), size(_size){};
    ~FTANode();
    
    std::string NodeVal(){
        return oup_list[0].second.get()->toString();
    }
};
/*bool equal(FTANode * a, FTANode * b){
    if(a->symbol != b->symbol) return false;
    if(a->oup_list.size() != b->oup_list.size())return false;
    for(int i = 0; i < a->oup_list.size(); i++){
        if(!(a->oup_list[i].second == b->oup_list[i].second)) return false;
    }
    return true;
}*/
int cmp(const PProgram & a,const PProgram & b){
    if(a->size() < b->size()) return -1;
    else if(a->size() > b->size()) return 1;
    if(a->semantics < b->semantics) return -1;
    else if(a->semantics > b->semantics) return 1;
    int asize = a->sub_list.size();
    int bsize = b->sub_list.size();
    for(int i = 0; i < std::min(asize, bsize); i++){
        if(cmp(a->sub_list[i], b->sub_list[i])) return cmp(a->sub_list[i], b->sub_list[i]);
    }
    return 0;
}
bool operator < (const PProgram & a,const PProgram & b){
    return cmp(a,b) == -1;
}
FTANode::~FTANode() {
    for (auto* edge: edge_list) delete edge;
}

typedef std::vector<FTANode*> NodeList;
typedef std::vector<NodeList> NodeStorage;

class FTA {
public:
    FTANode * root;
    std::vector<NodeStorage> node_list; // All nodes in the FTA.
    std::vector<std::vector<std::unordered_map<std::string, FTANode *>>> val_pos;
    int size;
    FTA(int symbol_cnt):
        root(nullptr), node_list(symbol_cnt), size(0) {
    }
    int getSize() {
        int total_size = 0;
        for (auto& nodes: node_list) {
            for (auto& sub_list: nodes) {
                total_size += sub_list.size();
            }
        }
        return total_size;
    }
    FTANode * insert_and_merge(FTANode * new_node){
        if(val_pos[new_node->symbol->id][new_node->size].count(new_node->all_oup)){
            auto a = val_pos[new_node->symbol->id][new_node->size][new_node->all_oup];
            a->example_program = std::min(a->example_program, new_node->example_program);
            for(auto x: new_node->edge_list){
                a->edge_list.push_back(x);
            }
            new_node->edge_list.clear();
            return a;
            }
        node_list[new_node->symbol->id][new_node->size].push_back(new_node);
        val_pos[new_node->symbol->id][new_node->size][new_node->all_oup] = new_node;
        return new_node;
    }
    ~FTA() {
        for(auto & symbol: node_list)
            for(auto & list: symbol) 
                for(auto node: list) delete node;
    }
};
void _getAllSizeScheme(int pos, int rem, const std::vector<std::vector<int>>& pool, std::vector<int>& tmp, std::vector<std::vector<int>>& res);

// Get all possible schemes to distribute @size to all parameters.
// @pool[i] is the list of possible sizes of the i-th parameter.
std::vector<std::vector<int>> getAllSizeScheme(int size, const std::vector<std::vector<int>>& pool) {
    std::vector<int> tmp;
    std::vector<std::vector<int>> res;
    _getAllSizeScheme(0, size, pool, tmp, res);
    return res;
}

void _getAllSizeScheme(int pos, int rem, const std::vector<std::vector<int>>& pool, std::vector<int>& tmp, std::vector<std::vector<int>>& res) {
    if (pos == pool.size()) {
        if (rem == 0) res.push_back(tmp); return;
    }
    for (auto size: pool[pos]) {
        if (size > rem) continue;
        tmp.push_back(size);
        _getAllSizeScheme(pos + 1, rem - size, pool, tmp, res);
        tmp.pop_back();
    }
}
// make FTA nodes of a given size from sub programs
void constructFTANodes(Rule* rule, 
    NonTerminal* symbol,
    NodeStorage & nodes,
    NodeList & sub_list,
    ProgramList & data_list,
    int pos,
    NodeList & ret_list,
    Env * env,
    const IOExample & example,
    int size
){
    //printf2("now make size %d\n" ,size);
    
    //printf2("example %s\n",example.first[0].get()->toString().c_str());
    if (pos == rule->param_list.size()) {
        auto example_program = rule->buildProgram(data_list);
        auto res = env->run(example_program.get(), example.first);
        FTAEdge * new_edge = new FTAEdge(rule, sub_list);
        FTANode * new_node = new FTANode(symbol, IOExampleList(), size);
        new_node->oup_list.push_back(std::make_pair(example.first,res));
        new_node->all_oup += res.toString();
        new_node->example_program = example_program;
        new_node->edge_list.push_back(new_edge);
        
        ret_list.push_back(new_node);
        return;
    }
    for (auto sub_node: nodes[pos]) {
        data_list.push_back(sub_node->example_program);
        sub_list.push_back(sub_node);
        constructFTANodes(rule, symbol, nodes, sub_list, data_list, pos + 1, ret_list, env, example, size);
        sub_list.pop_back();
        data_list.pop_back();
    }
}
NodeList constructNodes(Rule * rule, NonTerminal* symbol, NodeStorage& nodes, Env * env, const IOExample & example, int size){
    NodeList sub_list, ret_list;
    ProgramList data_list;
    constructFTANodes(rule, symbol, nodes, sub_list, data_list, 0, ret_list, env, example, size);
    return ret_list;
}
/**
 * Construct an FTA for grammar @grammar and input-output example @example.
 * @limit is a threshold limiting the FTA to finite. It can be the depth limit of the FTA or an upper bound of the
 * program size. You can choose a meaning convenient for your implementation.
 */

void printFTA(FTA* fta, Grammar* grammar){
    printf2("FTA PRINT \n");
    for(auto symbol: grammar->symbol_list){
        for(int s = 0; s <= fta->size; s++){
            printf2("symbol %d size %d num %d \n", symbol->id, s, fta->node_list[symbol->id][s].size());
            for(auto node:fta->node_list[symbol->id][s])printf2("node size %d min prog %s\n", s, node->example_program->toString().c_str());
        }
    }
}
// make single example fta with max size = limit
FTA* constructFTA(Grammar* grammar, const IOExample& example, Env* env, int limit, bool cut = true) {
    FTA* new_fta = new FTA(grammar->symbol_list.size());
    std::vector<NodeStorage> nodes_storage(grammar->symbol_list.size());
    int size = 0;
    for (auto symbol: grammar->symbol_list) {
        nodes_storage[symbol->id].emplace_back();
        new_fta->node_list.emplace_back();
        new_fta->node_list[symbol->id].emplace_back();
        new_fta->val_pos.emplace_back();
        new_fta->val_pos[symbol->id].emplace_back();
    }
    for (size = 1; size <= limit ; ++size) {
        //printf2("size now %d \n", size);
        new_fta->size = size;
        for (auto symbol: grammar->symbol_list) {
            new_fta->node_list[symbol->id].emplace_back();
            new_fta->val_pos[symbol->id].emplace_back();
            nodes_storage[symbol->id].emplace_back();
            for (auto rule: symbol->rule_list) {
                std::vector<std::vector<int>> size_pool;
                for(auto nt: rule->param_list){
                    //printf2("symbol %s\n",rule->toString().c_str());
                    std::vector<int> size_list;
                    for (int i = 0; i < size; i++){
                        if(!nodes_storage[nt->id][i].empty()) size_list.push_back(i);
                    }
                    size_pool.push_back(size_list);
                }
                auto schemeList = getAllSizeScheme(size - 1, size_pool);
                for (const auto& scheme: schemeList) {
                    NodeStorage tmp_storage;
                    for(int i = 0; i < scheme.size(); i++){
                        tmp_storage.push_back(nodes_storage[rule->param_list[i]->id][scheme[i]]);
                    }
                    auto new_nodes = constructNodes(rule, symbol, tmp_storage, env, example, size);
                    for (auto new_node: new_nodes) {
                        if(new_fta->insert_and_merge(new_node) == new_node){
                            nodes_storage[symbol->id][size].push_back(new_node);
                        }
                        else delete new_node;
                    }
                }
            }
        }
    }
    if(cut){
        std::queue<FTANode *> q;
        std::unordered_set<FTANode *> good_nodes;
        for(auto node: new_fta->node_list[grammar->start->id][limit]){
            if(node->oup_list[0].second == example.second){
                new_fta -> root = node;
                q.push(node);
                good_nodes.insert(node);
            }
        }
        if(q.empty()){
            delete new_fta;
            return nullptr;
        }
        while(!q.empty()){
                auto cur = q.front(); q.pop();
                for(auto edge: cur->edge_list){
                    for(auto node: edge->node_list){
                        if(!good_nodes.count(node)){
                            q.push(node);
                            good_nodes.insert(node);
                            //printf2("found good node %p %s\n",node, node->example_program->toString().c_str());
                        }
                    }
                }
                //printf2("q size %d \n", q.size());
            }
        for(auto symbol: grammar->symbol_list){
            for(int i = 1; i <= limit; ++i){
                NodeList cur_good_nodes;
                std::unordered_map<std::string, FTANode *> cur_val_pos;
                for(auto node: new_fta->node_list[symbol->id][i]){
                    if(good_nodes.count(node)){
                        cur_good_nodes.push_back(node);
                        cur_val_pos[node->all_oup] = node;
                    }
                    else delete node;
                }
                new_fta->node_list[symbol->id][i] = cur_good_nodes;
                new_fta->val_pos[symbol->id][i] = cur_val_pos;
            }
        }
    }
    //printf2("new fta \n");
    //printFTA(new_fta, grammar);
    
    return new_fta;
}
// Extract a valid program from a non-empty FTA
PProgram extractProgramFromFTA(FTA* fta, Grammar* grammar, const IOExampleList & examples, Env* env, int example_cnt){
    auto x = grammar->start;
    int i = fta->size;
            for(auto node: fta->node_list[x->id][i]){
                bool found = true;
                for(int j=0; j < example_cnt; j++){
                    if(!(env->run(node->example_program.get(), examples[j].first) == examples[j].second)){
                        //printf2("prog %s fail %s oup %s\n", node->example_program->toString().c_str(), example::ioExample2String(examples[j]).c_str(), env->run(node->example_program.get(), examples[j].first).toString().c_str());
                        found = false;
                        break;
                    }
                }
                if(found)return node->example_program;
            }
    return nullptr;
}
FTA* cutFTA(FTA* fta, const IOExampleList & examples, Grammar* grammar, Env* env){
    if(!fta) return nullptr;
    FTA* new_fta = fta;
    auto x = grammar->start;
    int limit = fta->size;
    std::queue<FTANode *> q;
    std::unordered_set<FTANode *> good_nodes;
    int example_cnt = examples.size();
    for(auto node: new_fta->node_list[x->id][limit]){
        bool found = true;
        for(int j=0; j < example_cnt; j++){
            if(!(env->run(node->example_program.get(), examples[j].first) == examples[j].second)){
                    found = false;
                    break;
                }
            }
            if(found){
                q.push(node);
                good_nodes.insert(node);
            }
        }
    if(q.empty()){
        delete new_fta;
        return nullptr;
    }
    while(!q.empty()){
            auto cur = q.front(); q.pop();
            for(auto edge: cur->edge_list){
                for(auto node: edge->node_list){
                    if(!good_nodes.count(node)){
                        q.push(node);
                        good_nodes.insert(node);
                        //printf2("found good node %p %s\n",node, node->example_program->toString().c_str());
                    }
                }
            }
            //printf2("q size %d \n", q.size());
        }
    // std::cerr<< "found " << good_nodes.size() << " good nodes\n";
    for(auto symbol: grammar->symbol_list){
        for(int i = 1; i <= new_fta->size; ++i){
            NodeList cur_good_nodes;
            std::unordered_map<std::string, FTANode *> cur_val_pos;
            for(auto node: new_fta->node_list[symbol->id][i]){
                if(good_nodes.count(node)){
                    cur_good_nodes.push_back(node);
                    cur_val_pos[node->all_oup] = node;
                }
                else delete node;
            }
            new_fta->node_list[symbol->id][i] = cur_good_nodes;
            new_fta->val_pos[symbol->id][i] = cur_val_pos;
        }
    }
    return new_fta;
}
void makeBag(
    FTANode * node, 
    FTAEdge * edge, 
    Env * env, 
    IOExample example, 
    int pos, 
    std::unordered_map<std::string, FTANode *> & cur_bag, 
    std::unordered_map<FTANode *, std::vector<FTANode *>> & all_bag, 
    FTA* ref_fta, 
    std::vector<FTANode *> & sub_list,
    NonTerminal * symbol,
    int size
    ){
    if(pos == edge->rule->param_list.size()){
        ProgramList prog_list;
        for(auto x: sub_list) prog_list.push_back(x->example_program);
        auto prog = edge->rule->buildProgram(prog_list);
        auto oup = env->run(prog.get(), example.first);
        auto oup_str = oup.toString();
        if(ref_fta->val_pos[symbol->id][size].count(oup_str)){
            if(cur_bag.count(oup_str)){
                FTAEdge * new_edge = new FTAEdge(edge->rule, sub_list);
                cur_bag[oup_str]->edge_list.push_back(new_edge);
                cur_bag[oup_str]->example_program = std::min(cur_bag[oup_str]->example_program, prog);
            }
            else{
                FTANode * new_node = new FTANode(*node);
                new_node->edge_list.clear();
                new_node->example_program = prog;
                new_node->oup_list.push_back(std::make_pair(example.first, oup));
                new_node->all_oup += oup_str;
                FTAEdge * new_edge = new FTAEdge(edge->rule, sub_list);
                new_node->edge_list.push_back(new_edge);
                cur_bag[oup_str] = new_node;
            }
        }
        return;
    }
    for(auto cur_node: all_bag[edge->node_list[pos]]){
        sub_list.push_back(cur_node);
        makeBag(node, edge, env, example, pos+1, cur_bag, all_bag, ref_fta, sub_list, symbol, size);
        sub_list.pop_back();
    }

}
FTA* intersectFTA(FTA* cur_fta, FTA* ref_fta, Grammar * grammar, Env * env, IOExample example) {
    FTA* new_fta = new FTA(grammar->symbol_list.size());
    for(auto symbol: grammar->symbol_list){
        new_fta->node_list.emplace_back();
        new_fta->val_pos.emplace_back();
        new_fta->node_list[symbol->id].emplace_back();
        new_fta->val_pos[symbol->id].emplace_back();
    }
    std::unordered_map<FTANode *, std::vector<FTANode *>> node_bag;
    for(int i = 1; i <= cur_fta->size; i++){
        for(auto symbol: grammar->symbol_list){
            new_fta->node_list[symbol->id].emplace_back();
            new_fta->val_pos[symbol->id].emplace_back();
            for(auto node: cur_fta->node_list[symbol->id][i]){
                node_bag[node] = std::vector<FTANode *>();
                std::unordered_map<std::string, FTANode *> cur_bag;
                for(auto edge: node->edge_list){
                    std::vector<FTANode *> sub_list;
                    makeBag(node, edge, env, example, 0, cur_bag, node_bag, ref_fta, sub_list, symbol, i);
                }
                for(auto new_node: cur_bag) node_bag[node].push_back(new_node.second);
            }
        }
    }
    for(auto symbol: grammar->symbol_list)
        for(int i = 1; i <= cur_fta->size; i++){
            for(auto node: cur_fta->node_list[symbol->id][i]){
                for(auto new_node: node_bag[node]){
                    new_fta->node_list[symbol->id][i].push_back(new_node);
                    new_fta->val_pos[symbol->id][i][new_node->all_oup] = new_node;
                }
            }
        }
    new_fta->size = cur_fta->size;
    return new_fta;
}

const int KIntRange = 5;

IOExampleList testGenRandomExamples(Z3ExampleSpace* example_space) {
    auto gen = [](Type* type) {
        if (dynamic_cast<TBool*>(type)) {
            return BuildData(Bool, rand() & 1);
        }
        if (dynamic_cast<TInt*>(type)) {
            return BuildData(Int, rand() % (2 * KIntRange + 1) - KIntRange);
        }
        LOG(FATAL) << "Unknown type " << type->getName();
    };
    auto* io_space = dynamic_cast<IOExampleSpace*>(example_space);
    IOExampleList examples;
    for (int _ = 0; _ < 100; ++_) {
        Example example;
        for (auto& inp_type: example_space->type_list) {
            example.push_back(gen(inp_type.get()));
        }
        if (io_space) {
            auto new_example = io_space->getIOExample(example);
            LOG(INFO) << "Random IO Example: " << example::ioExample2String(new_example);
            examples.push_back(new_example);
        } else {
            assert(0);
            //LOG(INFO) << "Random Example: " << data::dataList2String(example);
        }
    }
    return examples;
}

IOExampleList extractExamples(FiniteExampleSpace * example_space){
    IOExampleList examples;
    auto* io_space = dynamic_cast<IOExampleSpace*>(example_space);
    assert(io_space);
    for(auto example:example_space->example_space){
        examples.push_back(io_space->getIOExample(example));
    }
    return examples;
}
std::vector<FTA*> single_fta, partial_fta;

std::pair<PProgram, int> synthesisFromExample(Grammar* grammar, Env* env, IOExampleList examples) {

    auto gen = [](Type* type) {
        if (dynamic_cast<TBool*>(type)) {
            return BuildData(Bool, rand() & 1);
        }
        if (dynamic_cast<TInt*>(type)) {
            return BuildData(Int, rand() % 11 - 5);
        }
        LOG(FATAL) << "Unknown type " << type->getName();
    };
    IOExampleList cur_examples;
    int limit = 1;
    FTA* cur_fta;
    for(;; limit++){
        cur_examples.clear();
        single_fta.clear();
        partial_fta.clear();
        //assert(limit <= 7);
        LOG(INFO) << "Limit now " << limit;
        //printf2("limit now %d\n", limit);

        LOG(INFO) << "Example " << example::ioExample2String(examples[0]);
        //printf2("example 0 %s\n",  example::ioExample2String(examples[0]).c_str());
        cur_fta = constructFTA(grammar, examples[0], env, limit);
        cur_examples.push_back(examples[0]);
        single_fta.push_back(cur_fta);
        partial_fta.push_back(cur_fta);
        if(!cur_fta){
            continue;
        }
        bool success = true;
        printf2("example cnt %d\n", examples.size());
        for (int i = 1; i < examples.size(); ++i) {
            printf2("example %d %s\n", i, example::ioExample2String(examples[i]).c_str());
            cur_examples.push_back(examples[i]);
            FTA* example_fta = constructFTA(grammar, examples[i], env, limit);
            single_fta.push_back(example_fta);
            if(!example_fta){
                success = false; break;
            }
            FTA* new_fta = intersectFTA(cur_fta, example_fta, grammar, env, examples[i]);
            // std::cout << "size after intersect " << new_fta->getSize() << std::endl;
            
            new_fta = cutFTA(new_fta, cur_examples, grammar, env);
            partial_fta.push_back(new_fta);
            //delete cur_fta;
            if(!new_fta || new_fta->node_list[grammar->start->id].size() <= limit || new_fta->node_list[grammar->start->id][limit].empty()){
                success=false; 
                printf2("fta empty after intersection\n");break;
            } else {
                // std::cout << "size after cut " << new_fta->getSize() << std::endl;
            }
            
            cur_fta = new_fta;
            //delete example_fta;
            if(!success)break;
        }
        if(!success)continue;
        auto prog = extractProgramFromFTA(cur_fta, grammar, examples, env, examples.size());
        if(!prog) printf2("no valid prog\n");
        
        return {prog, limit};
    }
    
    return {nullptr, -1};
}
std::pair<FTA*, int> constructMergedFTA(Grammar* grammar, Env* env, const IOExampleList& examples, int limit) {
    auto* res = constructFTA(grammar, examples[0], env, limit);
    IOExampleList current_examples;
    int total_cost = 0;
    for (int i = 1; i < examples.size(); ++i) {
        current_examples.push_back(examples[i]);
        auto* next_fta = constructFTA(grammar, examples[i], env, limit);
        res = intersectFTA(res, next_fta, grammar, env, examples[i]);
        total_cost += res->getSize();
        res = cutFTA(res, current_examples, grammar, env);
    }
    return {res, total_cost};
}

void analyzeFTA(Grammar* grammar, Env* env, const IOExampleList& examples, int limit){
    int cur_cnt = 0;
    std::cout << "example num " << examples.size() << std::endl;
    for (int check_step = 1; check_step < 10; ++check_step) {
        std::vector<std::pair<IOExampleList, FTA*>> ref_fta_list;
        int total_cost = 0;
        for (int i = 0; i < 10; ++i) {
            IOExampleList current_examples;
            for (int j = 0; j < check_step; ++j) current_examples.push_back(examples[i * check_step + j]);
            auto [fta, cost] = constructMergedFTA(grammar, env, current_examples, limit);
            ref_fta_list.emplace_back(current_examples, fta);
            total_cost += cost;
        }
        std::cerr << "\n\n==========================================\n\n";
        std::cerr << "Step size " << check_step << " with cost " << total_cost << std::endl;
        for (auto tree: partial_fta) {
            int valid_node_cnt = 0, total_node_cnt = 0;
            std::cerr << "FTA for examples 0 to " << cur_cnt++ << '\n';
            for (auto s: grammar->symbol_list) {
                for (int i = 1; i < tree->size; i++) {
                    for (auto node: tree->node_list[s->id][i]) {
                        ++total_node_cnt;
                        auto min_prog = node->example_program;

                        //std::cerr << "  Program " << min_prog->toString() << std::endl;
                        bool is_valid = true;
                        for (const auto& [test_examples, test_tree]: ref_fta_list) {
                            std::string all_res;
                            for (auto& example: test_examples) {
                                auto res = env->run(min_prog.get(), example.first);
                                all_res += res.toString();
                            }
                            //std::cout << "  " << all_res << std::endl;
                            if (!test_tree->val_pos[s->id][i].count(all_res)) {
                                /*std::cout << "  not found" << std::endl;
                                for (auto possible_val: test_tree->val_pos[s->id][i]) {
                                    std::cout << "    " << possible_val.first << std::endl;
                                }*/
                                is_valid = false; break;
                            }
                        }
                        if (is_valid) {
                            ++valid_node_cnt;
                        }
                        /*bool in_all_trees = true;
                        for (auto single_tree: single_fta) {
                            auto res = env->run(min_prog.get(), example);
                            if (!single_tree->val_pos[s->id][i].count(res.toString())) {
                                in_all_trees = false;
                                break;
                            }
                        }
                        auto final_tree = partial_fta[partial_fta.size() - 1];
                        std::string all_res;
                        for (auto example: examples) {
                            auto res = env->run(min_prog.get(), example.first);
                            all_res += res.toString();
                        }

                        if (final_tree->val_pos[s->id][i].count(all_res)) ++valid_node_cnt;*/
                    }
                }
            }
            std::cerr << valid_node_cnt << " nodes in " << total_node_cnt << " valid\n";
        }
    }
}
std::pair<PProgram, int> synthesisFirstProgram(Grammar* grammar, Env* env) {
  for (auto* symbol : grammar->symbol_list) {
    for (auto* rule : symbol->rule_list) {
      if (rule->param_list.size() != 0) continue;
      ProgramList sub_list;
      PProgram prog = rule->buildProgram(sub_list);
      return {prog, 5};
    }
  }
  assert(false);
}

PProgram cegis(Grammar* grammar, Verifier* v, IOExampleSpace* example_space,
               Env* env, const IOExampleList& full_examples) {
  IOExampleList counter_examples;
  Example counter_example;
  FunctionContext res;
  res[example_space->func_name] = nullptr;
  while (1) {
    auto [candidate, limit] =
        (counter_examples.empty()
             ? synthesisFirstProgram(grammar, env)
             : synthesisFromExample(grammar, env, counter_examples));
    LOG(INFO) << "Candidate " << candidate->toString();
    res[example_space->func_name] = candidate;
    if (v->verify(res, &counter_example)) {
        
        analyzeFTA(grammar, env, full_examples, limit);
        return candidate;
    }
    auto new_example = example_space->getIOExample(counter_example);
    //LOG(INFO) << "New example: " << example::ioExample2String(new_example);
    counter_examples.push_back(new_example);
  }
}

int main(int argv, char** argc) {
    std::string task_path;
    if (argv == 2) {
        task_path = std::string(argc[1]);
    } else task_path = config::KSourcePath + "/tests/max3.sl";
    Verifier *verifier = nullptr;
    auto spec = parser::getSyGuSSpecFromFile(task_path);
    bool is_z3 = false;
    auto *example_space = dynamic_cast<FiniteExampleSpace*>(spec->example_space.get());
    IOExampleList initial_examples;
    if (example_space) {
        verifier = new FiniteExampleVerifier(example_space);
        initial_examples = extractExamples(example_space);
    }
    else {
        auto *z3_example_space = dynamic_cast<Z3ExampleSpace*>(spec->example_space.get());
        if (z3_example_space) {
            initial_examples = testGenRandomExamples(dynamic_cast<Z3ExampleSpace*>(spec->example_space.get()));
            verifier = new Z3Verifier(z3_example_space);
        }
        else assert(0);
    }
    auto* io_space = dynamic_cast<IOExampleSpace*>(spec->example_space.get());
    auto* grammar = spec->info_list[0]->grammar;
    auto* recorder = new TimeRecorder();
    grammar->indexSymbol();
    std::cout << "start synthesis" << std::endl;
    recorder->start("synthesis");
    auto res = cegis(grammar, verifier, io_space, spec->env.get(), initial_examples);
    recorder->end("synthesis");
    std::cout << "Result: " << res->toString() << std::endl;
    std::cout << "Time Cost: " << recorder->query("synthesis") << " seconds\n\n";
}