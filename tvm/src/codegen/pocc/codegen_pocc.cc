/*!
 *  Copyright (c) 2021 by Contributors
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

std::string CodeGenPoCC::CreateDelimiter(std::string symbol) {
    std::string delimiter;
    delimiter = "# ";
    for(int i = 0; i < 47; i ++) {
        delimiter = delimiter + symbol;
    }
    delimiter = delimiter + " ";
    return delimiter;
}

void CodeGenPoCC::IncrStmtNum() {
    no_of_stmt = no_of_stmt + 1;
}

int CodeGenPoCC::GetStmtNum() {
    return no_of_stmt;
}

void CodeGenPoCC::ResetAccessFuncStat() {
    access_func_stmt = false;
}

void CodeGenPoCC::SetAccessFuncStat() {
    access_func_stmt = true;
}

bool CodeGenPoCC::GetAccessFuncStat() {
    return access_func_stmt;
}

void CodeGenPoCC::SetScatFuncStat() {
    scat_func_stmt = true;
}

void CodeGenPoCC::ResetScatFuncStat() {
    scat_func_stmt = false;
}

bool CodeGenPoCC::GetScatFuncStat() {
    return scat_func_stmt;
}

void CodeGenPoCC::InsertParams(std::string param) {
    parameters.push_back(param);
}

int CodeGenPoCC::GetParams() {
    return parameters.size();
}

bool CodeGenPoCC::IsParamEmpty() {
    return parameters.empty();
}

void CodeGenPoCC::PushIterBounds(iter_bounds ib) {
    iterators.push_back(ib);
}

void CodeGenPoCC::PopIterBounds() {
    iterators.pop_back();
}

int CodeGenPoCC::SizeIterBounds() {
    return iterators.size();
}

std::vector<iter_bounds> CodeGenPoCC::GetIterBounds() {
    return iterators;
}

int CodeGenPoCC::SizeIterCoeff() {
    return iterator_coeff_dict.size();
}


int CodeGenPoCC::Index(std::string vid, std::vector<std::string> vmap) {
    int index{-1};
    auto it = std::find(vmap.begin(), vmap.end(), vid);
    if (it != vmap.end()) {
        index = it - vmap.begin();
    }
    return index;
}

std::vector<std::string> CodeGenPoCC::Split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(this->Strip(item));
    }

    return result;
}

std::string CodeGenPoCC::Strip(const std::string &s) {
    auto start = s.begin();
    auto end = s.rbegin();

    while (std::isspace(*start)) {
        ++start;
    }
    while (std::isspace(*end)) {
        ++end;
    }

    return std::string(start, end.base());
}

std::string CodeGenPoCC::Join(std::vector<std::string> v, std::string delim) {
    std::string s{""};
    for (size_t i = 0; i < v.size(); i++) {
        if (i == v.size() - 1) {
            s = s + v[i];
        } else {
            s = s + v[i] + delim;
        }
    }
    return s;
}

void CodeGenPoCC::MapVid(std::string vid) {
    read_write_variable.push_back(vid);
    if (std::find(vid_map.begin(), vid_map.end(), vid) == vid_map.end()) {
        vid_map.push_back(vid);
    }
}

std::string CodeGenPoCC::WriteParams() {
    return this->Join(parameters, " ");
}

std::string CodeGenPoCC::WriteMatrix(std::vector<std::vector<std::string>> matrix_) {
    std::string matrix{""};
    for (auto vec: matrix_) {
        matrix = matrix + "   " + this->Join(vec, "\t") + "\n";
    }
    return matrix;
}

std::string CodeGenPoCC::ConstructScatteringMatrix() {
    std::string matrix{""};
    // root | iterators | paramaters | constant
    int no_of_cols = 1 + curr_iterators.size() + this->GetParams() + 1;
    int no_of_rows = 2 * curr_iterators.size() + 1;

    matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";

    std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));

    matrix_[0][no_of_cols] = "## 0";

    int row{1};
    for (size_t i = 0; i < curr_iterators.size(); i++) {
        int child{-1};

        std::vector<std::string> row1(no_of_cols + 1, "0");
        auto found = schedule.find(curr_iterators[i]);
        if (found != schedule.end()) {
            child = found->second;
        }
        
        row1[i + 1] = "1";
        row1[no_of_cols] = "## " + curr_iterators[i];
        matrix_[row] = row1;
        row++;

        std::vector<std::string> row2(no_of_cols + 1, "0");
        row2[no_of_cols - 1] = std::to_string(child);
        row2[no_of_cols] = "## " + std::to_string(child);
        matrix_[row] = row2;
        row++;
    }
    
    matrix += this->WriteMatrix(matrix_);

    return matrix;
}



// FIXME: Will change based on the multidimensional array presentation
std::string CodeGenPoCC::ConstructReadWriteAccessMatrix() {
    std::string matrix{""};
    // vid | iterators | parameters | constant
    int no_of_cols = 1 + curr_iterators.size() + this->GetParams() + 1;
    int no_of_rows = read_write_variable.size();

    matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";
    
    std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));
    
    int row{0};
    for (auto rw_var: read_write_variable) {
        std::vector<std::string> row1(no_of_cols + 1, "0");
        std::string rw_expr{"## " + rw_var + "["};

        row1[0] = std::to_string(this->Index(rw_var, read_write_variable)); 
        std::unordered_map<std::string, std::string> iterator_coeff_dict_;
        auto found = iterator_coeff_dict.find(rw_var);
        if(found != iterator_coeff_dict.end()){
            iterator_coeff_dict_ = found->second;
        } else {
            rw_expr = rw_expr + "0]";
            row1[no_of_cols] = rw_expr;
            matrix_[row] = row1;
            row++;
            continue;
        }
        // FIXME: Tackle dimension here [i][j][k]. Do I need to do? HeteroCL seems to be flattening
        //        everythign.
        //        As of now only a * i + b * j can work
        std::vector<std::string> v;
        for (size_t i = 0; i < curr_iterators.size(); i++) {
            auto found = iterator_coeff_dict_.find(curr_iterators[i]);
            if (found != iterator_coeff_dict_.end()) {
                row1[i + 1] = found->second;
                v.push_back(found->second + "*" + curr_iterators[i]);
            }
        }
        rw_expr = rw_expr + this->Join(v, "+");
        v.clear();
        // NOTE: Now adding the constants
        auto foundc = constant_coeff_dict.find(rw_var);
        if (foundc != constant_coeff_dict.end()) {
            std::string constant = foundc->second;
            row1[no_of_cols - 1] = constant;
            rw_expr = rw_expr + "+" + constant + "]";
        } else {
            rw_expr = rw_expr + "]";
        }
        row1[no_of_cols] = rw_expr;
        matrix_[row] = row1;
        row++;
    }

    matrix += this->WriteMatrix(matrix_);

    return matrix;
}

std::string CodeGenPoCC::ConstructIterDomMatrix() {
    std::string matrix{""};
    // e/i | iterators | parameters | constant
    int no_of_cols = 1 + this->SizeIterBounds() + this->GetParams() + 1;
    int no_of_rows = 2 * this->SizeIterBounds();

    matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";
    
    // +1 for columns to pretty print the comment for iterator inequality
    std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));
    
    std::vector<iter_bounds> iterators_ = this->GetIterBounds();
    
    int row{0};
    for (size_t i = 0; i < iterators_.size(); i++) {

        std::string iterator = std::get<0>(iterators_[i].LB);
        std::string lb = std::get<1>(iterators_[i].LB);
        
        std::string ub = std::get<1>(iterators_[i].UB);
        
        std::vector<std::string> row1(no_of_cols + 1, "0");
        row1[0] = "1";

        row1[i + 1] = "1";
        row1[no_of_cols - 1] = lb;
        row1[no_of_cols] = "## " + iterator + " >= " + lb;
        matrix_[row] = row1;
        row++;

        std::vector<std::string> row2(no_of_cols + 1, "0");
        row2[0] = "1";

        row2[i + 1] = "-1";
        row2[no_of_cols - 1] = ub;
        row2[no_of_cols] = "## -" + iterator + " + " + ub + " >= 0";
        matrix_[row] = row2;
        row++;
    }

    matrix += this->WriteMatrix(matrix_);

    return matrix;
}

void CodeGenPoCC::ConstructSCoP(std::string statement) {
  pocc_stream << CreateDelimiter("=") << "Statement " << this->GetStmtNum() << "\n";
  pocc_stream << CreateDelimiter("-") << this->GetStmtNum() << ".1 Domain" << "\n";
  pocc_stream << "# Iteration domain\n";
  pocc_stream << "1\n";
  pocc_stream << iter_domain_matrix;
  pocc_stream << "\n";
  pocc_stream << CreateDelimiter("-") << this->GetStmtNum() << ".2 Scattering" << "\n";
  if (this->GetScatFuncStat()) {
      pocc_stream << "# Scattering function is provided\n";
      pocc_stream << "1\n";
      pocc_stream << "# Scattering function\n";
      pocc_stream << scattering_matrix;
      this->ResetScatFuncStat();
  } else {
      pocc_stream << "# Scattering function is not provided\n";
      pocc_stream << "0\n";
  }
  pocc_stream << "\n";
  pocc_stream << CreateDelimiter("-") << this->GetStmtNum() << ".3 Access" << "\n";
  if (this->GetAccessFuncStat()) {
      pocc_stream << "# Access informations are provided\n";
      pocc_stream << "1\n";
      pocc_stream << "# Read access informations\n";
      pocc_stream << read_access_matrix;
      pocc_stream << "\n";
      pocc_stream << "# Write access informations\n";
      pocc_stream << write_access_matrix;
      pocc_stream << "\n";
      this->ResetAccessFuncStat();
  } else {
      pocc_stream << "# Access informations are not provided\n";
      pocc_stream << "0\n";
  }
  pocc_stream << CreateDelimiter("-") << this->GetStmtNum() << ".4 Body" << "\n";
  pocc_stream << "# Statement body is provided\n";
  pocc_stream << "1\n";
  pocc_stream << "# Original iterator names\n";
  
  std::string iterator_names{""};
  size_t iterator_num = curr_iterators.size();
  for (size_t i = 0; i < iterator_num; i++) {
      if (i == iterator_num - 1){
          iterator_names = iterator_names + curr_iterators[i];
      } else {
          iterator_names = iterator_names + curr_iterators[i] + " ";
      }
  }
  pocc_stream << iterator_names <<"\n";
  pocc_stream << "# Statement body\n";
  pocc_stream << statement << ";\n";
  pocc_stream << "\n\n";
}

void CodeGenPoCC::WriteSCoP() {
  stream << "# [File generated by HeteroCL]\n";
  stream << "\n";
  stream << "SCoP\n";
  stream << "\n";
  stream << CreateDelimiter("=") << "Global\n";
  stream << "# Language\n";
  stream << "C\n";
  stream << "\n";
  stream << "# Context\n";
  stream << "\n"; // FIXME: Fill in the blanks after Louis-Noel responds
  stream << "\n";
  if(!this->IsParamEmpty()) {
      stream << "# Parameter names are provided\n";
      stream << this->GetParams() << "\n";
      stream << "# Parameter names\n";
      stream << this->WriteParams() << "\n";
      stream << "\n";
  } else {
      stream << "# Parameter names are not provided\n";
      stream << this->GetParams() << "\n";
      stream << "\n";
  }

  stream << "# Number of statements\n";
  stream << this->GetStmtNum() << "\n";
  stream << "\n";
  stream << pocc_stream.str();
  stream << CreateDelimiter("=") << "Options";
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

  // Write arguments
  // FIXME: Don't know why removing the for loop creates a Codegen error. Has to ask.
  for (size_t i = 0; i < f->args.size(); ++i) {
    Var v = f->args[i];
    std::string vid = AllocVarID(v.get());
  }
  
  int func_scope = this->BeginScope();
  this->PrintStmt(f->body);
  this->EndScope(func_scope);

  this->WriteSCoP();
}

std::string CodeGenPoCC::Finish() {
  return CodeGenC::Finish();
}

void CodeGenPoCC::BindThreadIndex(const IterVar& iv) {
  LOG(FATAL) << "PoCC doesn't support thread binding";
  return ;
}

void CodeGenPoCC::PrintType(Type t, std::ostream& os) {  // NOLINT(*)
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
  // FIXME: extent j = 18 + 2 * i + N
  std::string extent = PrintExpr(op->extent);
  std::string min = PrintExpr(op->min);
  std::string vid = AllocVarID(op->loop_var.get());

  iter_bounds ib;
  ib.LB = make_tuple(vid, min);
  ib.UB = make_tuple(vid, extent);

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
  }
}

void CodeGenPoCC::VisitStmt_(const Allocate* op) {
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

void CodeGenPoCC::VisitStmt_(const Store* op) {
  Type t = op->value.type();
  if (t.lanes() == 1) {
    iter_domain_matrix = this->ConstructIterDomMatrix();

    std::string value = this->PrintExpr(op->value);
    read_access_matrix =  this->ConstructReadWriteAccessMatrix();   
    /* The step is needed to ensure that the coefficients from the read
     * access are cleared */
    read_write_variable.clear();
    iterator_coeff_dict.clear();
    constant_coeff_dict.clear();

    std::string ref = this->GetBufferRef(t, op->buffer_var.get(), op->index);
    write_access_matrix = this->ConstructReadWriteAccessMatrix();
    /* This step is needed to ensure that the coefficients from the write
     * access are cleared */
    read_write_variable.clear();
    iterator_coeff_dict.clear();
    constant_coeff_dict.clear();

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

