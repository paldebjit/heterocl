/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_scop_verif.cc
 */
#include <tvm/runtime/config.h>
#include <tvm/packed_func_ext.h>
#include <vector>
#include <string>
#include <tuple>
#include <regex>
#include "./codegen_scop.h"
#include "../../runtime/thread_storage_scope.h"

namespace TVM {
namespace codegen {


// Important PoCC commands that will be used for the verification approach
// Assume PoCC is available in the path. Check for it first
// Step 1: Generate the SCoP. Check if the SCoP exists
// Step 2: Check schedule violation and capture the PoCC output in a log file
//          $ pocc --read-scop kernel.scop --check-schedule --verbose
// Step 3: 
 
}  // namespace codegen
}  // namespace TVM
