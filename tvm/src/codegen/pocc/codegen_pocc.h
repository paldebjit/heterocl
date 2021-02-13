/*!
 *  Copyright (c) 2017 by Contributors
 * \file codegen_opencl.h
 * \brief Generate OpenCL device code.
 */
#ifndef TVM_CODEGEN_CODEGEN_MERLINC_H_
#define TVM_CODEGEN_CODEGEN_MERLINC_H_

#include <tvm/codegen.h>
#include <tvm/packed_func_ext.h>
#include <string>
#include <queue>
#include <tuple>
#include "./codeanalys_pocc.h"
#include "../codegen_c.h"

namespace TVM {
namespace codegen {

struct iter_bounds {
    std::tuple<std::string, std::string> LB;
    std::tuple<std::string, std::string> UB;
};

class CodeGenPoCC final : public CodeGenC {
 public:
  CodeGenPoCC();
  void AddFunction(LoweredFunc f, str2tupleMap<std::string, Type> map_arg_type);
  std::string Finish();

  // override print thread tag.
  void InitFuncState(LoweredFunc f) final;
  void BindThreadIndex(const IterVar& iv) final;  // NOLINT(*)
  void PrintStorageScope(const std::string& scope, std::ostream& os) final; // NOLINT(*)
  void PrintStorageSync(const Call* op) final;  // NOLINT(*)
  void PrintType(Type t, std::ostream& os) final; // NOLINT(*)
  void PrintVecStore(const Variable* buffer,
                     Type t, Expr base,
                     const std::string& value) final;  // NOLINT(*)
  // the address of load/store
  void PrintVecAddr(const Variable* buffer, Type t,
                    Expr base, std::ostream& os);  // NOLINT(*)
  // overload visitor for Expression
  void VisitExpr_(const Broadcast* op, std::ostream& os) final; // NOLINT(*)
  void VisitExpr_(const Load* op, std::ostream& os) final; // NOLINT(*)
  void VisitExpr_(const IntImm* op, std::ostream& os) final;  // NOLINT(*)
  void VisitExpr_(const UIntImm* op, std::ostream& os) final;  // NOLINT(*)
  void VisitExpr_(const FloatImm* op, std::ostream& os) final;  // NOLINT(*)
  void VisitExpr_(const StringImm* op, std::ostream& os) final;  // NOLINT(*)
    
  // overload visitor for Statement
  void VisitStmt_(const For* op) override;
  void VisitStmt_(const LetStmt* op) final; // NOLINT(*)
  void VisitStmt_(const IfThenElse* op) final; // NOLINT(*)
  void VisitStmt_(const Store* op) final; // NOLINT(*)
  void VisitStmt_(const Allocate* op) final; // NOLINT(*)
  // overload buffer parsing
  std::string GetBufferRef(Type t, const Variable* buffer, Expr index) final; // NOLINT(*)
  // functions to manipulate SCoP matrices
  std::string CreateDelimiter(std::string symbol); // NOLINT(*)
  void IncrStmtNum(); // NOLINT(*)
  int GetStmtNum(); // NOLINT(*)
  void SetScatFuncStat(); // NOLINT(*)
  bool GetScatFuncStat(); // NOLINT(*)
  void SetAccessFuncStat(); // NOLINT(*)
  bool GetAccessFuncStat(); // NOLINT(*)
  int GetTotalNumStmts(); // NOLINT(*)
  void PushIterBounds(iter_bounds ib);
  void PopIterBounds();
  void InsertParams(std::string); // NOLINT(*)
  int GetParams(); // NOLINT(*)
  bool IsParamEmpty();
  std::string WriteParams(); 

  // Add column
  // Add rows

  // functions to readout the matrices
  std::string WriteIterDomMatrix();

 private:
  /*! \brief SCoP matrices */
  std::vector<std::string> parameters;
  std::vector<iter_bounds> iterators;                  // e/i | iterators | params | const
  //std::vector<std::vector<int>> scattering;          // root | iterators | params | const
  //std::vector<std::vector<int>> read_access;         // array ident | iterators | params | const
  //std::vector<std::vector<int>> write_access;       // array ident | iterators | params | const

  std::queue<std::string> statements;
  std::queue<std::string> iteration_domain;
  int no_of_stmt{0};
  bool scat_func_stmt{false};
  bool access_func_stmt{false};
};

}  // namespace codegen
}  // namespace TVM

#endif  // TVM_CODEGEN_CODEGEN_MERLINC_H_
