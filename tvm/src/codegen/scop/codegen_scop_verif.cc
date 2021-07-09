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

// Working plan for the 2-step verification
// Important PoCC commands that will be used for the verification approach
// Assume PoCC is available in the path. 
// Step 0: Check existence of executables
//         i) if pocc and gcc exists in the system path. FATAL if not.
//         ii) $ mkdir -pv tmp
// Step 1: 
//         i) Generate the SCoP. Check if the SCoP exists. Dump the SCoP in ./tmp
//         ii) Generate schedules.kernel.mat and dump it in ./tmp
//         iii) Alternately append the modified schedule within the .scop file itself
// Step 2: Check schedule violation and capture the PoCC output in a log file
//          $ cd tmp
//          $ $POCC_HOME/bin/pocc --read-scop kernel.scop --check-schedule --verbose
// Step 3: Check the generated code with respect to the verified schedule from Step 2
//          $ $POCC_HOME/bin/pocc kernel.c --codegen-tracer --tc-scheds schedules.kernel.mat
//          $ $POCC_HOME/bin/pocc kernel_transformed.c --codegen-tracer --tc-scheds schedules.kernel.mat
//          $ gcc kernel.pocc.c -o kernel
//          $ gcc kernel_transformed.pocc.c -o kernel_transformed
//          $ ./kernel > trace.kernel
//          $ ./kernel_transformed > trace.kernel_transformed
//          $ $POCC_HOME/generators/scripts/trace-checker trace.kernel trace.kernel_transformed
// Step 4: Generate an optimized schedule using PLuTo (this is an optional step)
//          $ echo "4 4 4 4 4 4 4 4 4 4 4 4" > tile.sizes (can also be done with file write)
//          $ $POCC_HOME/bin/pocc --read-scop kernel.scop --output-scop kernel_plutoized.scop --no-codegen
//                                --pluto --pluto-tile --pluto-tile-scat

void CodeGenSCoP::VerifySchedule() {
    
    chdir("tmp");

    std::vector<std::string> files_to_check = {"legality.scop",
                                               "kernel.transfo.hclcp"};
    
    for (const auto &file: files_to_check) {
        if (FILE *fp = fopen(file.c_str(), "r")) {
            fclose(fp);
        } else {
            LOG(FATAL) << "Could not find " << file << " in run directory to for schedule verification.\n";
        }
    }

    std::string _cmd1 = POCC_HOME + "/generators/scripts/hcl-scop-check-customization-primitives-legality legality.scop kernel.transfo.hclcp > check_schedule.rpt 2>&1";
    const char* cmd1 = _cmd1.c_str();
    
    std::string result;
    result = this->ExecCmd(cmd1);

    chdir("..");

}

bool CodeGenSCoP::ParseVerifSchResult() {

    chdir("tmp");

    std::string LINE = "[HCL-check-legality][OK]";
    std::ifstream file("check_schedule.rpt");
    std::string str;
    while(std::getline(file, str)) {
        if(str.find(LINE) != std::string::npos) {
            chdir("..");
            return true;
        }
    }

    chdir("..");
    return false;

}

bool CodeGenSCoP::ParseVerifGenCodeResult() {

    chdir("tmp");

    std::string LINE_F = "[TraceChecker] Traces differ";
    std::string LINE_P = "[TraceChecker][OK]";

    std::ifstream file("check_codegen.rpt");
    std::string str;
    while(std::getline(file, str)) {
        if(str.find(LINE_P) != std::string::npos) {
            chdir("..");
            return true;
        } else {
            if(str.find(LINE_F) != std::string::npos) {
                chdir("..");
                return false;
            }
        }
    }

    chdir("..");
    return false;
}

