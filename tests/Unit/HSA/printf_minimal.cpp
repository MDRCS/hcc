// RUN: %hc %s -DHCC_ENABLE_ACCELERATOR_PRINTF  -o %t.out && %t.out | %FileCheck %s

#include <hc/hc.hpp>
#include <hc/hc_printf.hpp>

int main() {
  hc::parallel_for_each(hc::extent<1>(1), [](hc::index<1>) [[hc]] {
      hc::printf("Accelerator: Hello World!\n");
  }).wait();
  return 0;
}

// CHECK: Accelerator: Hello World!
