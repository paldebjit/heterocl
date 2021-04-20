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

void CodeGenSCoP::GenHintSch() {

    chdir("tmp");

    // No tiling
    std::string _cmd1 = POCC_HOME + "/bin/pocc --read-scop kernel.scop --output-scop -o kernel_plutoized_nt --no-codegen --pluto 2>&1";
    const char* cmd1 = _cmd1.c_str();

    std::string result;
    result = this->ExecCmd(cmd1);

    // With fixed tiling size
    std::string _cmd2 = "echo 4 4 4 4 4 4 4 4 4 4 4 4 > tile.sizes 2>&1";
    std::string _cmd3 = POCC_HOME + "/bin/pocc --read-scop kernel.scop --output-scop -o kernel_plutoized_t --no-codegen --pluto --pluto-tile --pluto-tile-scat 2>&1";
    const char* cmd2 = _cmd2.c_str();
    const char* cmd3 = _cmd3.c_str();

    result = this->ExecCmd(cmd2);
    result = this->ExecCmd(cmd3);

    chdir("..");

}

void CodeGenSCoP::VerifySchedule() {
    
    chdir("tmp");

    std::string _cmd1 = POCC_HOME + "/bin/pocc --read-scop kernel.scop --check-schedule --verbose > check_schedule.rpt 2>&1";
    const char* cmd1 = _cmd1.c_str();
    
    std::string result;
    result = this->ExecCmd(cmd1);

    chdir("..");

}

bool CodeGenSCoP::ParseVerifSchResult() {

    chdir("tmp");

    std::string LINE = "[PoCC] No violation detected in candidate schedule";
    std::ifstream file("check_schedule.rpt");
    std::string str;
    while(std::getline(file, str)) {
        if(LINE.compare(str) == 0) {
            chdir("..");
            return true;
        }
    }

    chdir("..");
    return false;

}

void CodeGenSCoP::VerifyGenCode() {
    
    // FIXME: Ask LNP about the shcedule thing. It is an orange box in the Step 1 slide. How does
    //        he want that information to be grabbed in the .mat file?
    std::string _cmd1 = POCC_HOME + "/bin/pocc kernel.c --codegen-tracer --tc-scheds schedules.kernel.mat -o kernel_tracer.c";
    const char* cmd1 = _cmd1.c_str();

    std::string _cmd2 = POCC_HOME + "/bin/pocc kernel_transformed.c --codegen-tracer -o kernel_transformed_tracer.c";
    const char* cmd2 = _cmd2.c_str();
    
    std::string _cmd3 = "gcc kernel_tracer.c -o kernel_tracer";
    const char* cmd3 = _cmd3.c_str();

    std::string _cmd4 = "gcc kernel_transformed_tracer.c -o kernel_transformed_tracer";
    const char* cmd4 = _cmd4.c_str();

    std::string _cmd5 = "./kernel_tracer > kernel.trace";
    const char* cmd5 = _cmd5.c_str();

    std::string _cmd6 = "./kernel_transformed_tracer > kernel_transformed.trace";
    const char* cmd6 = _cmd6.c_str();

    std::string _cmd7 = POCC_HOME + "/generators/scripts/trace-checker kernel.trace kernel_transformed.trace";
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

    std::ofstream scop_file;
    scop_file.open("tmp/kernel.scop");
    scop_file << stream.str();
    scop_file.close();

    std::ofstream scat_file;
    scat_file.open("tmp/schedules.kernel.mat");
    scat_file << "<schedule-candidate>\n";
    scat_file << this->GetStmtNum() << "\n";
    scat_file << scat_stream.str();
    scat_file << "</schedule-candidate>";
    scat_file.close();

}

void CodeGenSCoP::Verify() {

    this->WriteSCoP();
    this->CheckEnv();
    this->VerifySchedule();
    if (!this->ParseVerifSchResult()) {
        LOG(FATAL) << "Schedule verification failed. Check tmp/violation.rpt for more details.\n";
    } else {
        LOG(INFO) << "Schedule verification passed. Proceeding to verify generated code.\n";
    }
    //this->VerifyGenCode();
    //this->ParseVerifGenCodeResult();
    this->GenHintSch();

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
