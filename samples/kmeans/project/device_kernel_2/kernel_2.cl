#include "ihc_apint.h"
#pragma OPENCL EXTENSION cl_intel_arbitrary_precision_integers : enable
__kernel void default_function(__global int* restrict points, __global int* restrict means, __global int* restrict labels) {
  int _top;
  int main_loop;
  for (int _1 = 0; _1 < 200; ++_1) {
    #pragma ii 1
    for (int n = 0; n < 320; ++n) {
      int scalar0;
      scalar0 = 100000;
      int scalar1;
      scalar1 = labels[n];
      for (int i = 0; i < 16; ++i) {
        int scalar2;
        scalar2 = 0;
        for (int i1 = 0; i1 < 32; ++i1) {
          scalar2 = ((int)(((int64_t)scalar2) + ((int64_t)(((int64_t)((int33_t)(points[(i1 + (n * 32))] - means[(i1 + (i * 32))]))) * ((int64_t)((int33_t)(points[(i1 + (n * 32))] - means[(i1 + (i * 32))])))))));
        }
        if (scalar2 < scalar0) {
          scalar0 = scalar2;
          scalar1 = i;
        }
      }
      labels[n] = scalar1;
    }
    int num_k[16];
    int num_k_partitioned;
    int sum_k[512];
    int sum_k_partitioned;
    int calc_sum;
    #pragma ii 1
    for (int n1 = 0; n1 < 320; ++n1) {
      num_k[labels[n1]] = (num_k[labels[n1]] + 1);
      for (int i2 = 0; i2 < 32; ++i2) {
        sum_k[(i2 + (labels[n1] * 32))] = ((int)(((int33_t)sum_k[(i2 + (labels[n1] * 32))]) + ((int33_t)points[(i2 + (n1 * 32))])));
      }
    }
    int update_mean;
    for (int k = 0; k < 16; ++k) {
      #pragma unroll
      for (int d = 0; d < 32; ++d) {
        means[(d + (k * 32))] = (sum_k[(d + (k * 32))] / num_k[k]);
      }
    }
  }
}

