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
#include <sstream>
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
  void ResetAccessFuncStat(); // NOLINT(*)
  bool GetAccessFuncStat(); // NOLINT(*)

  int GetTotalNumStmts(); // NOLINT(*)

  void PushIterBounds(iter_bounds ib);  // NOLINT(*)
  void PopIterBounds(); // NOLINT(*)
  int SizeIterBounds(); // NOLINT(*)

  void UpdateIterCoeff(std::string vid, std::string iterator, std::string coeff); // NOLINT(*)
  int SizeIterCoeff(); // NOLINT(*)

  std::vector<iter_bounds> GetIterBounds();

  void InsertParams(std::string); // NOLINT(*)
  int GetParams(); // NOLINT(*)
  bool IsParamEmpty();  // NOLINT(*)
  std::string WriteParams();    // NOLINT(*)

  // functions to readout the matrices
  std::string WriteIterDomMatrix(); // NOLINT(*)
  std::string WriteReadWriteAccessMatrix(); // NOLINT(*)
  std::string WriteScatteringMatrix(); // NOLINT(*)

  // generic functions
  std::vector<std::string> Split(const std::string &s, char delim); // NOLINT(*)
  std::string Strip(const std::string &s); // NOLINT(*)
  int Index(std::string vid, std::vector<std::string>);
  void MapVid(std::string vid); // NOLINT(*)

 private:
  /*! \brief SCoP matrices */
  std::vector<std::string> vid_map;
  // First key is vid, second key is iterator and then the value is the value of the iterator
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iterator_coeff_dict;
  std::vector<std::string> read_write_variable;
  std::vector<std::string> parameters;
  std::vector<iter_bounds> iterators;
  std::vector<std::string> iterator_sequence;
  std::unordered_map<std::string, int> schedule;
  std::vector<std::string> curr_iterators; // FIXME: possibly same as iterator_sequence

  std::queue<std::string> statements;
  int no_of_stmt{0};
  bool scat_func_stmt{false};
  bool access_func_stmt{false};
};

}  // namespace codegen
}  // namespace TVM

#endif  // TVM_CODEGEN_CODEGEN_MERLINC_H_
