/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_scop_util.cc
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

std::string CodeGenSCoP::CreateDelimiter(std::string symbol) {
    std::string delimiter;
    delimiter = "# ";
    for(int i = 0; i < 47; i ++) {
        delimiter = delimiter + symbol;
    }
    delimiter = delimiter + " ";
    return delimiter;
}

void CodeGenSCoP::IncrStmtNum(int phase) {
    no_of_stmt_2 = no_of_stmt_2 + 1;
    if (phase == 1) {
        no_of_stmt_1 = no_of_stmt_1 + 1;
    }
}

int CodeGenSCoP::GetStmtNum(int phase) {
    if (phase == 1) {
        return no_of_stmt_1;
    }
    return no_of_stmt_2;
}

void CodeGenSCoP::ResetAccessFuncStat() {
    access_func_stmt = false;
}

void CodeGenSCoP::SetAccessFuncStat() {
    access_func_stmt = true;
}

bool CodeGenSCoP::GetAccessFuncStat() {
    return access_func_stmt;
}

void CodeGenSCoP::SetScatFuncStat() {
    scat_func_stmt = true;
}

void CodeGenSCoP::ResetScatFuncStat() {
    scat_func_stmt = false;
}

bool CodeGenSCoP::GetScatFuncStat() {
    return scat_func_stmt;
}

void CodeGenSCoP::InsertParams(std::string param) {
    parameters.push_back(param);
}

int CodeGenSCoP::GetParams() {
    return parameters.size();
}

bool CodeGenSCoP::IsParamEmpty() {
    return parameters.empty();
}

void CodeGenSCoP::PushIterBounds(iter_bounds ib) {
    iterators.push_back(ib);
}

void CodeGenSCoP::PopIterBounds() {
    iterators.pop_back();
}

int CodeGenSCoP::SizeIterBounds() {
    return iterators.size();
}

std::vector<iter_bounds> CodeGenSCoP::GetIterBounds() {
    return iterators;
}

int CodeGenSCoP::Index(std::string vid, std::vector<std::string> vmap) {
    int index{-1};
    auto it = std::find(vmap.begin(), vmap.end(), vid);
    if (it != vmap.end()) {
        index = it - vmap.begin();
    }
    return index;
}

std::vector<std::string> CodeGenSCoP::Split(const std::string s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    bool has_seen = false;

    while (getline(ss, item, delim)) {
        if (item.empty()) {
            has_seen = true;
            continue;
        }
        if ((delim == '-') & has_seen) {
            result.push_back("-" + this->Strip(item));
        } else if ((delim == '-') & !has_seen) { 
            result.push_back(this->Strip(item));
            has_seen = true;
        } else {
            result.push_back(this->Strip(item));
        }
    }
    return result;
}

std::vector<std::string> CodeGenSCoP::Split(const std::string s, std::string delim) {
    std::vector<std::string> result;

    result.push_back(s);

    for (auto delim_: delim) {
        std::vector<std::string> result_;
        for (auto s_: result) {
            std::vector<std::string> result__ = this->Split(s_, delim_);
            result_.insert(
                    result_.end(),
                    std::make_move_iterator(result__.begin()),
                    std::make_move_iterator(result__.end())
                    );
            result__.clear();
        }
        result = result_;
        result_.clear();
    }

    return result;
}

