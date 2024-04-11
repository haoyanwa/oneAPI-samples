#include <iostream>

#include "exception_handler.hpp"
#include "state_machine.hpp"

template <typename StreamIn>
bool populateStream(sycl::queue q, size_t len, float *testData,
                    float *testDataExpected, float testCoeff) {
  for (int i = 0; i < len; i++) {
    testDataExpected[i] = testData[i] * testCoeff;
    MyStreamingBeat testBeat;
    testBeat.sop = (i == 0);
    testBeat.eop = (i == (len - 1));
    testBeat.data = testData[i];
    StreamIn::write(q, testBeat);
  }
  return true;
}

template <typename StreamOut>
bool checkOutput(sycl::queue q, size_t len, float *testDataExpected,
                 float *testDataCalc) {
  std::cout << "capture and check output..." << std::endl;
  bool passed = true;
  for (int i = 0; i < len; i++) {
    bool didRead = true;
    MyStreamingBeat testBeat = StreamOut::read(q);
    if (!didRead) {
      std::cerr << "ERROR: could not read sample " << i << std::endl;
      return false;
    }

    testDataCalc[i] = testBeat.data;

    bool firstBeat = (i == 0);
    bool lastBeat = (i == (len - 1));
    // check for failures
    if (firstBeat != testBeat.sop) {
      std::cerr << "ERROR: SOP mismatch [" << i << "]" << std::endl;
      passed = false;
    }
    if (lastBeat != testBeat.eop) {
      std::cerr << "ERROR: EOP mismatch [" << i << "]" << std::endl;
      passed = false;
    }
    if (testDataCalc[i] != testDataExpected[i]) {
      std::cerr << "ERROR: data mismatch [" << i << "]" << std::endl;
      passed = false;
    }
  }
  return passed;
}

#define _USE_MATH_DEFINES
#include <math.h>
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
    float *testData = new float[kInputSize];
    float coeff_ProperStateMachine = 0.95f;
    float coeff_NaiveStateMachine = 1.95f;
    float *testDataExpected_NaiveStateMachine = new float[kInputSize];
    float *testDataExpected_ProperStateMachine = new float[kInputSize];
    float *testDataCalc = new float[kInputSize];
    for (int i = 0; i < kInputSize; i++) {
      testData[i] = i;
    }

    int *iterationCountShared = sycl::malloc_shared<int>(1, q);

    populateStream<StreamIn_ProperStateMachine>(q, kInputSize, testData, testDataExpected_ProperStateMachine,
                             coeff_ProperStateMachine);
    float *coeffShared = sycl::malloc_shared<float>(1, q);
    coeffShared[0] = coeff_ProperStateMachine;
    // TODO: add performance comparison
    std::cout << "Test for proper implementation of the state machine:" << std::endl;
    std::cout << "initialize coeff = " << coeffShared[0] << "... " << std::endl;
    std::cout << "Process a vector of size " << kInputSize << std::endl;
    q.single_task<IDProperStateMachine>(ProperStateMachine{
      coeffShared, iterationCountShared, true, (kIterations)
      }).wait();
    passed &=
        checkOutput<StreamOut_ProperStateMachine>(q, kInputSize, testDataExpected_ProperStateMachine, testDataCalc);

    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;

    populateStream<StreamIn_NaiveStateMachine>(q, kInputSize, testData, testDataExpected_NaiveStateMachine,
                             coeff_NaiveStateMachine);
    coeffShared[0] = coeff_NaiveStateMachine;
    std::cout << "\nTest for naive implementation of the state machine:" << std::endl;
    std::cout << "initialize coeff = " << coeffShared[0] << "... " << std::endl;
    std::cout << "Process a vector of size " << kInputSize << std::endl;
    *iterationCountShared = 0;
    q.single_task<IDNaiveStateMachine>(NaiveStateMachine{
      coeffShared, iterationCountShared, true, (kIterations)
      }).wait();
    passed &=
        checkOutput<StreamOut_NaiveStateMachine>(q, kInputSize, testDataExpected_NaiveStateMachine, testDataCalc);
    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;
    
    sycl::free(coeffShared, q);
    delete[] testData;
    delete[] testDataCalc;
    delete[] testDataExpected_ProperStateMachine;
    delete[] testDataExpected_NaiveStateMachine;
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