void CodeGenSCoP::VerifyGenCode() {

    chdir("tmp");

    std::vector<std::string> files_to_check = {"kernel_orig.c",
                                               "identityscheds_codegen.mat",
                                               "kernel_opt.c"};
    
    for (const auto &file: files_to_check) {
        if (FILE *fp = fopen(file.c_str(), "r")) {
            fclose(fp);
        } else {
            LOG(FATAL) << "Could not find " << file << " in run directory to for generated code verification.\n";
        }
    }
    
    // FIXME: Ask LNP about the shcedule thing. It is an orange box in the Step 1 slide. How does
    //        he want that information to be grabbed in the .mat file?
    std::string _cmd1 = POCC_HOME + "/generators/scripts/hcl-c-function-to-tracer kernel_orig.c identityscheds_codegen.mat";
    const char* cmd1 = _cmd1.c_str();

    std::string _cmd2 = POCC_HOME + "/generators/scripts/hcl-c-function-to-tracer kernel_opt.c";
    const char* cmd2 = _cmd2.c_str();
    
    std::string _cmd3 = "gcc -O2 -lm kernel_orig.c.tracer.c -o tracer_orig";
    const char* cmd3 = _cmd3.c_str();

    std::string _cmd4 = "gcc -O2 -lm kernel_opt.c.tracer.c -o tracer_transfo";
    const char* cmd4 = _cmd4.c_str();

    std::string _cmd5 = "ulimit -s unlimited && ./tracer_orig > trace_original.trace.txt";
    const char* cmd5 = _cmd5.c_str();

    std::string _cmd6 = "ulimit -s unlimited && ./tracer_transfo > trace_transfo.trace.txt";
    const char* cmd6 = _cmd6.c_str();

    std::string _cmd7 = POCC_HOME + "/generators/scripts/trace-checker-faster trace_original.trace.txt trace_transfo.trace.txt > check_codegen.rpt 2>&1";
    const char* cmd7 = _cmd7.c_str();

    std::string result;

    result = this->ExecCmd(cmd1);
    result = this->ExecCmd(cmd2);
    result = this->ExecCmd(cmd3);
    result = this->ExecCmd(cmd4);
    result = this->ExecCmd(cmd5);
    result = this->ExecCmd(cmd6);
    result = this->ExecCmd(cmd7);

}

void CodeGenSCoP::CheckEnv() {

    static const std::string MY_VAR = "POCC_HOME";
    char const* temp = getenv(MY_VAR.c_str());
    if (temp != NULL) {
        POCC_HOME = std::string(temp);
    } else {
        LOG(FATAL) << "Set the POCC_HOME environment variable.\n";
    }

    struct stat sb_pocc;
    std::string POCC_BIN = POCC_HOME + "/bin/pocc";
    if ((stat(POCC_BIN.c_str(), &sb_pocc) == 0) && (sb_pocc.st_mode & S_IXOTH)) {
        LOG(INFO) << "PoCC found in system path.\n";
    } else {
        LOG(FATAL) << "PoCC executable not found.\n";
    }

    if (std::system("which gcc > /dev/null 2>&1")) {
        LOG(FATAL) << "gcc not found\n";
    } else {
        LOG(INFO) << "gcc found in system path.\n";
    }
    
    struct stat sb_tmp;
    if ((stat("tmp", &sb_tmp) == 0) && (((sb_tmp.st_mode) & S_IFMT) == S_IFDIR)) {
        LOG(WARNING) << "tmp directory already exists in workspace.\n";
    } else {
        if(mkdir("tmp", 0755) == -1) {
            LOG(FATAL) << "tmp directory cannot be created in the workspace.\n";
        } else {
            LOG(INFO) << "tmp directory created in the workspace.\n";
        }
    }
}

