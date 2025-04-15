#include <algorithm>
#include <queue>

#include "istool/fta/fta.h"
#include "istool/fta/fta_value.h"

using namespace fta;

class StrengthenStringValueGenerator {
    StringValue value;
    std::string str;
    class Iterator {
        const int max_additional_cond_num = 2;
        StringValue value;
        std::vector<int> new_cond_pos;
        std::vector<int> available_pos;
        int additional_cond_num;
        std::string str;

    public:
        Iterator(StringValue value, std::string str)
            : value(value), new_cond_pos(), additional_cond_num(0), str(str) {
            if (value.sv_type == StringValueType::TRUE) {
                for (int i = 0; i < str.length(); i++) {
                    available_pos.push_back(i);
                }
                return;
            }
            assert(value.length.value() == -1 ||
                   value.length.value() == str.length());
            for (int i = 0; i < str.length(); i++) {
                if (!value.at.count(i)) {
                    available_pos.push_back(i);
                }
            }
        }
        StringValue operator*() const { return value; }
        bool genNext() {
            for (int i = additional_cond_num - 1; i >= 0; i--) {
                int p = std::upper_bound(available_pos.begin(),
                                         available_pos.end(), new_cond_pos[i]) -
                        available_pos.begin();
                if (available_pos.size() - p >= additional_cond_num - i) {
                    int cur_new_cond = available_pos[p];
                    for (int j = i; j < additional_cond_num; j++) {
                        value.at.erase(new_cond_pos[j]);
                    }
                    for (int j = i; j < additional_cond_num; j++) {
                        new_cond_pos[j] = cur_new_cond;
                        value.at[cur_new_cond] = str[cur_new_cond];
                        if (j != additional_cond_num - 1)
                            cur_new_cond = *std::upper_bound(
                                available_pos.begin(), available_pos.end(),
                                cur_new_cond);
                    }
                    return true;
                }
            }
            additional_cond_num++;
            if (additional_cond_num > available_pos.size())
                return false;
            if (additional_cond_num > max_additional_cond_num) {
                // generate full string
                additional_cond_num = available_pos.size();
            }
            for (auto x : new_cond_pos)
                value.at.erase(x);
            new_cond_pos.clear();
            for (int i = 0; i < additional_cond_num; i++) {
                new_cond_pos.push_back(available_pos[i]);
                value.at[available_pos[i]] = str[available_pos[i]];
            }
            return true;
        }
        Iterator &operator++() {
            if (value.sv_type == StringValueType::TRUE) {
                value.sv_type = StringValueType::CONDITION;
                value.length = str.length();
                return *this;
            }
            if (genNext())
                return *this;
            value.at.clear();
            value.length = -1;
            return *this;
        }
        bool operator!=(Iterator &other) const {
            return !value.equal(&(other.value));
        }
    };

public:
    StrengthenStringValueGenerator(StringValue *value, std::string str)
        : value(value), str(str) {}
    Iterator begin() const { return Iterator(value, str); }
    Iterator end() const {
        StringValue value(StringValueType::CONDITION);
        value.length = -1;
        return Iterator(value, str);
    }
};

bool needStrengthenChildren(FTANode *parent, int example_index, Value *nv,
                            ExecuteInfo *info) {
    DataList inp_list;
    for (auto p : parent->edge_list[0]->node_list) {
        inp_list.push_back(p->oup_info->getFullOutput()[example_index]);
    }
    return !nv->isSubValue(
        util::mergeOutput(parent->edge_list[0]->semantics.get(),
                          std::move(inp_list), info)
            .get());
}

PInfo buildOupInfo(DataList &list) {
    FixedVector<PInfo> oup_info(list.size());
    for (int i = 0; i < list.size(); i++)
        oup_info[i] = std::make_shared<SingleOutputInfo>(list[i]);
    return std::make_shared<MultiOutputInfo>(std::move(oup_info));
}