std::string CodeGenSCoP::Strip(const std::string &s) {
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

std::string CodeGenSCoP::Join(std::vector<std::string> v, std::string delim) {
    std::string delimo;
    std::string s{""};
    for (size_t i = 0; i < v.size(); i++) {
        // Get the next element
        std::string curr = v[i];

        // Join the first element as-is
        if (i == 0) {
            s = s + curr;
        } else {
            // store the delimiter temporarily
            delimo = delim;
            // check the sign of the element and delimiter
            // - + ---> -
            // - - ---> +
            if (curr[0] == '-' && delim == " + ") {
                delim = " - ";
            } else {
                if (curr[0] == '-' && delim == " - ") {
                    delim = " + ";
                }
            }

            // If the original delimiter == + and sign of element == -, then
            // skip the first char of the element
            if (curr[0] == '-' && delimo == " + ") {
                curr = curr.substr(1);
            }
            
            // Construct the joint string
            s = s + delim + curr;
            // Restore the original delimiter
            delim = delimo;
        }
    }
    return s;
}

void CodeGenSCoP::MapVid(std::string vid) {
    read_write_variable.push_back(vid);
    if (std::find(vid_map.begin(), vid_map.end(), vid) == vid_map.end()) {
        vid_map.push_back(vid);
    }
}

std::string CodeGenSCoP::WriteParams() {
    return this->Join(parameters, " ");
}

std::string CodeGenSCoP::WriteMatrix(std::vector<std::vector<std::string>> matrix_) {
    std::string matrix{""};
    for (auto vec: matrix_) {
        matrix = matrix + "   " + this->Join(vec, "\t") + "\n";
    }
    return matrix;
}

std::string CodeGenSCoP::ConstructScatteringMatrix() {
    std::string matrix{""};
    // root | iterators | paramaters | constant
    int no_of_cols = 1 + curr_iterators.size() + this->GetParams() + 1;
    int no_of_rows = 2 * curr_iterators.size() + 1;

    matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";

    std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));

    matrix_[0][no_of_cols - 1] = std::to_string(Schedule);
    matrix_[0][no_of_cols] = "## " + std::to_string(Schedule);

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
std::string CodeGenSCoP::ConstructReadWriteAccessMatrix() {
    std::string matrix{""};
    // vid | iterators | parameters | constant
    int no_of_cols = 1 + curr_iterators.size() + this->GetParams() + 1;
    //int no_of_rows = read_write_variable.size();
    int no_of_rows = read_write_coeff_map.size();

    matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";
    
    std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));
    
    int row{0};
    for (auto rw_var: read_write_variable) {
        auto found = read_write_coeff_map.find(rw_var);
        if(found != read_write_coeff_map.end()) {
            size_t count = read_write_coeff_map.count(rw_var);
    
            for (size_t i = 0; i < count; i++) {
                std::vector<std::string> row1(no_of_cols + 1, "0");
                std::string rw_expr{"## " + rw_var + "["};
    
                row1[0] = std::to_string(this->Index(rw_var, vid_map)); 
    
                auto found = read_write_coeff_map.find(rw_var);
                std::unordered_map<std::string, std::string> coeff_map;
                // Checking if Read/Write access is a scalar access or array element access
                coeff_map = found->second;
                read_write_coeff_map.erase(found);
                if (coeff_map.empty()) {
                    rw_expr = rw_expr + "0]";
                    row1[no_of_cols] = rw_expr;
                    matrix_[row] = row1;
                    row++;
                    continue;
                }
                // FIXME: Tackle dimension here [i][j][k]. i
                //        Do I need to do? HeteroCL seems to be flattening everything.
                //        As of now only a * i + b * j can work
                std::vector<std::string> v;
                // Tackling iterators
                for (size_t i = 0; i < curr_iterators.size(); i++) {
                    auto found = coeff_map.find(curr_iterators[i]);
                    if (found != coeff_map.end()) {
                        row1[i + 1] = found->second;
                        v.push_back(found->second + "*" + curr_iterators[i]);
                    }
                }
                // Tackling parameters
                for (size_t i = 0; i < parameters.size(); i++) {
                    auto found = coeff_map.find(parameters[i]);
                    if (found != coeff_map.end()) {
                        row1[i + curr_iterators.size() + 1] = found->second;
                        v.push_back(found->second + "*" + curr_iterators[i]);
                    }
                }
                // Tackling constants
                auto foundc = coeff_map.find("_CONSTANT_");
                if (foundc != coeff_map.end()) {
                    row1[no_of_cols - 1] = foundc->second;
                    v.push_back(foundc->second);
                }
                
                rw_expr = rw_expr + this->Join(v, " + ") + "]";
                v.clear();
    
                row1[no_of_cols] = rw_expr;
                matrix_[row] = row1;
                row++;
            }
        }
    }

    matrix += this->WriteMatrix(matrix_);

    return matrix;
}