void CodeGenSCoP::WriteSCoP() {

    // Write two SCoP files. One for legality verification and another for codegen verification
    // Write two Identity Sched files. One for legality verification and another for codegen verification


    // SCoP file for legality checking from original algorithm
    std::ofstream scop_file_legality;
    scop_file_legality.open("tmp/legality.scop");
    scop_file_legality << "# [File generated by HeteroCL for Generated Code Verification]\n";
    scop_file_legality << "\n";
    scop_file_legality << "SCoP\n";
    scop_file_legality << "\n";
    scop_file_legality << CreateDelimiter("=") << "Global\n";
    scop_file_legality << "# Language\n";
    scop_file_legality << "C\n";
    scop_file_legality << "\n";
    scop_file_legality << "# Context\n";
    scop_file_legality << this->GetParams() << " " << this->GetParams() + 2 << "\n";
    scop_file_legality << "\n"; // FIXME: Double check with LNP once again
    if(!this->IsParamEmpty()) {
        scop_file_legality << "# Parameter names are provided\n";
        scop_file_legality << this->GetParams() << "\n";
        scop_file_legality << "# Parameter names\n";
        scop_file_legality << this->WriteParams() << "\n";
        scop_file_legality << "\n";
    } else {
        scop_file_legality << "# Parameter names are not provided\n";
        scop_file_legality << this->GetParams() << "\n";
        scop_file_legality << "\n";
    }
    scop_file_legality << "# Number of statements\n";
    scop_file_legality << this->GetStmtNum(1) << "\n";
    scop_file_legality << "\n";
    scop_file_legality << scop_stream_legality.str();
    scop_file_legality << CreateDelimiter("=") << "Options\n";
    scop_file_legality.close();

    // SCoP file for codegen checking from original algorithm
    std::ofstream scop_file_codegen;
    scop_file_codegen.open("tmp/codegen.scop");
    scop_file_codegen << "# [File generated by HeteroCL for Generated Code Verification]\n";
    scop_file_codegen << "\n";
    scop_file_codegen << "SCoP\n";
    scop_file_codegen << "\n";
    scop_file_codegen << CreateDelimiter("=") << "Global\n";
    scop_file_codegen << "# Language\n";
    scop_file_codegen << "C\n";
    scop_file_codegen << "\n";
    scop_file_codegen << "# Context\n";
    scop_file_codegen << this->GetParams() << " " << this->GetParams() + 2 << "\n";
    scop_file_codegen << "\n"; // FIXME: Double check with LNP once again
    if(!this->IsParamEmpty()) {
        scop_file_codegen << "# Parameter names are provided\n";
        scop_file_codegen << this->GetParams() << "\n";
        scop_file_codegen << "# Parameter names\n";
        scop_file_codegen << this->WriteParams() << "\n";
        scop_file_codegen << "\n";
    } else {
        scop_file_codegen << "# Parameter names are not provided\n";
        scop_file_codegen << this->GetParams() << "\n";
        scop_file_codegen << "\n";
    }
    scop_file_codegen << "# Number of statements\n";
    scop_file_codegen << this->GetStmtNum(2) << "\n";
    scop_file_codegen << "\n";
    scop_file_codegen << scop_stream_codegen.str();
    scop_file_codegen << CreateDelimiter("=") << "Options\n";
    scop_file_codegen.close();

    // Identity schedule for legality verification
    std::ofstream scat_file_legality;
    scat_file_legality.open("tmp/identityscheds_legality.mat");
    scat_file_legality << "<schedule-candidate>\n";
    scat_file_legality << this->GetStmtNum(1) << "\n";
    scat_file_legality << scat_stream_legality.str();
    scat_file_legality << "</schedule-candidate>";
    scat_file_legality.close();


    // Identity schedule for generated code verification
    std::ofstream scat_file_codegen;
    scat_file_codegen.open("tmp/identityscheds_codegen.mat");
    scat_file_codegen << "<schedule-candidate>\n";
    scat_file_codegen << this->GetStmtNum(2) << "\n";
    scat_file_codegen << scat_stream_codegen.str();
    scat_file_codegen << "</schedule-candidate>";
    scat_file_codegen.close();

}

void CodeGenSCoP::VerifyLegality() {

    this->WriteSCoP();
    this->CheckEnv();
    this->VerifySchedule();
    if (!this->ParseVerifSchResult()) {
        LOG(FATAL) << "Schedule verification failed. Check tmp/violation.rpt for more details.\n";
    } else {
        LOG(INFO) << "Schedule verification passed.\n";
    }

}

void CodeGenSCoP::VerifyE2E() {

    this->WriteSCoP();
    this->CheckEnv();
    this->VerifySchedule();
    if (!this->ParseVerifSchResult()) {
        LOG(FATAL) << "Schedule verification failed. Check tmp/violation.rpt for more details.\n";
    } else {
        LOG(INFO) << "Schedule verification passed. Proceeding to verify generated code.\n";
    }
    this->VerifyGenCode();
    if(!this->ParseVerifGenCodeResult()) {
        LOG(FATAL) << "Generated code does not follow the legal schedule. Check tmp/violation.rpt for more details.\n";
    } else {
        LOG(INFO) << "Generated code follows the legal schedule.\n";
    }

}

std::string CodeGenSCoP::ExecCmd(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    auto pipe = popen(cmd, "r");

    if (!pipe) {
        LOG(FATAL) << "popen() failed!\n";
    }

    while(!feof(pipe)) {
        if(fgets(buffer.data(), 128, pipe) != nullptr) {
            result += buffer.data();
        }
    }

    auto rc = pclose(pipe);
    
    if (rc == EXIT_SUCCESS) {
        LOG(INFO) << "Successfully executed command: " << std::string(cmd) << "\n";
    } else {
        LOG(FATAL) << "Failed to execute command: " << std::string(cmd) << "\n";
    }

    return result;

}

}  // namespace codegen
}  // namespace TVM
