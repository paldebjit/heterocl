#include "ihc_apint.h"
#pragma OPENCL EXTENSION cl_intel_arbitrary_precision_integers : enable
__kernel void default_function(__global float* restrict A, __global float* restrict Gx, __global float* restrict Gy, __global float* restrict output) {
  float _top;
  float B[64516];
  for (int x = 0; x < 254; ++x) {
    for (int y = 0; y < 254; ++y) {
      int sum1;
      sum1 = 0;
      for (int ra0 = 0; ra0 < 3; ++ra0) {
        for (int ra1 = 0; ra1 < 3; ++ra1) {
          sum1 = ((int)((A[((y + ra1) + ((x + ra0) * 256))] * Gx[(ra1 + (ra0 * 3))]) + ((float)sum1)));
        }
      }
      B[(y + (x * 254))] = ((float)sum1);
    }
  }
  float C[64516];
  for (int x1 = 0; x1 < 254; ++x1) {
    for (int y1 = 0; y1 < 254; ++y1) {
      int sum2;
      sum2 = 0;
      for (int ra2 = 0; ra2 < 3; ++ra2) {
        for (int ra3 = 0; ra3 < 3; ++ra3) {
          sum2 = ((int)((A[((y1 + ra3) + ((x1 + ra2) * 256))] * Gy[(ra3 + (ra2 * 3))]) + ((float)sum2)));
        }
      }
      C[(y1 + (x1 * 254))] = ((float)sum2);
    }
  }
  for (int x2 = 0; x2 < 254; ++x2) {
    for (int y2 = 0; y2 < 254; ++y2) {
      output[(y2 + (x2 * 254))] = (sqrtf(((B[(y2 + (x2 * 254))] * B[(y2 + (x2 * 254))]) + (C[(y2 + (x2 * 254))] * C[(y2 + (x2 * 254))]))) * 5.891867e-02f);
    }
  }
}