std::string CodeGenSCoP::ConstructIterDomMatrix() {
    // TODO: i) Change how the row is calculated
    //      ii) Incorporate how the == can be included

    std::string matrix{""};
    // e/i | iterators | parameters | constant
    int no_of_cols = 1 + this->SizeIterBounds() + this->GetParams() + 1;
    //int no_of_rows = 2 * this->SizeIterBounds();

    //matrix +=  std::to_string(no_of_rows) + " " + std::to_string(no_of_cols) + "\n";
    
    // +1 for columns to pretty print the comment for iterator inequality
    //std::vector<std::vector<std::string>> matrix_(no_of_rows, std::vector<std::string>(no_of_cols + 1, "0"));
    std::vector<std::vector<std::string>> matrix_;
    
    std::vector<iter_bounds> iterators_ = this->GetIterBounds();
    /* <VID      <SYMBOL_NAME     SYMBOL_COEFFICIENT>> */
    int row{0};
    for (size_t i = 0; i < iterators_.size(); i++) {

        std::vector<std::string> v;

        std::string iterator = std::get<0>(iterators_[i].LB);
        std::unordered_map<std::string, std::string> lb = std::get<1>(iterators_[i].LB);
        std::unordered_map<std::string, std::string> ub = std::get<1>(iterators_[i].UB);

        /* Tackling equality case where LB == UB and hence only need one equality (_CONSTANT_) */
        auto foundlb = lb.find("_CONSTANT_");
        auto foundub = ub.find("_CONSTANT_");
        std::string clb, cub;
        if (foundlb != lb.end()) {
            clb = foundlb->second;
        }
        if (foundub != ub.end()) {
            cub = foundub->second;
        }
        if (clb == cub) {
            std::string eq_expr{"## " + iterator + " == " + clb};
            std::vector<std::string> row1(no_of_cols + 1, "0");

            row1[0] = "0";
            row1[i + 1] = "1";
            row1[no_of_cols - 1] = clb;
            row1[no_of_cols] = eq_expr;
            matrix_.push_back(row1);
            row++;
            continue;
        }

        
        /* Tackling the lower bound */
        std::string lb_expr{"## " + iterator + " >= "};
        std::vector<std::string> row1(no_of_cols + 1, "0");
        row1[0] = "1";

        row1[i + 1] = "1";
        
        // Tackling outermost iterators
        for (size_t j = 0; j < curr_iterators.size(); j++) {
            auto foundilb = lb.find(curr_iterators[j]);
            if (foundilb != lb.end()) {
                row1[j + 1] = foundilb->second;
                v.push_back(foundilb->second + "*" + curr_iterators[j]);
            }
        }
        
        // Tackling parameters
        for (size_t j = 0; j < parameters.size(); j++) {
            auto foundplb = lb.find(parameters[j]);
            if (foundplb != lb.end()) {
                row1[j + curr_iterators.size() + 1] = foundplb->second;
                v.push_back(foundplb->second + "*" + parameters[j]);
            }
        }

        // Tackling constants
        auto foundclb = lb.find("_CONSTANT_");
        if (foundclb != lb.end()) {
            row1[no_of_cols - 1] = foundclb->second;
            v.push_back(foundclb->second);
        }

        lb_expr = lb_expr + this->Join(v, " + ");
        v.clear();

        row1[no_of_cols] = lb_expr;
        //matrix_[row] = row1;
        matrix_.push_back(row1);
        row++;

        /* Tackling the upper bound */
        std::string ub_expr{"## -" + iterator};
        v.push_back(ub_expr);
        std::vector<std::string> row2(no_of_cols + 1, "0");
        row2[0] = "1";

        row2[i + 1] = "-1";
        // Tackling outermost iterators
        for (size_t j = 0; j < curr_iterators.size(); j++) {
            auto foundiub = ub.find(curr_iterators[j]);
            if (foundiub != ub.end()) {
                row2[j + 1] = foundiub->second;
                v.push_back(foundiub->second + "*" + curr_iterators[j]);
            }
        }
        
        // Tackling parameters
        for (size_t j = 0; j < parameters.size(); j++) {
            auto foundpub = ub.find(parameters[j]);
            if (foundpub != ub.end()) {
                row2[j + curr_iterators.size() + 1] = foundpub->second;
                v.push_back(foundpub->second + "*" + parameters[j]);
            }
        }

        // Tackling constants
        auto foundcub = ub.find("_CONSTANT_");
        if (foundcub != ub.end()) {
            row2[no_of_cols - 1] = foundcub->second;
            v.push_back(foundcub->second);
        }
        
        std::string s = this->Join(v, " + ");
        ub_expr = s + " >= 0";
        v.clear();

        row2[no_of_cols] = ub_expr;
        //matrix_[row] = row2;
        matrix_.push_back(row2);
        row++;
    }

    matrix +=  std::to_string(row) + " " + std::to_string(no_of_cols) + "\n";
    matrix += this->WriteMatrix(matrix_);

    return matrix;
}

