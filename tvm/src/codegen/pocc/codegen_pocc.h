/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_pocc.h
 * \brief Generate PoCC SCoP code.
 */
#ifndef TVM_CODEGEN_CODEGEN_POCC_H_
#define TVM_CODEGEN_CODEGEN_POCC_H_

#include <tvm/codegen.h>
#include <tvm/packed_func_ext.h>
#include <string>
#include <queue>
#include <tuple>
#include <sstream>
#include <iomanip>
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
  
  // functions to manipulate SCoP matrix components
  std::string CreateDelimiter(std::string symbol); // NOLINT(*)

  // To keep track of number of statements in the generated code
  void IncrStmtNum(); // NOLINT(*)
  int GetStmtNum(); // NOLINT(*)

  // To keep track of Scattering Function per statement (FIXME: Need to make more intelligent)
  void SetScatFuncStat(); // NOLINT(*)
  void ResetScatFuncStat(); // NOLINT(*)
  bool GetScatFuncStat(); // NOLINT(*)

  // To keep track of Read/Write access Function per statement (FIXME: Need to make more intelligent)
  void SetAccessFuncStat(); // NOLINT(*)
  void ResetAccessFuncStat(); // NOLINT(*)
  bool GetAccessFuncStat(); // NOLINT(*)
  void MapVid(std::string vid); // NOLINT(*)

  // To keep track of iteration bounds (vid, lower bound, upper bound) 
  // per loop nest (scope wise)
  void PushIterBounds(iter_bounds ib);  // NOLINT(*)
  void PopIterBounds(); // NOLINT(*)
  int SizeIterBounds(); // NOLINT(*)
  std::vector<iter_bounds> GetIterBounds(); // NOLINT(*)

  // To keep track of array access index coefficientis per read/write array
  // For scalar, it is treated as a vector of length 1
  void UpdateIterCoeff(std::string vid, std::string iterator, std::string coeff); // NOLINT(*)
  int SizeIterCoeff(); // NOLINT(*)

  // To tackle any parameters.
  void InsertParams(std::string); // NOLINT(*)
  int GetParams(); // NOLINT(*)
  bool IsParamEmpty();  // NOLINT(*)
  std::string WriteParams();    // NOLINT(*)

  // Functions to write SCoP component matrices
  std::string ConstructIterDomMatrix(); // NOLINT(*)
  std::string ConstructReadWriteAccessMatrix(); // NOLINT(*)
  std::string ConstructScatteringMatrix(); // NOLINT(*)

  std::string WriteMatrix(std::vector<std::vector<std::string>> matrix_);

  // Function to construct the SCoP on-the-fly
  void ConstructSCoP(std::string statement); // NOLINT(*)
  void WriteSCoP(); // NOLINT(*)

  // Generic string manipulation functions
  std::vector<std::string> Split(const std::string &s, char delim); // NOLINT(*)
  std::string Strip(const std::string &s); // NOLINT(*)
  int Index(std::string vid, std::vector<std::string>);

 private:
  /*! \brief PoCC specific stream to store all polyhedral model/SCoP info temporaily
   * and redirect to base class stream at the end. This is to ensure we have 
   * necessary summary information such number of statements etc before wrting
   * other artifacts in the SCoP file.*/
  std::ostringstream pocc_stream;
  /*! \brief To store a unique numeric mapping per variable (read/write variable).
   * The index of the variable in the vector is its unique numeric map. This is 
   * needed as SCoP read/write access matrices need an unique number per read/write variable.*/
  std::vector<std::string> vid_map;
  /*! \brief Variable to store user-defined parameters whose value is NOT known at 
   * compile time.*/
  std::vector<std::string> parameters;
  /*! \brief To store the iterator coefficicents in the indices of a variable per 
   * read/write access temporarily. The format is the following:
   * <Read/Write_Variable_Name, <Iterator_Name, Iterator_Coefficicent>>. Reused across
   * different read/write access.*/
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iterator_coeff_dict;
  /*! \brief A reusable string vector to store read/write variable per statement of the form 
   * ref = value; */
  std::vector<std::string> read_write_variable;
  /*! \brief To store the (vid, lower bound) and (vid, upper bound) for each of the iterators
   * per statement. Reusable.*/
  std::vector<iter_bounds> iterators;
  /*! \brief Storing the iterator sequence on the fly per statement.*/
  std::vector<std::string> curr_iterators;
  /*! \brief Storing the schedule per statement in 2d + 1 format where `d' is the 
   * loop nest depth of a given statement.*/ 
  std::unordered_map<std::string, int> schedule;
    
  /*! \brief Total number of statements found in the IR for which SCoP has been written.*/
  int no_of_stmt{0};
  /*! \brief To store scattering function status per statement. */
  bool scat_func_stmt{false};
  /*! \brief To store read/write access function status per statement. */
  bool access_func_stmt{false};

  /*! \brief Temporarily store the iteration domain matrix as a string per statement.*/
  std::string iter_domain_matrix;
  /*! \brief Temporarily store the read access matrix as a string per statement.*/
  std::string read_access_matrix;
  /*! \brief Temporarily store the write access matrix as a string per statement.*/
  std::string write_access_matrix;
  /*! \brief Temporarily store the scattering domain matrix as a string per statement.*/
  std::string scattering_matrix;
};

}  // namespace codegen
}  // namespace TVM

#endif  // TVM_CODEGEN_CODEGEN_POCC_H_
