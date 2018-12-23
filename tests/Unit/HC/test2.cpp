
// RUN: %hc %s -o %t.out && %t.out

#include <hc/hc.hpp>

#include <iostream>

#define TEST_DEBUG (0)

// loop to deliberately slow down kernel execution
#define LOOP_COUNT (1024)

#define GRID_SIZE (32)

/// test fetching the number of async operations associated with one accelerator_view
int main() {

  bool ret = true;

  std::vector<int> table1(GRID_SIZE);
  std::vector<int> table2(GRID_SIZE);

  std::vector<int> table3(GRID_SIZE);
  std::vector<int> table4(GRID_SIZE);
  std::vector<int> table5(GRID_SIZE);

  for (int i = 0; i < GRID_SIZE; ++i) {
    table1[i] = i;
    table2[i] = i;
  }

  hc::array_view<const int, 1> av1(GRID_SIZE, table1);
  hc::array_view<const int, 1> av2(GRID_SIZE, table2);
  hc::array_view<int, 1> av3(GRID_SIZE, table3);
  hc::array_view<int, 1> av4(GRID_SIZE, table4);
  hc::array_view<int, 1> av5(GRID_SIZE, table5);

  hc::accelerator_view accelerator_view = hc::accelerator().get_default_view();

  // do 3 kernel dispatches + 3 barriers
  hc::parallel_for_each(hc::extent<1>(GRID_SIZE), [=](hc::index<1>& idx) [[hc]] {
    for (int i = 0; i < LOOP_COUNT; ++i)
      av3(idx) = av1(idx) + av2(idx);
  });

  accelerator_view.create_marker();

  hc::parallel_for_each(hc::extent<1>(GRID_SIZE), [=](hc::index<1>& idx) [[hc]] {
    for (int i = 0; i < LOOP_COUNT; ++i)
      av4(idx) = av1(idx) + av2(idx);
  });

  accelerator_view.create_marker();

  hc::parallel_for_each(hc::extent<1>(GRID_SIZE), [=](hc::index<1>& idx) [[hc]] {
    for (int i = 0; i < LOOP_COUNT; ++i)
      av5(idx) = av1(idx) + av2(idx);
  });

  accelerator_view.create_marker();

  // wait for async operations to complete
  hc::accelerator().get_default_view().wait();

  for (decltype(GRID_SIZE) i = 0; i != GRID_SIZE; ++i) {
    if (av3[i] != 2 * i) return EXIT_FAILURE;
    if (av4[i] != 2 * i) return EXIT_FAILURE;
    if (av5[i] != 2 * i) return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

