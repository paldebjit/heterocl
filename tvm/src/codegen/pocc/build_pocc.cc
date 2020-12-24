/*!
 *  Copyright (c) 2017 by Contributors
 *  Build PoCC artifacts from source.
 * \file build_pocc.cc
 */
#include <tvm/base.h>
#include <tvm/runtime/config.h>
#include <unordered_map>
#include "./codeanalys_pocc.h"
#include "./codegen_pocc.h"
#include "../build_common.h"

namespace TVM {
namespace codegen {

std::string BuildPoCC(Array<LoweredFunc> funcs) {
  using TVM::runtime::Registry;

  CodeAnalysPoCC ca;
  CodeGenPoCC cg;
  for (LoweredFunc f : funcs) {
    // 1st pass: Analyze AST and collect necessary information
    ca.AddFunction(f);
    str2tupleMap<std::string, Type> map_arg_type;
    map_arg_type = ca.Finish();

    // 2nd pass: Generate kernel code
    cg.AddFunction(f, map_arg_type);
  }
  std::string code = cg.Finish();

  if (const auto* f = Registry::Get("tvm_callback_pocc_postproc")) {
    code = (*f)(code).operator std::string();
  }
  LOG(WARNING) << "PoCC doesn't have runtime, return kernel code";
  return code;
}

TVM_REGISTER_API("codegen.build_pocc")
.set_body([](TVMArgs args, TVMRetValue* rv) {
    *rv = BuildPoCC(args[0]);
  });
}  // namespace codegen
}  // namespace TVM
