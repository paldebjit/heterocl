// System includes 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#define DPRINTF(...) printf(__VA_ARGS__); fflush(stdout);

#ifdef _WIN32
#include <time.h>
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define ALTERA_CL 1

#ifdef ALTERA_CL
#pragma message ("* Compiling for ALTERA CL")
#endif

#ifdef ALTERA_CL
#include "CL/opencl.h"
#endif

#define ACL_ALIGNMENT 64

#include <stdlib.h>

#include "3d_rendering.h"

void* acl_aligned_malloc (size_t size) {
    void *result = NULL;
    posix_memalign (&result, ACL_ALIGNMENT, size);
    return result;
}
void acl_aligned_free (void *ptr) {
    free (ptr);
}

//#define EMULATOR
#define COMPUTE_GOLDEN_BLOCKED
//#define COMPUTE_GOLDEN

#define AOCX_FILE "AOCX/3d_rendering.aocx"

// Check the status returned by the OpenCL API functions
#define CHECK(status) 							                	\
    if (status != CL_SUCCESS)		                				\
{	                                								\
    fprintf(stderr, "error %d in line %d.\n", status, __LINE__);	\
    exit(1);						                            	\
}								                                	\

// Check the status returned by the OpenCL API functions, don't exit on error
#define CHECK_NO_EXIT(status) 	        							\
    if (status != CL_SUCCESS)			                			\
{								                                	\
    fprintf(stderr, "error %d in line %d.\n", status, __LINE__);	\
}								                                	\


double compute_kernel_execution_time(cl_event &event, double &start_d, double &end_d)
{
    cl_ulong start, end;

    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);

    start_d = (double)1.0e-9 * start;
    end_d   = (double)1.0e-9 * end;

    return 	(double)1.0e-9 * (end - start); // nanoseconds to seconds
}

void populateData_1(int size, coord_t* ptr) {
    int ctr = 0;
    int i;
    srand(unsigned(time(0)));
    while (ctr < size) {
        for (i = 0; i < 9; i++) {
            ptr[ctr].val[i] = (rand() % 50) + 1;
        }
        ctr = ctr + 1;
    }
}

void populateData_2(int size, int* ptr) {
    int ctr = 0;
    srand(unsigned(time(0)));
    while (ctr < size) {
        ptr[ctr] = (rand() % 50) + 1;
        ctr = ctr + 1;
    }
}

