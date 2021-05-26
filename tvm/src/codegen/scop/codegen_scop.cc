/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_scop.cc
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

CodeGenSCoP::CodeGenSCoP() {
  restrict_keyword_ = "restrict"; // FIXME: Check if this is useful
  return ;
}

void CodeGenSCoP::InitFuncState(LoweredFunc f) {
  CodeGenC::InitFuncState(f);
  for (Var arg : f->args) {
    if (arg.type().is_handle()) {
      alloc_storage_scope_[arg.get()] = "global";
    }
  }
  return ;
}

void CodeGenSCoP::AddFunction(LoweredFunc f,
        str2tupleMap<std::string, Type> map_arg_type) {
  // Clear previous generated state
  this->InitFuncState(f);

  // Skip the first underscore, so SSA variable starts from _1
  GetUniqueName("_");

  // Register alloc buffer type
  for (const auto & kv : f->handle_data_type) {
    RegisterHandleType(kv.first.get(), kv.second.type());
  }

  // Write arguments
  // FIXME: Don't know why removing the for loop creates a Codegen error. Has to ask.
  for (size_t i = 0; i < f->args.size(); ++i) {
    Var v = f->args[i];
    std::string vid = AllocVarID(v.get());
  }
  
  int func_scope = this->BeginScope();
  this->PrintStmt(f->body);
  this->EndScope(func_scope);

  this->AssembleSCoP();

  this->Verify();
}

std::string CodeGenSCoP::Finish() {
  return CodeGenC::Finish();
}

void CodeGenSCoP::BindThreadIndex(const IterVar& iv) {
  LOG(FATAL) << "SCoP doesn't support thread binding";
  return ;
}

void CodeGenSCoP::PrintType(Type t, std::ostream& os) {  // NOLINT(*)
}

