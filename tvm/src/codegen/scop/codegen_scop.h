/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_scop.h
 * \brief Generate SCoP code.
 */
#ifndef TVM_CODEGEN_CODEGEN_SCOP_H_
#define TVM_CODEGEN_CODEGEN_SCOP_H_

#include <tvm/codegen.h>
#include <tvm/packed_func_ext.h>
#include <iostream>
#include <cstdio>
#include <array>
#include <memory>
#include <fstream>
#include <string>
#include <queue>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <regex>
#include <iterator>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "./codeanalys_scop.h"
#include "../codegen_c.h"

namespace TVM {
namespace codegen {

struct iter_bounds {
    /* The arrangement is the following
     * <VID      <SYMBOL_NAME     SYMBOL_COEFFICIENT>>
     * VID \in {iterator_name}
     * SYMBOL_NAME \in {_CONSTANT_, parameter_name, iterator_name (outer)}
     * SYMBOL_COEFFICIENT \in [+|-]?[0-9]+ 
     */
    std::tuple<std::string, std::unordered_map<std::string, std::string>> LB;
    std::tuple<std::string, std::unordered_map<std::string, std::string>> UB;
};

class CodeGenSCoP final : public CodeGenC {
 public:
  CodeGenSCoP();
  void AddFunction(LoweredFunc f, str2tupleMap<std::string, Type> map_arg_type);
  std::string Finish();

  // override print thread tag.
  void InitFuncState(LoweredFunc f) final;
  void BindThreadIndex(const IterVar& iv) final;  // NOLINT(*)
  void PrintStorageScope(const std::string& scope, std::ostream& os) final; // NOLINT(*)
  void PrintStorageSync(const Call* op) final;  // NOLINT(*)
  //void PrintType(Type t, std::ostream& os) final; // NOLINT(*)
  void PrintVecStore(const Variable* buffer, Type t, Expr base, const std::string& value) final;  // NOLINT(*)

  // the address of load/store
  void PrintVecAddr(const Variable* buffer, Type t,  Expr base, std::ostream& os);  // NOLINT(*)

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
  void IncrStmtNum(int phase); // NOLINT(*)
  int GetStmtNum(int phase); // NOLINT(*)

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
  void UpdateIterCoefficient(std::string s, std::string coeff); // NOLINT(*)
  int SizeIterBounds(); // NOLINT(*)
  std::vector<iter_bounds> GetIterBounds(); // NOLINT(*)

  // To keep track of array access index coefficientis per read/write array
  // For scalar, it is treated as a vector of length 1
  void UpdateReadWriteAccessCoefficient(std::string vid); // NOLINT(*)
  void UpdateReadWriteAccessCoefficient(std::string s, std::string coeff); // NOLINT(*)

  // To tackle any parameters.
  void InsertParams(std::string); // NOLINT(*)
  int GetParams(); // NOLINT(*)
  bool IsParamEmpty();  // NOLINT(*)
  std::string WriteParams();    // NOLINT(*)

  // Functions to write SCoP component matrices
  std::string ConstructContextMatrix(); // NOLINT(*)
  std::string ConstructIterDomMatrix(); // NOLINT(*)
  std::string ConstructReadWriteAccessMatrix(); // NOLINT(*)
  std::string ConstructScatteringMatrix(); // NOLINT(*)
  std::string WriteMatrix(std::vector<std::vector<std::string>> matrix_);

  // Function to construct and write the SCoP
  void ConstructSCoP(std::string statement, int phase); // NOLINT(*)
  void AssembleSCoP(int phase); // NOLINT(*)

  // Generic string manipulation functions
  std::vector<std::string> Split(const std::string s, char delim); // NOLINT(*)
  std::vector<std::string> Split(const std::string s, std::string delim); // NOLINT(*)
  std::string Strip(const std::string &s); // NOLINT(*)
  std::string Join(std::vector<std::string> v, std::string delim);
  int Index(std::string vid, std::vector<std::string> vmap); // NOLINT(*)
  bool IsNumeric(std::string &s); // NOLINT(*)

  // Functions for 2-step verification and PLuTO-based optimization
  std::string ExecCmd(const char* cmd); // NOLINT(*)
  void Verify(); // NOLINT(*)
  void CheckEnv(); // NOLINT(*)
  void WriteSCoP(); // NOLINT(*)
  void VerifySchedule(); // NOLINT(*)
  bool ParseVerifSchResult(); // NOLINT(*)
  void VerifyGenCode(); // NOLINT(*)
  //void ParseVerifGenCodeResult(); // NOLINT(*)
  void GenHintSch(); // NOLINT(*)

 private:
  /*! \brief SCoP specific stream to store all polyhedral model/SCoP info temporaily
   * and redirect to base class stream at the end. This is to ensure we have 
   * necessary summary information such number of statements etc before wrting
   * other artifacts in the SCoP file.*/
  std::ostringstream scop_stream_legality;
  std::ostringstream scop_stream_codegen;

  std::ostringstream scat_stream_legality;
  std::ostringstream scat_stream_codegen;
  /*! \brief To store a unique numeric mapping per variable (read/write variable).
   * The index of the variable in the vector is its unique numeric map. This is 
   * needed as SCoP read/write access matrices need an unique number per read/write variable.*/
  std::vector<std::string> vid_map;
  /*! \brief Variable to store user-defined parameters whose value is NOT known at 
   * compile time.*/
  std::vector<std::string> parameters;

  // FIXME: How can I tackle multiple dimesnion of the array in this same structure?

  /*! \brief To store the parameter coefficicents in the indices of a variable per 
   * read/write access temporarily. The format is the following:
   * <Read/Write_Variable_Name, <Parameter/Iterator/Constant_Name, Parameter_Coefficicent>>. 
   * Reused across different read/write access.*/
  std::unordered_multimap<std::string, std::unordered_map<std::string, std::string>> read_write_coeff_map;
  std::unordered_map<std::string, std::string> read_write_coeff;
  /*! \brief A reusable string vector to store read/write variable per statement.*/
  std::vector<std::string> read_write_variable;
  /*! \brief To store the (vid, lower bound) and (vid, upper bound) for each of the iterators
   * per statement. Reusable.*/
  std::vector<iter_bounds> iterators;
  /*! \brief Storing the iterator sequence on the fly per statement.*/
  std::vector<std::string> curr_iterators;
  /*! \brief Storing the intra-loop schedule per statement in 2d + 1 format where `d' is the 
   * loop nest depth of a given statement.*/ 
  std::unordered_map<std::string, int> schedule;
  /*! \brief Storing the inter-statement schedule
   * */
  int Schedule{0};
  /*! \brief To store the iterator/parameter/constant coefficients in the extent
   * per iterator temporarily. The format is the following:
   * <Parameter/Iterator/Constant_Name, Parameter_Coefficicent>.
   * Reused across different read/write access.*/
  std::unordered_map<std::string, std::string> min_extent_coeff_map;
    
  /*! \brief Total number of statements found in the IR for which SCoP has been written.*/
  int no_of_stmt_1{0};
  int no_of_stmt_2{0};
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

  /*! \brief Storing pocc related info */
  std::string POCC_HOME{""};
};

}  // namespace codegen
}  // namespace TVM

#endif  // TVM_CODEGEN_CODEGEN_SCOP_H_