void CodeGenSCoP::ConstructSCoP(std::string statement, int phase) {
  scop_stream_codegen << CreateDelimiter("=") << "Statement " << this->GetStmtNum(2) << "\n";
  scop_stream_codegen << CreateDelimiter("-") << this->GetStmtNum(2) << ".1 Domain" << "\n";
  scop_stream_codegen << "# Iteration domain\n";
  scop_stream_codegen << "1\n";
  scop_stream_codegen << iter_domain_matrix;
  scop_stream_codegen << "\n";
  scop_stream_codegen << CreateDelimiter("-") << this->GetStmtNum(2) << ".2 Scattering" << "\n";
  if (this->GetScatFuncStat()) {
      scop_stream_codegen << "# Scattering function is provided\n";
      scop_stream_codegen << "1\n";
      scop_stream_codegen << "# Scattering function\n";
      scop_stream_codegen << scattering_matrix;
      scat_stream_codegen << scattering_matrix;
  } else {
      scop_stream_codegen << "# Scattering function is not provided\n";
      scop_stream_codegen << "0\n";
  }
  scop_stream_codegen << "\n";
  scop_stream_codegen << CreateDelimiter("-") << this->GetStmtNum(2) << ".3 Access" << "\n";
  if (this->GetAccessFuncStat()) {
      scop_stream_codegen << "# Access informations are provided\n";
      scop_stream_codegen << "1\n";
      scop_stream_codegen << "# Read access informations\n";
      scop_stream_codegen << read_access_matrix;
      //scop_stream_codegen << "\n";
      scop_stream_codegen << "# Write access informations\n";
      scop_stream_codegen << write_access_matrix;
      scop_stream_codegen << "\n";
  } else {
      scop_stream_codegen << "# Access informations are not provided\n";
      scop_stream_codegen << "0\n";
  }
  scop_stream_codegen << CreateDelimiter("-") << this->GetStmtNum(2) << ".4 Body" << "\n";
  scop_stream_codegen << "# Statement body is provided\n";
  scop_stream_codegen << "1\n";
  scop_stream_codegen << "# Original iterator names\n";
  
  std::string iterator_names{""};
  size_t iterator_num = curr_iterators.size();
  for (size_t i = 0; i < iterator_num; i++) {
      if (i == iterator_num - 1){
          iterator_names = iterator_names + curr_iterators[i];
      } else {
          iterator_names = iterator_names + curr_iterators[i] + " ";
      }
  }
  scop_stream_codegen << iterator_names <<"\n";
  scop_stream_codegen << "# Statement body\n";
  scop_stream_codegen << statement << ";\n";
  scop_stream_codegen << "\n\n";

  if (phase == 1) {
	  scop_stream_legality << CreateDelimiter("=") << "Statement " << this->GetStmtNum(1) << "\n";
	  scop_stream_legality << CreateDelimiter("-") << this->GetStmtNum(1) << ".1 Domain" << "\n";
	  scop_stream_legality << "# Iteration domain\n";
	  scop_stream_legality << "1\n";
	  scop_stream_legality << iter_domain_matrix;
	  scop_stream_legality << "\n";
	  scop_stream_legality << CreateDelimiter("-") << this->GetStmtNum(1) << ".2 Scattering" << "\n";
	  if (this->GetScatFuncStat()) {
	      scop_stream_legality << "# Scattering function is provided\n";
	      scop_stream_legality << "1\n";
	      scop_stream_legality << "# Scattering function\n";
	      scop_stream_legality << scattering_matrix;
	      scat_stream_legality << scattering_matrix;
	  } else {
	      scop_stream_legality << "# Scattering function is not provided\n";
	      scop_stream_legality << "0\n";
	  }
	  scop_stream_legality << "\n";
	  scop_stream_legality << CreateDelimiter("-") << this->GetStmtNum(1) << ".3 Access" << "\n";
	  if (this->GetAccessFuncStat()) {
	      scop_stream_legality << "# Access informations are provided\n";
	      scop_stream_legality << "1\n";
	      scop_stream_legality << "# Read access informations\n";
	      scop_stream_legality << read_access_matrix;
	      //scop_stream_legality << "\n";
	      scop_stream_legality << "# Write access informations\n";
	      scop_stream_legality << write_access_matrix;
	      scop_stream_legality << "\n";
	  } else {
	      scop_stream_legality << "# Access informations are not provided\n";
	      scop_stream_legality << "0\n";
	  }
	  scop_stream_legality << CreateDelimiter("-") << this->GetStmtNum(1) << ".4 Body" << "\n";
	  scop_stream_legality << "# Statement body is provided\n";
	  scop_stream_legality << "1\n";
	  scop_stream_legality << "# Original iterator names\n";
	  
	  std::string iterator_names{""};
	  size_t iterator_num = curr_iterators.size();
	  for (size_t i = 0; i < iterator_num; i++) {
	      if (i == iterator_num - 1){
	          iterator_names = iterator_names + curr_iterators[i];
	      } else {
	          iterator_names = iterator_names + curr_iterators[i] + " ";
	      }
	  }
	  scop_stream_legality << iterator_names <<"\n";
	  scop_stream_legality << "# Statement body\n";
	  scop_stream_legality << statement << ";\n";
	  scop_stream_legality << "\n\n";
  }

  this->ResetScatFuncStat();
  this->ResetAccessFuncStat();

}

