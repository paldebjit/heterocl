#include "ihc_apint.h"
#pragma OPENCL EXTENSION cl_intel_arbitrary_precision_integers : enable
__kernel void default_function(__global float* restrict inputs, __global float* restrict pred) {
  float _top;
  float update;
  for (int x = 0; x < 30; ++x) {
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      pred[(x * 3)] = (pred[(x * 3)] + 2.051282e+00f);
    } else {
      pred[(x * 3)] = (pred[(x * 3)] + -9.876543e-01f);
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.638554e-01f);
    } else {
      if (inputs[((x * 4) + 2)] <= 1.750000e+00f) {
        if (inputs[(x * 4)] <= 4.950000e+00f) {
          if (inputs[((x * 4) + 2)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.162162e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.638554e-01f);
          }
        } else {
          if (inputs[((x * 4) + 2)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.638554e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.120156e+00f);
          }
        }
      } else {
        if (inputs[(x * 4)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 11)] <= 5.950000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.162162e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.638554e-01f);
          }
        } else {
          pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.638554e-01f);
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.052632e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
        } else {
          if (inputs[((x * 4) + 1)] <= 5.450000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.052632e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 11)] <= 3.100000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.052632e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
        } else {
          if (inputs[((x * 4) + 16)] <= 6.750000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.818182e+00f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      pred[(x * 3)] = (pred[(x * 3)] + 2.009510e+00f);
    } else {
      if (inputs[(x * 4)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 3)] <= 1.750000e+00f) {
          if (inputs[((x * 4) + 3)] <= 1.550000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.831162e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.836726e-01f);
          }
        } else {
          if (inputs[((x * 4) + 7)] <= 3.150000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.827344e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.829373e-01f);
          }
        }
      } else {
        if (inputs[((x * 4) + 3)] <= 1.700000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + -9.797680e-01f);
        } else {
          if (inputs[((x * 4) + 12)] <= 6.600000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.827344e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.827344e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
        pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.597774e-01f);
      } else {
        if (inputs[((x * 4) + 3)] <= 3.750000e+00f) {
          if (inputs[((x * 4) + 3)] <= 3.450000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.597774e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.597774e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 3.500000e-01f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.597774e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.597774e-01f);
          }
        }
      }
    } else {
      if (inputs[(x * 4)] <= 1.750000e+00f) {
        if (inputs[((x * 4) + 11)] <= 4.950000e+00f) {
          if (inputs[(x * 4)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.115292e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.595087e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.595087e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.102392e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 11)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 1)] <= 5.950000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.115292e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.595087e-01f);
          }
        } else {
          if (inputs[((x * 4) + 1)] <= 6.150000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.595087e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.595087e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          if (inputs[((x * 4) + 1)] <= 2.350000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.046325e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.046464e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
        } else {
          if (inputs[((x * 4) + 9)] <= 6.950000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.048422e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.797933e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 9)] <= 5.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.046464e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
        }
      } else {
        if (inputs[((x * 4) + 16)] <= 2.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
        } else {
          if (inputs[((x * 4) + 1)] <= 5.550000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.786451e+00f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      if (inputs[((x * 4) + 1)] <= 3.500000e-01f) {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          if (inputs[((x * 4) + 3)] <= 3.900000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + 1.969610e+00f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + 1.969610e+00f);
          }
        } else {
          if (inputs[((x * 4) + 3)] <= 3.300000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + 1.969610e+00f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + 1.969610e+00f);
          }
        }
      } else {
        pred[(x * 3)] = (pred[(x * 3)] + 1.969610e+00f);
      }
    } else {
      if (inputs[(x * 4)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 1)] <= 1.750000e+00f) {
          if (inputs[((x * 4) + 1)] <= 1.550000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.786359e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.797386e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 4.850000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.782888e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.778945e-01f);
          }
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 1.700000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + -9.721542e-01f);
        } else {
          if (inputs[((x * 4) + 20)] <= 6.600000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.778945e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.778945e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.557469e-01f);
      } else {
        if (inputs[((x * 4) + 1)] <= 5.250000e+00f) {
          pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.557469e-01f);
        } else {
          if (inputs[((x * 4) + 1)] <= 5.350000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.557469e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.557469e-01f);
          }
        }
      }
    } else {
      if (inputs[(x * 4)] <= 1.750000e+00f) {
        if (inputs[((x * 4) + 9)] <= 4.950000e+00f) {
          if (inputs[(x * 4)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.070624e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.552261e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.552253e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.085129e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 9)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 1)] <= 5.950000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.070626e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.552249e-01f);
          }
        } else {
          if (inputs[((x * 4) + 9)] <= 5.150000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.552249e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.552249e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          if (inputs[(x * 4)] <= 8.000000e-01f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.040136e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.040405e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755985e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755989e+00f);
        } else {
          if (inputs[((x * 4) + 1)] <= 5.450000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.044258e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.778334e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 13)] <= 5.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.040405e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755991e+00f);
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
          if (inputs[((x * 4) + 17)] <= 3.150000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755991e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755990e+00f);
          }
        } else {
          if (inputs[((x * 4) + 17)] <= 3.350000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755991e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.755991e+00f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      pred[(x * 3)] = (pred[(x * 3)] + 1.931467e+00f);
    } else {
      if (inputs[(x * 4)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 3)] <= 1.750000e+00f) {
          if (inputs[((x * 4) + 3)] <= 1.550000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.742131e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.758523e-01f);
          }
        } else {
          if (inputs[((x * 4) + 7)] <= 3.150000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.731334e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.737080e-01f);
          }
        }
      } else {
        if (inputs[((x * 4) + 3)] <= 1.700000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + -9.647999e-01f);
        } else {
          if (inputs[(x * 4)] <= 6.500000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.731335e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.731335e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.517639e-01f);
    } else {
      if (inputs[(x * 4)] <= 1.750000e+00f) {
        if (inputs[((x * 4) + 3)] <= 4.950000e+00f) {
          if (inputs[(x * 4)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.028014e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.510069e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.510048e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.068346e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 3)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 11)] <= 5.950000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 2.028018e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.510038e-01f);
          }
        } else {
          if (inputs[((x * 4) + 3)] <= 5.150000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.510034e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.510034e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          if (inputs[(x * 4)] <= 8.000000e-01f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.034062e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.034453e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726715e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726724e+00f);
        } else {
          if (inputs[((x * 4) + 1)] <= 5.450000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.040139e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.759354e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 13)] <= 3.100000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726728e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.034452e+00f);
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
          if (inputs[((x * 4) + 13)] <= 3.150000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726730e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726729e+00f);
          }
        } else {
          if (inputs[((x * 4) + 1)] <= 5.250000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726730e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.726730e+00f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 2)] <= 4.500000e-01f) {
          pred[(x * 3)] = (pred[(x * 3)] + 1.894973e+00f);
        } else {
          if (inputs[((x * 4) + 2)] <= 5.500000e-01f) {
            pred[(x * 3)] = (pred[(x * 3)] + 1.894973e+00f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + 1.894973e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 5.350000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + 1.894973e+00f);
        } else {
          pred[(x * 3)] = (pred[(x * 3)] + 1.894973e+00f);
        }
      }
    } else {
      if (inputs[(x * 4)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 2)] <= 1.750000e+00f) {
          if (inputs[((x * 4) + 2)] <= 1.550000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.698476e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.720134e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 4.850000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.691942e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.684497e-01f);
          }
        }
      } else {
        if (inputs[((x * 4) + 2)] <= 1.700000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + -9.576930e-01f);
        } else {
          pred[(x * 3)] = (pred[(x * 3)] + -9.684500e-01f);
        }
      }
    }
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.478284e-01f);
    } else {
      if (inputs[(x * 4)] <= 1.750000e+00f) {
        if (inputs[((x * 4) + 3)] <= 4.950000e+00f) {
          if (inputs[(x * 4)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.987329e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.468502e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.468461e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.052022e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 3)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 11)] <= 5.950000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.987336e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.468439e-01f);
          }
        } else {
          if (inputs[((x * 4) + 3)] <= 5.150000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.468435e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.468434e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          if (inputs[((x * 4) + 1)] <= 2.350000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.028099e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.028605e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698575e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698593e+00f);
        } else {
          if (inputs[((x * 4) + 9)] <= 6.950000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.036066e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.740963e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 9)] <= 5.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.028603e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698602e+00f);
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
          if (inputs[((x * 4) + 17)] <= 3.150000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698604e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698600e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.698604e+00f);
        }
      }
    }
    if (inputs[(x * 4)] <= 2.350000e+00f) {
      if (inputs[((x * 4) + 1)] <= 3.450000e+00f) {
        pred[(x * 3)] = (pred[(x * 3)] + 1.860026e+00f);
      } else {
        if (inputs[((x * 4) + 3)] <= 5.150000e+00f) {
          if (inputs[((x * 4) + 4)] <= 5.000000e-01f) {
            pred[(x * 3)] = (pred[(x * 3)] + 1.860026e+00f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + 1.860026e+00f);
          }
        } else {
          pred[(x * 3)] = (pred[(x * 3)] + 1.860026e+00f);
        }
      }
    } else {
      if (inputs[(x * 4)] <= 5.150000e+00f) {
        if (inputs[((x * 4) + 4)] <= 1.750000e+00f) {
          if (inputs[((x * 4) + 4)] <= 1.550000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.655392e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.682218e-01f);
          }
        } else {
          if (inputs[((x * 4) + 1)] <= 3.150000e+00f) {
            pred[(x * 3)] = (pred[(x * 3)] + -9.638423e-01f);
          } else {
            pred[(x * 3)] = (pred[(x * 3)] + -9.647465e-01f);
          }
        }
      } else {
        if (inputs[((x * 4) + 4)] <= 1.700000e+00f) {
          pred[(x * 3)] = (pred[(x * 3)] + -9.508220e-01f);
        } else {
          pred[(x * 3)] = (pred[(x * 3)] + -9.638427e-01f);
        }
      }
    }
    if (inputs[(x * 4)] <= 8.000000e-01f) {
      pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.439405e-01f);
    } else {
      if (inputs[(x * 4)] <= 1.750000e+00f) {
        if (inputs[((x * 4) + 3)] <= 4.950000e+00f) {
          if (inputs[(x * 4)] <= 1.650000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.948447e+00f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.427553e-01f);
          }
        } else {
          if (inputs[(x * 4)] <= 1.550000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.427487e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.036141e+00f);
          }
        }
      } else {
        if (inputs[((x * 4) + 3)] <= 4.850000e+00f) {
          if (inputs[((x * 4) + 11)] <= 3.100000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.427457e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + 1.948459e+00f);
          }
        } else {
          if (inputs[((x * 4) + 3)] <= 5.150000e+00f) {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.427444e-01f);
          } else {
            pred[((x * 3) + 1)] = (pred[((x * 3) + 1)] + -9.427443e-01f);
          }
        }
      }
    }
    if (inputs[(x * 4)] <= 1.750000e+00f) {
      if (inputs[((x * 4) + 1)] <= 4.950000e+00f) {
        if (inputs[(x * 4)] <= 1.650000e+00f) {
          if (inputs[(x * 4)] <= 8.000000e-01f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.022246e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.022859e+00f);
          }
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671505e+00f);
        }
      } else {
        if (inputs[(x * 4)] <= 1.550000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671533e+00f);
        } else {
          if (inputs[((x * 4) + 9)] <= 6.950000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.032037e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.723133e+00f);
          }
        }
      }
    } else {
      if (inputs[((x * 4) + 1)] <= 4.850000e+00f) {
        if (inputs[((x * 4) + 9)] <= 5.950000e+00f) {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + -1.022856e+00f);
        } else {
          pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671546e+00f);
        }
      } else {
        if (inputs[((x * 4) + 1)] <= 5.150000e+00f) {
          if (inputs[((x * 4) + 17)] <= 3.150000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671551e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671547e+00f);
          }
        } else {
          if (inputs[((x * 4) + 1)] <= 5.650000e+00f) {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671551e+00f);
          } else {
            pred[((x * 3) + 2)] = (pred[((x * 3) + 2)] + 1.671551e+00f);
          }
        }
      }
    }
  }
}

