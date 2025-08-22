#ifndef OBE_H
#define OBE_H

#include "istool/basic/example_space.h"
#include "istool/basic/program.h"
#include "istool/basic/verifier.h"

namespace obe {
std::string synthesis(Specification* spec, Verifier* verifier,
                      IOExampleList examples);
}  // namespace obe

#endif