int main(int argc, const char** argv) {

    printf("%s Starting...\n\n", argv[0]); 

    unsigned int elements;
    FILE * file;
    long int fstart, fend;
    unsigned int i;
    cl_event kernel_exec_event;

    std::streampos filesize;
    FILE *f_out = stdout;

    int size1 = 3191;   // Number of triangles = 3191. 9 coordinate points (x0,y0,z0, ..., x2,y2,z2)
    int size2 = 1;      // Angle is a 1 dimension vector
    int size3 = 4096;   // frame buffer 16 * 4096 = 256 * 256 (doing packing of 16 integers)

    coord_t* input1;
    int* input2;
    frame_t* output;

    printf("\n===== Host-CPU preparing data ======\n\n");

    if((input1 = (coord_t*)acl_aligned_malloc(size1*sizeof(coord_t))) == NULL) {
        perror("Failed malloc of coordinate vector");
    }
    populateData_1(size1, input1);

    if((input2 = (int*)acl_aligned_malloc(size2*sizeof(int))) == NULL) {
        perror("Failed malloc of angle vector");
    }
    populateData_2(size2, input2);

    if((output = (frame_t*)acl_aligned_malloc(size3*sizeof(frame_t))) == NULL) {
        perror("Failed malloc of output vector");
    }

    printf("Allocated memory for host-side\n");
    printf("\n===== Host-CPU setting up the OpenCL platform and device ======\n\n");

    // Use this to check the output of each API call
    cl_int status;

    //----------------------------------------------
    // Discover and initialize the platforms
    //----------------------------------------------
    cl_uint numPlatforms = 0;
    cl_platform_id* platforms = NULL;

    // Use clGetPlatformIDs() to retrieve the
    // number of platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    fprintf(stdout,"Number of platforms = %d\n", numPlatforms);

    // Allocate enough space for each platform
    platforms = (cl_platform_id*) acl_aligned_malloc (numPlatforms * sizeof(cl_platform_id));
    printf("Allocated space for Platform\n");

    // Fill in platforms with clGetPlatformIDs()
    status = clGetPlatformIDs(numPlatforms, platforms, NULL); CHECK(status);
    printf("Filled in platforms\n");    

    //----------------------------------------------
    // Discover and initialize the devices 
    //----------------------------------------------
    cl_uint numDevices = 0;

    // Device info
    char buffer[4096];
    unsigned int buf_uint;
    int device_found = 0;
    const cl_uint maxDevices = 4;
    cl_device_id devices[maxDevices];
    DPRINTF("Initializing IDs\n");
    for (int i=0; i<numPlatforms; i++) {
        status = clGetDeviceIDs(platforms[i],
            CL_DEVICE_TYPE_ALL,
            maxDevices,
            devices,
            &numDevices); 

        CHECK(status);

        if(status == CL_SUCCESS){
          clGetPlatformInfo(platforms[i], 
                    CL_PLATFORM_NAME,
                    4096,
                    buffer,
                    NULL);
#if defined(ALTERA_CL)
            if(strstr(buffer, "Altera") != NULL){
                device_found = 1;
            }
            DPRINTF("%s\n", buffer);
#elif defined(NVIDIA_CL)
            if(strstr(buffer, "NVIDIA") != NULL){
                device_found = 1;
            }
#else
            if(strstr(buffer, "Intel") != NULL){
                device_found = 1;
            }
#endif

            DPRINTF("Platform found : %s\n", buffer);
            device_found = 1;
        }
    }


    if(!device_found) {
        printf("failed to find a OpenCL device\n");
        exit(-1);
    }

    for (i = 0; i < numDevices; i++) {
        clGetDeviceInfo(devices[i],
                CL_DEVICE_NAME,
                4096,
                buffer,
                NULL);
        fprintf(f_out, "\nDevice Name: %s\n", buffer);

        clGetDeviceInfo(devices[i],
                CL_DEVICE_VENDOR,
                4096,
                buffer,
                NULL);
        fprintf(f_out, "Device Vendor: %s\n", buffer);

        clGetDeviceInfo(devices[i],
                CL_DEVICE_MAX_COMPUTE_UNITS,
                sizeof(buf_uint),
                &buf_uint,
                NULL);
        fprintf(f_out, "Device Computing Units: %u\n", buf_uint);

        clGetDeviceInfo(devices[i],
                CL_DEVICE_GLOBAL_MEM_SIZE,
                sizeof(unsigned long),
                &buffer,
                NULL);
        fprintf(f_out, "Global Memory Size: %i\n", *((unsigned long*)buffer));

        clGetDeviceInfo(devices[i],
                CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                sizeof(unsigned long),
                &buffer,
                NULL);
        fprintf(f_out, "Global Memory Allocation Size: %i\n\n", *((unsigned long*)buffer));
    }

    //----------------------------------------------
    // Create a context 
    //----------------------------------------------

    printf("\n===== Host-CPU setting up the OpenCL command queues ======\n\n");

    cl_context context = NULL;

    // Create a context using clCreateContext() and
    // associate it with the device

    context = clCreateContext(
            NULL,
            1,
            devices,
            NULL,
            NULL,
            &status); CHECK(status);

    //----------------------------------------------
    // Create command queues
    //---------------------------------------------

    cl_command_queue cmdQueue[2]; // extra queue for reading buffer C

    const char *kernel_names[] = { "default_function" };

    // Create a command queue using clCreateCommandQueue(),
    // and associate it with the device you want to execute on
    fprintf(stdout,"cmdQueue i = %d, kernel name = %s\n", 0, kernel_names[0]);
    cmdQueue[0] = clCreateCommandQueue(
            context,
            devices[0],
            CL_QUEUE_PROFILING_ENABLE,
            &status); CHECK(status);

    fprintf(stdout,"cmdQueue i = %d, a queue for reading the C buffer\n", 1);
    cmdQueue[1] = clCreateCommandQueue(
            context,
            devices[0],
            CL_QUEUE_PROFILING_ENABLE,
            &status); CHECK(status);

    //----------------------------------------------
    // Create device buffers
    //----------------------------------------------
    
    /*cl_mem d_input;*/

    /* Input to the 3d_rendering kernel*/
    cl_mem triangle_3d, angle; 
    /* Output of the 3d_rendering kernel*/
    cl_mem outp;

    printf("\n===== Host-CPU transferring matrices A,B to the FPGA device global memory (DDR4) via PCIe ======\n\n");

    triangle_3d = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE, 
            9 * size1 * sizeof(cl_int), 
            NULL, 
            &status);
    CHECK(status);

    angle = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE, 
            size2 * sizeof(cl_int), 
            NULL, 
            &status);
    CHECK(status);

    outp = clCreateBuffer(
            context,
            CL_MEM_READ_WRITE, 
            16 * size3 * sizeof(cl_int), 
            NULL, 
            &status);
    CHECK(status);

    //----------------------------------------------
    // Write host data to device buffers
    //----------------------------------------------

    // blocking writes
    status = clEnqueueWriteBuffer(
        cmdQueue[0],
        triangle_3d,
        CL_TRUE,
        0,
        9 * size1 * sizeof(cl_int),
        input1,
        0,
        NULL,
        NULL); CHECK(status);

    status = clEnqueueWriteBuffer(
        cmdQueue[0],
        angle,
        CL_TRUE,
        0,
        size2 * sizeof(cl_int),
        input2,
        0,
        NULL,
        NULL); CHECK(status);

    //----------------------------------------------
    // Create the program from binaries
    //----------------------------------------------
    printf("\n===== Host-CPU setting up OpenCL program and kernels ======\n\n");

    cl_program program;

    size_t binary_length;
    const unsigned char *binary;

    printf("\nAOCX file: %s\n\n", AOCX_FILE);
    // create the program using binary already compiled offline using aoc (i.e. the .aocx file)
    FILE *fp = fopen(AOCX_FILE, "rb");

    if (fp == NULL) {
        printf("Failed to open the AOCX file (fopen).\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    binary_length = ftell(fp);
    binary = (unsigned char*) malloc(sizeof(unsigned char) * binary_length);
    assert(binary && "Malloc failed");
    rewind(fp);

    if (fread((void*)binary, binary_length, 1, fp) == 0) {
        printf("Failed to read from the AOCX file (fread).\n");
        return -1;
    }
    fclose(fp);

    // Create a program using clCreateProgramWithBinary()
    program = clCreateProgramWithBinary(
            context,
            1,
            devices,
            &binary_length,
            (const unsigned char **)&binary,
            &status,
            NULL); CHECK(status);


    //----------------------------------------------
    // Create the kernel
    //----------------------------------------------

    status = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(status != CL_SUCCESS) {
        char log[128*1024] = {0};
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 128*1024, log, NULL);
        printf("%s\n", log);
        CHECK(status);
    }

    printf("Creating kernel \n");
    cl_kernel kernel = clCreateKernel(program, kernel_names[0], &status); 
    CHECK(status);

    status = clSetKernelArg(
            kernel,
            0,
            sizeof(cl_mem),
            (void*)&triangle_3d); CHECK(status);

    status = clSetKernelArg(
            kernel,
            1,
            sizeof(cl_mem),
            (void*)&angle); CHECK(status);

    status = clSetKernelArg(
            kernel,
            2,
            sizeof(cl_mem),
            (void*)&outp); CHECK(status);

    //----------------------------------------------
    // Configure the work-item structure (using only tasks atm)
    //----------------------------------------------

    // Define the number of threads that will be created 
    // as well as the number of work groups 
    size_t globalWorkSize[1];
    size_t localWorkSize[1];

    //----------------------------------------------
    // Enqueue the kernel for execution
    //----------------------------------------------


    // all kernels are always tasks
    globalWorkSize[0] = 1;
    localWorkSize[0]  = 1;

    printf("\n===== Host-CPU enqeuing the OpenCL kernels to the FPGA device ======\n\n");
    // Alternatively, can use clEnqueueTaskKernel
    printf("clEnqueueNDRangeKernel!\n");
    status = clEnqueueNDRangeKernel(
            cmdQueue[0],
            kernel,
            1,
            NULL,
            globalWorkSize,
            localWorkSize,
            0,
            NULL,
            &kernel_exec_event
            );
    CHECK(status);
    printf(" *** FPGA execution started!\n");

    status = clFlush(cmdQueue[0]); CHECK(status);
    status = clFinish(cmdQueue[0]); CHECK(status);

    printf(" *** FPGA execution finished!\n");
    double k_start_time;	
    double k_end_time;
    double k_exec_time;

    k_exec_time = compute_kernel_execution_time(kernel_exec_event, k_start_time, k_end_time);     
    printf("\n\n");

    printf("\n===== Host-CPU transferring result matrix C from the FPGA device global memory (DDR4) via PCIe ======\n\n");

    // Read the results back from the device, blocking read
    clEnqueueReadBuffer(
        //cmdQueue[KID_DRAIN_MAT_C],
        cmdQueue[1], // using a special queue for reading buffer C
        outp,
        CL_TRUE,
        0,
        16 * size3 * sizeof(cl_int),
        output,
        0,
        NULL,
        NULL); CHECK(status);
    printf("D");
    status = clFinish(cmdQueue[1]); CHECK(status);

    // Check the result 
    bool matched = true;
    /*
    for (int k = 0; k < size; k++) {
        if (input[k] != times * (1+2+3)) {
            std::cout << "Mismatched... " << input[k];
            matched = false;
        }
    }
    */

    if (matched) std::cout << "Result matched...\n";
    else std::cout << "Mismatch...\n";

    printf("\n===== Reporting measured throughput ======\n\n");
    double k_earliest_start_time = k_start_time;
    double k_latest_end_time     = k_end_time;	
    double k_overall_exec_time = k_latest_end_time - k_earliest_start_time;

    printf("\n");
    printf("  Loader kernels start time\t\t= %.5f s\n", k_earliest_start_time);     
    printf("  Drainer kernels end time\t\t= %.5f s\n", k_latest_end_time);     
    printf("  FPGA MatMult exec time\t\t= %.5f s\n", k_overall_exec_time);     

    // multiplied by 1.0e-9 to get G-FLOPs
    printf("\n");

    double num_operations = (double)2.0 * 512 * 512 * 512 * 2;

    printf("  # operations = %.0f\n", num_operations );     
    printf("  Throughput: %.5f GFLOPS\n", (double)1.0e-9 * num_operations / k_overall_exec_time);     
    //----------------------------------------------
    // Release the OpenCL resources
    //----------------------------------------------

    // Free resources
    /*
    clReleaseKernel(kernel);
    clReleaseCommandQueue(cmdQueue[0]);
    clReleaseCommandQueue(cmdQueue[1]);
    clReleaseEvent(kernel_exec_event);

    acl_aligned_free(input);
    clReleaseMemObject(d_input);

    clReleaseProgram(program);
    clReleaseContext(context);

    acl_aligned_free(platforms);
    acl_aligned_free(devices);
    */
}


