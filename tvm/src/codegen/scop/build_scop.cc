/*!
 *  Copyright (c) 2021 by Contributors
 *  Build SCoP artifacts from source.
 * \file build_scop.cc
 */
#include <tvm/base.h>
#include <tvm/runtime/config.h>
#include <unordered_map>
#include "./codeanalys_scop.h"
#include "./codegen_scop.h"
#include "../build_common.h"

namespace TVM {
namespace codegen {

std::string BuildSCoP(Array<LoweredFunc> funcs) {
  using TVM::runtime::Registry;

  CodeAnalysSCoP ca;
  CodeGenSCoP cg;
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
  LOG(WARNING) << "SCoP doesn't have runtime, return kernel code";
  return code;
}

TVM_REGISTER_API("codegen.build_scop")
.set_body([](TVMArgs args, TVMRetValue* rv) {
    *rv = BuildSCoP(args[0]);
  });
}  // namespace codegen
}  // namespace TVM