void CodeGenSCoP::PrintVecAddr(const Variable* buffer, Type t,
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

void CodeGenSCoP::PrintVecStore(const Variable* buffer,
                                  Type t, Expr base,
                                  const std::string& value) {
  // FIXME: What's this node for?
  this->PrintIndent();
  stream << "vstore" << t.lanes() << "(" << value << ", 0, ";
  PrintVecAddr(buffer, t, base, stream);
  stream << ");\n";
  return ;
}

void CodeGenSCoP::PrintStorageSync(const Call* op) {
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

void CodeGenSCoP::PrintStorageScope(
    const std::string& scope, std::ostream& os) { // NOLINT(*)
    return ;
}

void CodeGenSCoP::VisitExpr_(const Broadcast* op, std::ostream& os) { // NOLINT(*)
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

void CodeGenSCoP::VisitStmt_(const LetStmt* op) {
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

void CodeGenSCoP::VisitStmt_(const For* op) {
  // FIXME: extent j = 18 + 2 * i + N
  
  iter_bounds ib;

  std::string extent = PrintExpr(op->extent);
  std::string min = PrintExpr(op->min);
  std::string vid = AllocVarID(op->loop_var.get());

  this->MapVid(vid);

  min_extent_coeff_map.insert({"_CONSTANT_", min});
  ib.LB = make_tuple(vid, min_extent_coeff_map);
  min_extent_coeff_map.clear();

  char to_remove[] = "()";
  
  for (auto to_remove_ : to_remove) {
      extent.erase(std::remove(extent.begin(), extent.end(), to_remove_), extent.end());
  }

  std::vector<std::string> tokens;
  tokens = this->Split(extent, "+-");

  /* Updating UB map */
  for (auto token: tokens) {
      // Checking for - sign at the begining
      std::string sign{""};
      if(token[0] == '-') {
          token = token.substr(1);
          sign = "-";
      }
      // Matching constants
      if(this->IsNumeric(token)) {
          int token_ = stoi(token) - 1;
          this->UpdateIterCoefficient("_CONSTANT_", sign + std::to_string(token_));
          continue;
      }
      // Matching iterators and parameters
      bool found = token.find("*") != std::string::npos;
      // No coefficient for the iterator or the Parameter. Store with default Coefficient 1
      if (!found) {
          this->UpdateIterCoefficient(token, sign + "1");
      } else {
          std::vector<std::string> token_ = this->Split(token, '*');
          this->UpdateIterCoefficient(token_[0], sign + token_[1]);
      }
  }

  ib.UB = make_tuple(vid, min_extent_coeff_map);
  min_extent_coeff_map.clear();

  int for_scope = BeginScope();

  schedule.insert({vid, 0});

  curr_iterators.push_back(vid);

  this->PushIterBounds(ib);

  PrintStmt(op->body);

  this->PopIterBounds();

  this->EndScope(for_scope);
  
  // readjusting the unordered map once the scope changes
  std::string back = curr_iterators.back();
  auto found = schedule.find(back);
  if (found != schedule.end()) {
      schedule.erase(found);
  }
  curr_iterators.pop_back();
  if (!curr_iterators.empty()) {
      std::string back = curr_iterators.back();
      auto found = schedule.find(back);
      if (found != schedule.end()) {
          found->second += 1;
      }
  } else {
      // This is needed for maintaining schedule among different loop nests in the code
      Schedule += 1;
  }
}

void CodeGenSCoP::VisitStmt_(const Allocate* op) {
  CHECK(!is_zero(op->condition));
  std::string vid = AllocVarID(op->buffer_var.get());

  if (op->new_expr.defined()) {
    CHECK_EQ(op->free_function, "nop");
  } else {
    int32_t constant_size = op->constant_allocation_size();
    CHECK_GT(constant_size, 0)
        << "Can only handle constant size stack allocation for now";
    const Variable* buffer = op->buffer_var.as<Variable>();

    std::string scope; // allocate on local scope by default 
    auto it = alloc_storage_scope_.find(buffer);
    if (it != alloc_storage_scope_.end())
      scope = alloc_storage_scope_.at(buffer);
    else scope = "local";

    buf_length_map_[buffer] = constant_size;
  }
  RegisterHandleType(op->buffer_var.get(), op->type);
  this->PrintStmt(op->body);
}

void CodeGenSCoP::VisitStmt_(const Store* op) {
  Type t = op->value.type();
  if (t.lanes() == 1) {
    iter_domain_matrix = this->ConstructIterDomMatrix();

    std::string value = this->PrintExpr(op->value);
    read_access_matrix =  this->ConstructReadWriteAccessMatrix();   
    /* The step is needed to ensure that the coefficients from the read
     * access are cleared */
    read_write_variable.clear();
    read_write_coeff_map.clear();

    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    write_access_matrix = this->ConstructReadWriteAccessMatrix();
    /* This step is needed to ensure that the coefficients from the write
     * access are cleared */
    read_write_variable.clear();
    read_write_coeff_map.clear();

    scattering_matrix = this->ConstructScatteringMatrix();

    std::string back = curr_iterators.back();
    auto found = schedule.find(back);
    if (found != schedule.end()) {
        found->second += 1;
    }

    this->IncrStmtNum();
    this->SetAccessFuncStat();
    this->SetScatFuncStat();

    std::string statement = ref + " = " + value;
    this->ConstructSCoP(statement);
  }
}

void CodeGenSCoP::VisitExpr_(const Load* op, std::ostream& os) {
  if (op->type.lanes() == 1) {
    std::string ref = this->GetBufferRef(op->type, op->buffer_var.get(), op->index);
    os << ref;
  }
}

std::string CodeGenSCoP::GetBufferRef(Type t, const Variable* buffer, Expr index) {
  std::ostringstream os;
  std::ostringstream expr;
  char to_remove[] = "()";

  std::string vid = GetVarID(buffer);

  this->MapVid(vid);

  std::string scope;
  if (alloc_storage_scope_.count(buffer)) {
    scope = alloc_storage_scope_.at(buffer);
  }
  
  if (t.lanes() == 1) {
    bool is_scalar = (buf_length_map_.count(buffer) == 1 &&
        buf_length_map_[buffer] == 1);
    if (is_scalar) {
      os << vid;
      read_write_coeff.insert({});
      this->UpdateReadWriteAccessCoefficient(vid);
      read_write_coeff.clear();
    } else {

      PrintExpr(index, expr);
      std::string expr_ = expr.str();

      os << vid;
      os << '[';
      os << expr_;
      os << ']';

      for (auto to_remove_ : to_remove) {
          expr_.erase(std::remove(expr_.begin(), expr_.end(), to_remove_), expr_.end());
      }

      // FIXME: For an expression 2 * x + 5 * y + N, this routine will treat N as 
      //        an iteration coefficient as well. 
      //        To fix this, do the follwoing:
      //        i) Check the token/token_[0] against iterators.
      //            a) if found put it inside UpdateIterCoeff
      //            b) if not found put it inside UpdateParamCoeff
      // NOTE:  Implemented above. Need to check.
      // FIXME: How to tackle stencil kind of array?
      std::vector<std::string> tokens;
      tokens = this->Split(expr_, "+-");
      for (auto token: tokens) {
          // Checking for - sign at the beginning
          std::string sign{""};
          if(token[0] == '-') {
              token = token.substr(1);
              sign = "-";
          }
          // Matching constants
          if(this->IsNumeric(token)) {
              this->UpdateReadWriteAccessCoefficient("_CONSTANT_", sign + token);
              continue;
          }
          // Matching iterators and parameters
          bool found = token.find("*") != std::string::npos;
          // No coefficient for the iterator or the Parameter. Store with default coefficient 1
          if (!found) {
              this->UpdateReadWriteAccessCoefficient(token, sign + "1");
          } else {
              std::vector<std::string> token_ = this->Split(token, '*');
              this->UpdateReadWriteAccessCoefficient(token_[0], sign + token_[1]);
          }
      }
      this->UpdateReadWriteAccessCoefficient(vid);
      read_write_coeff.clear();
    }
  }
  return os.str();
}

void CodeGenSCoP::VisitStmt_(const IfThenElse* op) {
  std::string cond = PrintExpr(op->condition);

  // Skip the buffer data checking
  if (std::regex_match(cond, std::regex("!\\((arg)(.+)(== NULL)\\)")))
      return ;
  /*
  PrintIndent();
  if (cond[0] == '(' && cond[cond.length() - 1] == ')') {
    stream << "if " << cond << " {\n";
  } else {
    stream << "if (" << cond << ") {\n";
  }
  */
  int then_scope = BeginScope();
  PrintStmt(op->then_case);
  this->EndScope(then_scope);

  if (op->else_case.defined()) {
    /*
    PrintIndent();
    stream << "} else {\n";
    */
    int else_scope = BeginScope();
    PrintStmt(op->else_case);
    this->EndScope(else_scope);
  }
  /*
  PrintIndent();
  stream << "}\n";
  */
}

inline void PrintConst(const IntImm* op, std::ostream& os, CodeGenSCoP* p) { // NOLINT(*)
  if (op->type == Int(32)) {
    std::ostringstream temp;
    temp << op->value;
    p->MarkConst(temp.str());
    os << temp.str();
  } else {
    os << "(";
    p->PrintType(op->type, os);
    os << ")" << op->value;
  }
}

inline void PrintConst(const UIntImm* op, std::ostream& os, CodeGenSCoP* p) { // NOLINT(*)
  if (op->type == UInt(32)) {
    std::ostringstream temp;
    temp << op->value << "U";
    p->MarkConst(temp.str());
    os << temp.str();
  } else {
    os << "(";
    p->PrintType(op->type, os);
    os << ")" << op->value;
  }
}

inline void PrintConst(const FloatImm* op, std::ostream& os, CodeGenSCoP* p) { // NOLINT(*)
  switch (op->type.bits()) {
    case 64: case 32: {
      std::ostringstream temp;
      temp << std::scientific << op->value;
      if (op->type.bits() == 32) temp << 'f';
      p->MarkConst(temp.str());
      os << temp.str();
      break;
    }
    case 16: {
      os << '(';
      p->PrintType(op->type, os);
      os << ')' << std::scientific <<op->value << 'f';
      break;
    }
    default: LOG(FATAL) << "Bad bit-width for float: " << op->type << "\n";
  }
}

void CodeGenSCoP::VisitExpr_(const IntImm *op, std::ostream& os) {  // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenSCoP::VisitExpr_(const UIntImm *op, std::ostream& os) {  // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenSCoP::VisitExpr_(const FloatImm *op, std::ostream& os) { // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenSCoP::VisitExpr_(const StringImm *op, std::ostream& os) { // NOLINT(*)
  os << "\"" << op->value << "\"";
}

}  // namespace codegen
}  // namespace TVM