void CodeGenSCoP::AssembleSCoP(int phase) {

  stream << "# [File generated by HeteroCL for Generated Code Verification]\n";
  stream << "\n";
  stream << "SCoP\n";
  stream << "\n";
  stream << CreateDelimiter("=") << "Global\n";
  stream << "# Language\n";
  stream << "C\n";
  stream << "\n";
  stream << "# Context\n";
  stream << this->GetParams() << " " << this->GetParams() + 2 << "\n";
  stream << "\n"; // FIXME: Double check with LNP once again
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
  stream << this->GetStmtNum(2) << "\n";
  stream << "\n";
  stream << scop_stream_codegen.str();
  stream << CreateDelimiter("=") << "Options\n";
}

void CodeGenSCoP::UpdateIterCoefficient(std::string s, std::string coeff) {
    min_extent_coeff_map.insert({s, coeff});
}

void CodeGenSCoP::UpdateReadWriteAccessCoefficient(std::string vid) {
    read_write_coeff_map.insert({vid, read_write_coeff});
}

void CodeGenSCoP::UpdateReadWriteAccessCoefficient(std::string s, std::string coeff) {
    auto found = read_write_coeff.find(s);
    if (found != read_write_coeff.end()) {
        std::string coeff_ = found->second;
        read_write_coeff.erase(found);
        int modified_coeff = std::stoi(coeff) + std::stoi(coeff_);
        coeff = std::to_string(modified_coeff);
    }
    read_write_coeff.insert({s, coeff});
}

bool CodeGenSCoP::IsNumeric(std::string &s) {
    return !s.empty() && std::find_if(s.begin(),
            s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

}  // namespace codegen
}  // namespace TVM