void CodeGenPoCC::VisitExpr_(const Load* op, std::ostream& os) {
  if (op->type.lanes() == 1) {
    std::string ref = this->GetBufferRef(op->type, op->buffer_var.get(), op->index);
    os << ref;
  }
}


// Print a reference expression to a buffer.
std::string CodeGenPoCC::GetBufferRef(Type t, const Variable* buffer, Expr index) {
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
      tokens = this->Split(expr_, '+');
      for (auto token: tokens) {
          // Matching constants
          if(this->IsNumeric(token)) {
              this->UpdateConstantCoeff(vid, token);
              continue;
          }
          // Matching iterators and parameters
          bool found = token.find("*") != std::string::npos;
          if (!found) {
              bool found_ = std::find(curr_iterators.begin(), curr_iterators.end(), token) != curr_iterators.end();
              if(!found_) {
                  this->UpdateParamCoeff(vid, token, "1");
              } else {
                  this->UpdateIterCoeff(vid, token, "1");
              }
          } else {
              std::vector<std::string> token_ = this->Split(token, '*');
              bool found_ = std::find(curr_iterators.begin(), curr_iterators.end(), token_[0]) != curr_iterators.end();
              if(!found_) {
                  this->UpdateParamCoeff(vid, token_[0], token_[1]);
              } else {
                  this->UpdateIterCoeff(vid, token_[0], token_[1]);
              }
          }
      }
    }
  }
  return os.str();
}

