#include <iostream>

#include "exception_handler.hpp"
#include "state_machine.hpp"

template <typename StreamIn>
bool PopulateStream(sycl::queue q, size_t len, float *test_data,
                    float *test_data_expected, float test_coeff) {
  for (int i = 0; i < len; i++) {
    test_data_expected[i] = test_data[i] * test_coeff;
    MyStreamingBeat test_beat;
    test_beat.sop = (i == 0);
    test_beat.eop = (i == (len - 1));
    test_beat.data = test_data[i];
    StreamIn::write(q, test_beat);
  }
  return true;
}

template <typename StreamOut>
bool CheckOutput(sycl::queue q, size_t len, float *test_data_expected,
                 float *test_data_calc) {
  std::cout << "Capture and check output..." << std::endl;
  bool passed = true;
  for (int i = 0; i < len; i++) {
    bool did_read = true;
    MyStreamingBeat test_beat = StreamOut::read(q);
    if (!did_read) {
      std::cerr << "ERROR: could not read sample " << i << std::endl;
      return false;
    }

    test_data_calc[i] = test_beat.data;

    bool first_beat = (i == 0);
    bool last_beat = (i == (len - 1));
    // check for failures
    if (first_beat != test_beat.sop) {
      std::cerr << "ERROR: SOP mismatch [" << i << "]" << std::endl;
      passed = false;
    }
    if (last_beat != test_beat.eop) {
      std::cerr << "ERROR: EOP mismatch [" << i << "]" << std::endl;
      passed = false;
    }
    if (test_data_calc[i] != test_data_expected[i]) {
      std::cerr << "ERROR: data mismatch [" << i << "]" << std::endl;
      passed = false;
    }
  }
  return passed;
}

#if FPGA_SIMULATOR
constexpr int kInputSize = 5;
#else
constexpr int kInputSize = 2000;
#endif

// add some additional iterations to account for the state changes
constexpr int kIterations = kInputSize + 1;

int main() {
  bool passed = true;

  try {
    // Use compile-time macros to select either:
    //  - the FPGA emulator device (CPU emulation of the FPGA)
    //  - the FPGA device (a real FPGA)
    //  - the simulator device
#if FPGA_SIMULATOR
    auto selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
    auto selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
    auto selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif

    sycl::queue q(selector, fpga_tools::exception_handler,
                  sycl::property::queue::enable_profiling{});

    auto device = q.get_device();

    // make sure the device supports USM host allocations
    if (!device.has(sycl::aspect::usm_host_allocations)) {
      std::cerr << "This design must either target a board that supports USM "
                   "Host/Shared allocations, or IP Component Authoring. "
                << std::endl;
      std::terminate();
    }

    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;

    // create test data
    float *test_data = new float[kInputSize];
    float coeff_optimized = 0.95f;
    float coeff_naive = 1.95f;
    float *test_data_expected_naive = new float[kInputSize];
    float *test_data_expected_optimized = new float[kInputSize];
    float *test_data_calc = new float[kInputSize];
    for (int i = 0; i < kInputSize; i++) {
      test_data[i] = i;
    }

    // Allocate shared memory for an integer.
    int *iteration_count_shared = sycl::malloc_shared<int>(1, q);
    float *coeff_shared = sycl::malloc_shared<float>(1, q);

    // Test for naive implementation.
    PopulateStream<StreamIn_NaiveStateMachine>(q, kInputSize, test_data, test_data_expected_naive,
                             coeff_naive);
    coeff_shared[0] = coeff_naive;
    std::cout << "\nTest for naive implementation of the state machine:" << std::endl;
    std::cout << "initialize coeff = " << coeff_shared[0] << "... " << std::endl;
    std::cout << "Process a vector of size " << kInputSize << std::endl;
    *iteration_count_shared = 0;
    q.single_task<IDNaiveStateMachine>(NaiveStateMachine{
      coeff_shared, iteration_count_shared, true, (kIterations)
      }).wait();
    passed &=
        CheckOutput<StreamOut_NaiveStateMachine>(q, kInputSize, test_data_expected_naive, test_data_calc);
    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;
    
    // Test for optimized implementation.
    PopulateStream<StreamIn_OptimizedStateMachine>(q, kInputSize, test_data, test_data_expected_optimized,
                             coeff_optimized);
    coeff_shared[0] = coeff_optimized;
    std::cout << "Test for optimized implementation of the state machine:" << std::endl;
    std::cout << "initialize coeff = " << coeff_shared[0] << "... " << std::endl;
    std::cout << "Process a vector of size " << kInputSize << std::endl;
    q.single_task<IDOptimizedStateMachine>(OptimizedStateMachine{
      coeff_shared, iteration_count_shared, true, (kIterations)
      }).wait();
    passed &=
        CheckOutput<StreamOut_OptimizedStateMachine>(q, kInputSize, test_data_expected_optimized, test_data_calc);
    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;

    // TODO: add performance comparison

    sycl::free(coeff_shared, q);
    delete[] test_data;
    delete[] test_data_calc;
    delete[] test_data_expected_optimized;
    delete[] test_data_expected_naive;
  } catch (sycl::exception const &e) {
    // Catches exceptions in the host code.
    std::cerr << "Caught a SYCL host exception:\n" << e.what() << "\n";

    // Most likely the runtime couldn't find FPGA hardware!
    if (e.code().value() == CL_DEVICE_NOT_FOUND) {
      std::cerr << "If you are targeting an FPGA, please ensure that your "
                   "system has a correctly configured FPGA board.\n";
      std::cerr << "Run sys_check in the oneAPI root directory to verify.\n";
      std::cerr << "If you are targeting the FPGA emulator, compile with "
                   "-DFPGA_EMULATOR.\n";
    }
    std::terminate();
  }

  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}