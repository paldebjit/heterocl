/*!
 *  Copyright (c) 2017 by Contributors
 * \file codegen_pocc.cc
 */
#include <tvm/runtime/config.h>
#include <tvm/packed_func_ext.h>
#include <vector>
#include <string>
#include <tuple>
#include <regex>
#include "./codegen_pocc.h"
#include "../../runtime/thread_storage_scope.h"

namespace TVM {
namespace codegen {

CodeGenPoCC::CodeGenPoCC() {
  restrict_keyword_ = "restrict"; // FIXME: Check if this is useful
  return ;
}

void CodeGenPoCC::InitFuncState(LoweredFunc f) {
  CodeGenC::InitFuncState(f);
  for (Var arg : f->args) {
    if (arg.type().is_handle()) {
      alloc_storage_scope_[arg.get()] = "global";
    }
  }
  return ;
}

void CodeGenPoCC::AddFunction(LoweredFunc f,
        str2tupleMap<std::string, Type> map_arg_type) {
  // Clear previous generated state
  this->InitFuncState(f);

  // Skip the first underscore, so SSA variable starts from _1
  GetUniqueName("_");

  // Register alloc buffer type
  for (const auto & kv : f->handle_data_type) {
    RegisterHandleType(kv.first.get(), kv.second.type());
  }

  // Write header files
  this->stream << "#include <string.h>\n";
  this->stream << "#include <math.h>\n";
  this->stream << "#include <assert.h>\n";

  // Write entry function name
  this->stream << "#pragma ACCEL kernel\n";
  this->stream << "void " << f->name << "(";

  // Write arguments
  for (size_t i = 0; i < f->args.size(); ++i) {
    Var v = f->args[i];
    std::string vid = AllocVarID(v.get());
    if (i != 0) this->stream << ", ";
    if (map_arg_type.find(vid) == map_arg_type.end()) {
      LOG(WARNING) << vid << " type not found\n";
      PrintType(v.type(), this->stream);
      this->stream << ' ' << vid;
    }
    else {
      auto arg = map_arg_type[vid];
      PrintType(std::get<1>(arg), this->stream);
      if (v.type().is_handle())
        this->stream << "*";
      this->stream << ' ' << std::get<0>(arg);
    }
  }
  stream << ") {\n";
  int func_scope = this->BeginScope();
  this->PrintStmt(f->body);
  this->EndScope(func_scope);
  this->PrintIndent();
  this->stream << "}\n\n";
}

std::string CodeGenPoCC::Finish() {
  return CodeGenC::Finish();
}

void CodeGenPoCC::BindThreadIndex(const IterVar& iv) {
  LOG(FATAL) << "Merlin doesn't support thread binding";
  return ;
}

void CodeGenPoCC::PrintType(Type t, std::ostream& os) {  // NOLINT(*)
  int lanes = t.lanes();
  if (t.is_handle()) {
    //LOG(FATAL) << "The buffer shouldn't call PrintType for printing type";
    os << "void*";
    return ;
  }
  bool fail = false;
  if (t.is_float()) {
    switch (t.bits()) {
      case 16: os << "half"; break;
      case 32: os << "float"; break;
      case 64: os << "double"; break;
      case 128: os << "double double"; break;
      default: fail = true; break;
    }
    if (!fail && lanes == 1) return;
    if (!fail && (lanes >= 2 && lanes <= 16)) {
      os << lanes; return;
    }
  } else if (t.is_uint() || t.is_int()) {
    if (t.is_uint()) {
      os << "unsigned ";
    }
    if (t.bits() == 8 && t.lanes() == 4) {
      // directly 4 8 bit int in integer.
      os << "int"; return;
    }

    int target_bit = 1;
    while (target_bit < t.bits())
      target_bit <<= 1;

    switch (target_bit) {
      case 1: os << "int"; break;
      case 2: os << "char"; break;
      case 4: os << "char"; break;
      case 8: os << "char"; break;
      case 16: os << "short"; break;
      case 32: os << "int"; break;
      case 64: os << "long"; break;
      case 128: os << "long"; break; // FIXME: Should use long long
      default: fail = true; break;
    }
    if (!fail && lanes == 1) return;
    // FIXME: Not yet support multiple lanes
    //if (!fail && (lanes >= 2 && lanes <= 16)) {
    //  os << lanes; return;
    //}
  }
  os << t;
  LOG(WARNING) << "Cannot convert type " << t ;
  return ;
}

void CodeGenPoCC::PrintVecAddr(const Variable* buffer, Type t,
                                 Expr base, std::ostream& os) {  // NOLINT(*)
  // FIXME: What's this node for?
  if (!HandleTypeMatch(buffer, t.element_of())) {
    os << '(';
    auto it = alloc_storage_scope_.find(buffer);
    if (it != alloc_storage_scope_.end()) {
      PrintStorageScope(it->second, os);
    }
    os << ' ';
    PrintType(t.element_of(), os);
    os << "*)";
  }
  os << GetVarID(buffer) << " + ";
  PrintExpr(base, os);
  return ;
}

void CodeGenPoCC::PrintVecStore(const Variable* buffer,
                                  Type t, Expr base,
                                  const std::string& value) {
  // FIXME: What's this node for?
  this->PrintIndent();
  stream << "vstore" << t.lanes() << "(" << value << ", 0, ";
  PrintVecAddr(buffer, t, base, stream);
  stream << ");\n";
  return ;
}

void CodeGenPoCC::PrintStorageSync(const Call* op) {
  const std::string& sync = op->args[0].as<StringImm>()->value;
  if (sync == "warp") {
    LOG(FATAL) << "warp sync not supported in Merlin";
  } else if (sync == "shared") {
    LOG(FATAL) << "shared sync not supported in Merlin";
  } else if (sync == "global") {
    LOG(FATAL) << "global sync not supported in Merlin";
  }
  return ;
}

void CodeGenPoCC::PrintStorageScope(
    const std::string& scope, std::ostream& os) { // NOLINT(*)
    return ;
}

void CodeGenPoCC::VisitExpr_(const Broadcast* op, std::ostream& os) { // NOLINT(*)
  std::string v = PrintExpr(op->value);
  os << "((";
  PrintType(op->type, os);
  os << ")(";
  for (int i = 0; i < op->lanes; ++i) {
    if (i != 0) os << ", ";
    os << v;
  }
  os << "))";
  return ;
}

void CodeGenPoCC::VisitStmt_(const LetStmt* op) {
  std::string value = PrintExpr(op->value);
  // Skip the argument retrieving assign statement
  std::string vid = AllocVarID(op->var.get());
  if (op->var.type() != Handle() &&
      value.find("TVMArray") == std::string::npos &&
      value.find("arg") != 0) {
    PrintIndent();
    PrintType(op->var.type(), this->stream);
    this->stream << ' '
                 << vid
                 << " = " << value << ";\n";
  }
  PrintStmt(op->body);
}

void CodeGenPoCC::VisitStmt_(const For* op) {
  /*
  if (op->for_type == ForType::Parallel)
    stream << "#pragma ACCEL parallel\n";
  else if (op->for_type == ForType::Unrolled) {
    int unroll_factor = 0;
    int i = 0;
    for (auto key : op->annotate_keys) {
      if (auto str = key.as<StringImm>()) {
        auto factor = op->annotate_values[i].as<IntImm>();
        if (str->value == "factor" && factor != nullptr && factor->value > 1) {
          unroll_factor = factor->value;
          break ;
        }
      }
      i++;
    }
    stream << "#pragma ACCEL parallel ";
    if (unroll_factor > 0)
      stream << "factor=" << unroll_factor << " ";
    stream << "flatten\n";
  }
  else if (op->for_type == ForType::Pipelined)
    stream << "#pragma ACCEL pipeline\n";
  CodeGenC::VisitStmt_(op);
  */
  PrintIndent();
  std::string extent = PrintExpr(op->extent);
  std::string min = PrintExpr(op->min);
  std::string vid = AllocVarID(op->loop_var.get());

  PrintIndent();
  stream << "{\n";
  stream << "vid = " << vid << " "
         << "Lbound = " << min << " "
         << "extent = " << extent << ";\n";
  int for_scope = BeginScope();
  PrintStmt(op->body);
  this->EndScope(for_scope);
  PrintIndent();
  stream << "}\n";
}

void CodeGenPoCC::VisitStmt_(const Store* op) {
  Type t = op->value.type();
  if (t.lanes() == 1) {
    std::string value = this->PrintExpr(op->value);
    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    this->PrintIndent();
    stream << "STORE: " << ref << " = " << value << ";\n";
  }/* else {
    CHECK(is_one(op->predicate))
        << "Predicated store is not supported";
    Expr base;
    if (TryGetRamp1Base(op->index, t.lanes(), &base)) {
      std::string value = this->PrintExpr(op->value);
      this->PrintVecStore(op->buffer_var.get(), t, base, value);
    } else {
      // The assignment below introduces side-effect, and the resulting value cannot
      // be reused across multiple expression, thus a new scope is needed
      int vec_scope = BeginScope();

      // store elements seperately
      std::string index = SSAGetID(PrintExpr(op->index), op->index.type());
      std::string value = SSAGetID(PrintExpr(op->value), op->value.type());
      std::string vid = GetVarID(op->buffer_var.get());
      for (int i = 0; i < t.lanes(); ++i) {
        this->PrintIndent();
        stream << vid;
        stream << '[';
        PrintVecElemLoad(index, op->index.type(), i, stream);
        stream << "] = ";
        PrintVecElemLoad(value, op->value.type(), i, stream);
        stream << ";\n";
      }
      EndScope(vec_scope);
    }
  }*/
}

void CodeGenPoCC::VisitExpr_(const Load* op, std::ostream& os) {
  //int lanes = op->type.lanes();
  // delcare type.
  if (op->type.lanes() == 1) {
    std::string ref = this->GetBufferRef(op->type, op->buffer_var.get(), op->index);
    os << "LOAD: " << ref;
  } /*else {
    CHECK(is_one(op->predicate))
        << "predicated load is not supported";
    Expr base;
    if (TryGetRamp1Base(op->index, op->type.lanes(), &base)) {
      std::string ref = GetVecLoad(op->type, op->buffer_var.get(), base);
      os << ref;
    } else {
      // The assignment below introduces side-effect, and the resulting value cannot
      // be reused across multiple expression, thus a new scope is needed
      int vec_scope = BeginScope();

      // load seperately.
      std::string svalue = GetUniqueName("_");
      this->PrintIndent();
      this->PrintType(op->type, stream);
      stream << ' ' << svalue << ";\n";
      std::string sindex = SSAGetID(PrintExpr(op->index), op->index.type());
      std::string vid = GetVarID(op->buffer_var.get());
      Type elem_type = op->type.element_of();
      for (int i = 0; i < lanes; ++i) {
        std::ostringstream value_temp;
        if (!HandleTypeMatch(op->buffer_var.get(), elem_type)) {
          value_temp << "((";
          if (op->buffer_var.get()->type.is_handle()) {
            auto it = alloc_storage_scope_.find(op->buffer_var.get());
            if (it != alloc_storage_scope_.end()) {
              PrintStorageScope(it->second, value_temp);
              value_temp << ' ';
            }
          }
          PrintType(elem_type, value_temp);
          value_temp << "*)" << vid << ')';
        } else {
          value_temp << vid;
        }
        value_temp << '[';
        PrintVecElemLoad(sindex, op->index.type(), i, value_temp);
        value_temp << ']';
        PrintVecElemStore(svalue, op->type, i, value_temp.str());
      }
      os << svalue;
      EndScope(vec_scope);
    }
  }*/
}

// Print a reference expression to a buffer.
std::string CodeGenPoCC::GetBufferRef(Type t, const Variable* buffer, Expr index) {
    
  return "GetBufferRef";
//  std::ostringstream os;
//  std::string vid = GetVarID(buffer);
//  std::string scope;
//  if (alloc_storage_scope_.count(buffer)) {
//    scope = alloc_storage_scope_.at(buffer);
//  }
//  bool is_vol = volatile_buf_.count(buffer) != 0;
//  if (t.lanes() == 1) {
//    bool is_scalar = (buf_length_map_.count(buffer) == 1 &&
//        buf_length_map_[buffer] == 1);
//    if (is_scalar) {
//      os << vid;
//    } else {
//      os << vid;
//      os << '[';
//      PrintExpr(index, os);
//      os << ']';
//    }
//  }
//  return os.str();
}

void CodeGenPoCC::VisitStmt_(const IfThenElse* op) {
  std::string cond = PrintExpr(op->condition);

  // Skip the buffer data checking
  if (std::regex_match(cond, std::regex("!\\((arg)(.+)(== NULL)\\)")))
      return ;

  PrintIndent();
  if (cond[0] == '(' && cond[cond.length() - 1] == ')') {
    stream << "if " << cond << " {\n";
  } else {
    stream << "if (" << cond << ") {\n";
  }
  int then_scope = BeginScope();
  PrintStmt(op->then_case);
  this->EndScope(then_scope);

  if (op->else_case.defined()) {
    PrintIndent();
    stream << "} else {\n";
    int else_scope = BeginScope();
    PrintStmt(op->else_case);
    this->EndScope(else_scope);
  }
  PrintIndent();
  stream << "}\n";
}
}  // namespace codegen
}  // namespace TVM