bool CodeGenPoCC::IsNumeric(std::string &s) {
    return !s.empty() && std::find_if(s.begin(),
            s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void CodeGenPoCC::UpdateIterCoeff(std::string vid, std::string iterator, std::string coeff) {
    auto found = iterator_coeff_dict.find(vid);
    if (found != iterator_coeff_dict.end()) {
        iterator_coeff_dict[vid].insert({iterator, coeff});
    } else {
        std::unordered_map<std::string, std::string> iter_coeff = {{iterator, coeff}};
        iterator_coeff_dict.insert({vid, iter_coeff});
    }
}

void CodeGenPoCC::UpdateParamCoeff(std::string vid, std::string parameter, std::string coeff) {
    auto found = parameter_coeff_dict.find(vid);
    if (found != parameter_coeff_dict.end()) {
        parameter_coeff_dict[vid].insert({parameter, coeff});
    } else {
        std::unordered_map<std::string, std::string> param_coeff = {{parameter, coeff}};
        parameter_coeff_dict.insert({vid, param_coeff});
    }
}

// NOTE:Every index fo ran array access can possibly have at most one constant
//      However, in unlikely case there are multiple constants, we can augment 
//      this function to handle that.
void CodeGenPoCC::UpdateConstantCoeff(std::string vid, std::string constant) {
    constant_coeff_dict.insert({vid, constant});
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

inline void PrintConst(const IntImm* op, std::ostream& os, CodeGenPoCC* p) { // NOLINT(*)
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

inline void PrintConst(const UIntImm* op, std::ostream& os, CodeGenPoCC* p) { // NOLINT(*)
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

inline void PrintConst(const FloatImm* op, std::ostream& os, CodeGenPoCC* p) { // NOLINT(*)
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

void CodeGenPoCC::VisitExpr_(const IntImm *op, std::ostream& os) {  // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenPoCC::VisitExpr_(const UIntImm *op, std::ostream& os) {  // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenPoCC::VisitExpr_(const FloatImm *op, std::ostream& os) { // NOLINT(*)
  PrintConst(op, os, this);
}

void CodeGenPoCC::VisitExpr_(const StringImm *op, std::ostream& os) { // NOLINT(*)
  os << "\"" << op->value << "\"";
}

}  // namespace codegen
}  // namespace TVM
