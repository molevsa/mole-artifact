//
// Created by pro on 2022/1/18.
//

#ifndef ISTOOL_COMPOSED_SEMANTICS_H
#define ISTOOL_COMPOSED_SEMANTICS_H

#include "istool/basic/program.h"

class ComposedSemantics: public FullExecutedSemantics {
public:
    PProgram body;
    int param_num;
    ComposedSemantics(const PProgram& _body, int _param_num, const std::string& name="");
    virtual Data run(DataList&& inp_list, ExecuteInfo* info);
    ~ComposedSemantics() = default;
};

#endif //ISTOOL_COMPOSED_SEMANTICS_H
