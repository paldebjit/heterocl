/*!
 *  Copyright (c) 2021 by Contributors
 * \file codegen_pocc_util.cc
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

int CodeGenPoCC::Index(std::string vid, std::vector<std::string> vmap) {
    int index{-1};
    auto it = std::find(vmap.begin(), vmap.end(), vid);
    if (it != vmap.end()) {
        index = it - vmap.begin();
    }
    return index;
}

std::vector<std::string> CodeGenPoCC::Split(const std::string s, char delim) {
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

std::vector<std::string> CodeGenPoCC::Split(const std::string s, std::string delim) {
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
        std::unordered_map<std::string, std::string> coeff_map;
        auto found = read_write_coeff_map.find(rw_var);
        // Checking if Read/Write access is a scalar access or array element access
        if(found != read_write_coeff_map.end()){
           coeff_map = found->second;
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
    /* <VID      <SYMBOL_NAME     SYMBOL_COEFFICIENT>> */
    int row{0};
    for (size_t i = 0; i < iterators_.size(); i++) {

        std::vector<std::string> v;

        std::string iterator = std::get<0>(iterators_[i].LB);
        std::unordered_map<std::string, std::string> lb = std::get<1>(iterators_[i].LB);
        std::unordered_map<std::string, std::string> ub = std::get<1>(iterators_[i].UB);
        
        /* Tackling the lower bound */
        std::string lb_expr{"## " + iterator + " >= "} ;
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
        matrix_[row] = row1;
        row++;

        /* Tackling the upper bound */
        std::string ub_expr{"## -" + iterator + " + "};
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

        ub_expr = ub_expr + this->Join(v, " + ") + " >= 0";
        v.clear();

        row2[no_of_cols] = ub_expr;
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

void CodeGenPoCC::UpdateIterCoefficient(std::string s, std::string coeff) {
    min_extent_coeff_map.insert({s, coeff});
}

void CodeGenPoCC::UpdateReadWriteAccessCoefficient(std::string vid, std::string s, std::string coeff) {
    auto found = read_write_coeff_map.find(vid);
    if (found != read_write_coeff_map.end()) {
        read_write_coeff_map[vid].insert({s, coeff});
    } else {
        std::unordered_map<std::string, std::string> s_coeff = {{s, coeff}};
        read_write_coeff_map.insert({vid, s_coeff});
    }
}

bool CodeGenPoCC::IsNumeric(std::string &s) {
    return !s.empty() && std::find_if(s.begin(),
            s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

}  // namespace codegen
}  // namespace TVM