void fta::refineAbstractValue(
    FTA *x, Env *env, const IOExample &example,
    std::shared_ptr<value::FTAValueSets> fta_value_sets) {
    int example_index =
        std::find(x->examples.begin(), x->examples.end(), example) -
        x->examples.begin();
    assert(example_index != x->examples.size());
    auto input = example.first;
    auto output = example.second;
    auto *root = x->root_list[0];
    auto *info = env->getExecuteInfoBuilder()->buildInfo(example.first, {});
    auto strengthen_root = [&](FTANode *root) {
        // LOG(INFO) << "strengthening root...";
        auto oup_list = root->oup_info->getFullOutput();
        auto *v = dynamic_cast<StringValue *>(oup_list[example_index].get());
        auto prog = util::extractMinimalProgramFromNode(root);
        if (!v) {
            oup_list[example_index] = env->run(prog.get(), input);
            root->oup_info = buildOupInfo(oup_list);
            return;
        }
        auto std_out = dynamic_cast<StringValue *>(output.value.get())
                           ->getFullString()
                           .value();
        int ind = 0;
        auto prog_out_data = env->run(prog.get(), input);
        auto prog_out = dynamic_cast<StringValue *>(prog_out_data.get())
                            ->getFullString()
                            .value();
        // LOG(INFO) << "prog: " << prog->toString();
        // LOG(INFO) << "prog_out: " << prog_out << ' ' << prog_out.length();
        if (v->sv_type != StringValueType::CONDITION) {
            v->sv_type = StringValueType::CONDITION;
            v->length = prog_out.length();
            root->oup_info = buildOupInfo(oup_list);
            return;
        }
        int target_index = -1;
        for (auto c : std_out) {
            if (!v->at.count(ind)) {
                if (c != prog_out[ind]) {
                    v->at[ind] = prog_out[ind];
                    DataList inp_list;
                    for (auto p : root->edge_list[0]->node_list) {
                        inp_list.push_back(
                            p->oup_info->getFullOutput()[example_index]);
                    }
                    if (!needStrengthenChildren(root, example_index, v, info)) {
                        target_index = ind;
                        break;
                    } else if (target_index == -1) {
                        target_index = ind;
                    }
                    v->at.erase(ind);
                }
            } else {
                assert(v->at[ind] == c);
            }
            ind++;
        }
        assert(target_index != -1);
        v->at[target_index] = prog_out[target_index];
        Data data = BuildData(String, v);
        fta_value_sets->addValue(example, data);
        oup_list[example_index] = data;
        root->oup_info = buildOupInfo(oup_list);
    };
    auto is_better_list = [](const DataList &cur,
                             const std::optional<DataList> &best) {
        if (!best.has_value()) {
            return true;
        }
        assert(cur.size() == best.value().size());
        int cur_cost = 0;
        for (int i = 0; i < cur.size(); i++) {
            auto *p = dynamic_cast<StringValue *>(cur[i].get());
            if (p) {
                cur_cost += p->length.has_value();
                cur_cost += p->at.size();
            }
        }
        int best_cost = 0;
        for (int i = 0; i < best.value().size(); i++) {
            auto *p = dynamic_cast<StringValue *>(best.value()[i].get());
            if (p) {
                best_cost += p->length.has_value();
                best_cost += p->at.size();
            }
        }
        return cur_cost < best_cost;
    };
    auto strengthen_children_dfs = [&](auto self, int dep, FTANode *parent,
                                       DataList cur_inp_list,
                                       std::optional<DataList> &best_inp_list) {
        auto &child_nodes = parent->edge_list[0]->node_list;
        if (dep == child_nodes.size()) {
            auto *semantics = parent->edge_list[0]->semantics.get();
            auto inp_list_new = cur_inp_list;
            auto exec_res =
                util::mergeOutput(semantics, std::move(inp_list_new), info);
            if (parent->oup_info->getFullOutput()[example_index]
                    .value->isSubValue(exec_res.value.get())) {
                if (is_better_list(cur_inp_list, best_inp_list))
                    best_inp_list = cur_inp_list;
            }
            return;
        }
        auto cur_child_oup_list = child_nodes[dep]->oup_info->getFullOutput();
        auto v = dynamic_cast<StringValue *>(
            cur_child_oup_list[example_index].get());
        if (!v) {
            // TODO: handle abstract int value and bool value
            cur_inp_list.push_back(env->run(
                util::extractMinimalProgramFromNode(child_nodes[dep]).get(),
                input));
            self(self, dep + 1, parent, cur_inp_list, best_inp_list);
            cur_inp_list.pop_back();
            return;
        }
        for (auto new_value : StrengthenStringValueGenerator(
                 v,
                 env->run(util::extractMinimalProgramFromNode(child_nodes[dep])
                              .get(),
                          input)
                     .toString())) {
            cur_inp_list.push_back(BuildData(String, new_value));
            self(self, dep + 1, parent, cur_inp_list, best_inp_list);
            cur_inp_list.pop_back();
        }
        return;
    };
    auto strengthen_children = [&](FTANode *parent) {
        // LOG(INFO) << "strengthening children...";
        DataList cur_inp_list;
        std::optional<DataList> best_inp_list;
        strengthen_children_dfs(strengthen_children_dfs, 0, parent,
                                cur_inp_list, best_inp_list);
        auto &child_nodes = parent->edge_list[0]->node_list;
        assert(best_inp_list.has_value());
        int ind = 0;
        for (auto data : best_inp_list.value()) {
            auto oup_list = child_nodes[ind]->oup_info->getFullOutput();
            oup_list[example_index] = data;
            child_nodes[ind]->oup_info = buildOupInfo(oup_list);
            fta_value_sets->addValue(example, data);
            ind++;
        }
    };
    strengthen_root(root);
    std::queue<FTANode *> q;
    q.push(root);
    while (!q.empty()) {
        auto p = q.front();
        q.pop();
        strengthen_children(p);
        for (auto node : p->edge_list[0]->node_list) {
            q.push(node);
        }
    }
    delete info;
}