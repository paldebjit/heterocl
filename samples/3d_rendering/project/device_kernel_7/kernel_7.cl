#include "ihc_apint.h"
#include "3d_rendering.h"

/* triangle_3d is of size 3191. Each pack_t contains an int array of size 9 where each array element
   is a coordinate value.
   typedef struct {
     int val[9];
   } coord_t;
*/

#pragma OPENCL EXTENSION cl_intel_arbitrary_precision_integers : enable
__kernel void default_function(__global coord_t* restrict triangle_3d, __global int* restrict angle, __global frame_t* restrict outp) {

  int _top;
  int z_buffer[65536];
  int frame_buffer[65536];
  for (int x = 0; x < 256; ++x) {
    for (int y = 0; y < 256; ++y) {
      z_buffer[(y + (x * 256))] = 255;
    }
  }
  int main_body;
  for (int m = 0; m < 3191; ++m) {
    /* Get all 9 coordinatre value x0, y0, z0, ..., x2, y2, z2 by a single main memory access
       and copy them on chip memory. 
    */
    coord_t coord = triangle_3d[m];
    int triangle_3d_[9];
    #pragma unroll
    for (int x1 = 0; x1 < 9; ++x1) {
      /* Get the individual coordinates from on chip memory */
      triangle_3d_[x1] = coord.val[x1];
    }
    int fragment[2000];
    pack_t pixels[500];
    int triangle_2d[7];
    int frag_cntr;
    frag_cntr = 0;
    int size_pixels;
    size_pixels = 0;
    int twod_update;
    for (int x2 = 0; x2 < 7; ++x2) {
      if (x2 == 0) {
        if (angle[0] == 2) {
          triangle_2d[0] = triangle_3d_[2];
        } else {
          triangle_2d[0] = triangle_3d_[0];
        }
      }
      if (x2 == 1) {
        if (angle[0] == 1) {
          triangle_2d[1] = triangle_3d_[2];
        } else {
          triangle_2d[1] = triangle_3d_[1];
        }
      }
      if (x2 == 2) {
        if (angle[0] == 2) {
          triangle_2d[2] = triangle_3d_[5];
        } else {
          triangle_2d[2] = triangle_3d_[3];
        }
      }
      if (x2 == 3) {
        if (angle[0] == 1) {
          triangle_2d[3] = triangle_3d_[5];
        } else {
          triangle_2d[3] = triangle_3d_[4];
        }
      }
      if (x2 == 4) {
        if (angle[0] == 2) {
          triangle_2d[4] = triangle_3d_[8];
        } else {
          triangle_2d[4] = triangle_3d_[6];
        }
      }
      if (x2 == 5) {
        if (angle[0] == 1) {
          triangle_2d[5] = triangle_3d_[8];
        } else {
          triangle_2d[5] = triangle_3d_[7];
        }
      }
      if (x2 == 6) {
        if (angle[0] == 0) {
          triangle_2d[6] = ((int)(((int64_t)(((int64_t)(triangle_3d_[2] / 3)) + ((int64_t)(triangle_3d_[5] / 3)))) + ((int64_t)(triangle_3d_[8] / 3))));
        } else {
          if (angle[0] == 1) {
            triangle_2d[6] = ((int)(((int64_t)(((int64_t)(triangle_3d_[1] / 3)) + ((int64_t)(triangle_3d_[4] / 3)))) + ((int64_t)(triangle_3d_[7] / 3))));
          } else {
            if (angle[0] == 2) {
              triangle_2d[6] = ((int)(((int64_t)(((int64_t)(triangle_3d_[0] / 3)) + ((int64_t)(triangle_3d_[3] / 3)))) + ((int64_t)(triangle_3d_[6] / 3))));
            }
          }
        }
      }
    }
    int mutate0;
    int x0;
    x0 = triangle_2d[0];
    int y0;
    y0 = triangle_2d[1];
    int x11;
    x11 = triangle_2d[2];
    int y1;
    y1 = triangle_2d[3];
    int x21;
    x21 = triangle_2d[4];
    int y2;
    y2 = triangle_2d[5];
    int z;
    z = triangle_2d[6];
    int cw;
    cw = ((int)((((int64_t)((int64_t)(x21 - x0))) * ((int64_t)((int64_t)(y1 - y0)))) - (((int64_t)((int64_t)(y2 - y0))) * ((int64_t)((int64_t)(x11 - x0))))));
    if (cw == 0) {
      frag_cntr = 0;
    } else {
      if (cw < 0) {
        int scalar0;
        scalar0 = x0;
        int scalar1;
        scalar1 = y0;
        x0 = x11;
        y0 = y1;
        x11 = scalar0;
        y1 = scalar1;
      }
    }
    int scalar2;
    scalar2 = 0;
    int scalar3;
    scalar3 = 0;
    int scalar4;
    scalar4 = 0;
    int scalar5;
    scalar5 = 0;
    if (x0 < x11) {
      if (x21 < x0) {
        scalar2 = x21;
      } else {
        scalar2 = x0;
      }
    } else {
      if (x21 < x11) {
        scalar2 = x21;
      } else {
        scalar2 = x11;
      }
    }
    if (x11 < x0) {
      if (x0 < x21) {
        scalar3 = x21;
      } else {
        scalar3 = x0;
      }
    } else {
      if (x11 < x21) {
        scalar3 = x21;
      } else {
        scalar3 = x11;
      }
    }
    if (y0 < y1) {
      if (y2 < y0) {
        scalar4 = y2;
      } else {
        scalar4 = y0;
      }
    } else {
      if (y2 < y1) {
        scalar4 = y2;
      } else {
        scalar4 = y1;
      }
    }
    if (y1 < y0) {
      if (y0 < y2) {
        scalar5 = y2;
      } else {
        scalar5 = y0;
      }
    } else {
      if (y1 < y2) {
        scalar5 = y2;
      } else {
        scalar5 = y1;
      }
    }
    int color;
    color = 100;
    int scalar6;
    scalar6 = 0;
    int S1;
    for (int i = 0; i < (scalar5 - scalar4); ++i) {
      for (int i1 = 0; i1 < (scalar3 - scalar2); ++i1) {
        int compute0;
        compute0 = ((int)((((int64_t)((int64_t)(((int64_t)(i1 - x0)) + ((int64_t)scalar2)))) * ((int64_t)((int64_t)(y1 - y0)))) - (((int64_t)((int64_t)(((int64_t)(i - y0)) + ((int64_t)scalar4)))) * ((int64_t)((int64_t)(x11 - x0))))));
        int compute1;
        compute1 = ((int)((((int64_t)((int64_t)(((int64_t)(i1 - x11)) + ((int64_t)scalar2)))) * ((int64_t)((int64_t)(y2 - y1)))) - (((int64_t)((int64_t)(((int64_t)(i - y1)) + ((int64_t)scalar4)))) * ((int64_t)((int64_t)(x21 - x11))))));
        int compute2;
        compute2 = ((int)((((int64_t)((int64_t)(((int64_t)(i1 - x21)) + ((int64_t)scalar2)))) * ((int64_t)((int64_t)(y0 - y2)))) - (((int64_t)((int64_t)(((int64_t)(i - y2)) + ((int64_t)scalar4)))) * ((int64_t)((int64_t)(x0 - x21))))));
        if (0 <= min(min(compute0, compute1), compute2)) {
          fragment[(scalar6 * 4)] = ((int)(((int64_t)i1) + ((int64_t)scalar2)));
          fragment[((scalar6 * 4) + 1)] = ((int)(((int64_t)i) + ((int64_t)scalar4)));
          fragment[((scalar6 * 4) + 2)] = z;
          fragment[((scalar6 * 4) + 3)] = color;
          scalar6 = (scalar6 + 1);
        }
      }
    }
    frag_cntr = scalar6;
    int mutate1;
    int scalar7;
    scalar7 = 0;
    int S2;
    #pragma ivdep
    for (int i2 = 0; i2 < frag_cntr; ++i2) {
      int scalar8;
      scalar8 = fragment[(i2 * 4)];
      int scalar9;
      scalar9 = fragment[((i2 * 4) + 1)];
      int scalar10;
      scalar10 = fragment[((i2 * 4) + 2)];
      int scalar11;
      scalar11 = fragment[((i2 * 4) + 3)];
      if (scalar10 < z_buffer[(scalar8 + (scalar9 * 256))]) {
        pack_t data1 = {scalar8, scalar9, scalar11};
        pixels[scalar7] = data1;
        scalar7 = (scalar7 + 1);
        z_buffer[(scalar8 + (scalar9 * 256))] = scalar10;
      }
    }
    size_pixels = scalar7;
    int mutate2;
    for (int x3 = 0; x3 < size_pixels; ++x3) {
      pack_t data2 = pixels[x3];
      int scalar12;
      scalar12 = data2.a;
      int scalar13;
      scalar13 = data2.b;
      frame_buffer[(scalar13 + (scalar12 * 256))] = data2.c;
    }
  }
  /* For output packing */

  /* Style 1 */
  /*
  for (int i3 = 0; i3 < 4096; ++i3) {
    frame_t data3 = {frame_buffer[i3 + 0], frame_buffer[i3 + 1], 
                     frame_buffer[i3 + 2], frame_buffer[i3 + 3], 
                     frame_buffer[i3 + 4], frame_buffer[i3 + 5],
                     frame_buffer[i3 + 6], frame_buffer[i3 + 7],
                     frame_buffer[i3 + 8], frame_buffer[i3 + 9],
                     frame_buffer[i3 + 10], frame_buffer[i3 + 11],
                     frame_buffer[i3 + 12], frame_buffer[i3 + 13],
                     frame_buffer[i3 + 14], frame_buffer[i3 + 15]
                     };  
    outp[i3] = data3;
    */
    /* Style 2 */
   for (int i3 = 0; i3 < 256; ++i3) {
            /* Intel Manual 6.1 */
            #pragma ii 1
            for (int i4 = 0; i4 < 256; i4 = i4 + 16) {
                frame_t data3 = {frame_buffer[i3 * 256 + i4 + 0], 
                                 frame_buffer[i3 * 256 + i4 + 1],  
                                 frame_buffer[i3 * 256 + i4 + 2],  
                                 frame_buffer[i3 * 256 + i4 + 3],  
                                 frame_buffer[i3 * 256 + i4 + 4],  
                                 frame_buffer[i3 * 256 + i4 + 5],  
                                 frame_buffer[i3 * 256 + i4 + 6],  
                                 frame_buffer[i3 * 256 + i4 + 7],  
                                 frame_buffer[i3 * 256 + i4 + 8],  
                                 frame_buffer[i3 * 256 + i4 + 9],  
                                 frame_buffer[i3 * 256 + i4 + 10],  
                                 frame_buffer[i3 * 256 + i4 + 11],  
                                 frame_buffer[i3 * 256 + i4 + 12],  
                                 frame_buffer[i3 * 256 + i4 + 13],  
                                 frame_buffer[i3 * 256 + i4 + 14],  
                                 frame_buffer[i3 * 256 + i4 + 15]  
                                 };  
                outp[i3 * 256/16 + i4/16] = data3;
        }
    }
}
