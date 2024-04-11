#ifndef STATE_MACHINE_HPP
#define STATE_MACHINE_HPP

#include <sycl/ext/intel/experimental/task_sequence.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/prototype/pipes_ext.hpp>
#include <sycl/sycl.hpp>

enum State {
  PROCESS,
  LD_COEFF
};

// Forward declare the kernel name in the global scope. This is an FPGA best
// practice that reduces name mangling in the optimization reports.
class IDNaiveStateMachine;
class IDProperStateMachine;
class IDStreamDataIn_NaiveStateMachine;
class IDStreamDataIn_ProperStateMachine;
class IDStreamDataOut_NaiveStateMachine;
class IDStreamDataOut_ProperStateMachine;

// define data pipes
using StreamingPipeProps = decltype(sycl::ext::oneapi::experimental::properties(
    sycl::ext::intel::experimental::uses_valid<true>,
    sycl::ext::intel::experimental::bits_per_symbol<32>,
    sycl::ext::intel::experimental::protocol<
        sycl::ext::intel::experimental::protocol_name::
            avalon_streaming_uses_ready>));
using MyStreamingBeat =
    sycl::ext::intel::experimental::StreamingBeat<float, true, false>;

using StreamIn_NaiveStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataIn_NaiveStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>;
using StreamIn_ProperStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataIn_ProperStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>; 
using StreamOut_NaiveStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataOut_NaiveStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>;
using StreamOut_ProperStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataOut_ProperStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>;

// function for task_sequence
float updateCoeff(float *coeffPtr, int *executionCount) {
  float coeffNew = coeffPtr[0];
  executionCount[0] = executionCount[0] + 1;
  return coeffNew;
}

float compute(float coeff, float data) {
  return coeff * data;
}

struct NaiveStateMachine {
  float *coeffPtr;
  int *executionCount;
  bool initCoeffBeforeStarting = false;
  int iterations;

  void operator()() const {
    State myState = 
        (initCoeffBeforeStarting) ? State::LD_COEFF : State::PROCESS;
    float coeff = 1.0f;

    for (int i = 0; i < iterations; i++) {
      switch (myState) {
        case State::PROCESS: {
          MyStreamingBeat beat = StreamIn_NaiveStateMachine::read();
          beat.data = beat.data * coeff;
          StreamOut_NaiveStateMachine::write(beat);
        } break;
        
        case State::LD_COEFF: {
          float coeffNew = coeffPtr[0];
          executionCount[0] = executionCount[0] + 1;
          coeff = coeffNew;
          myState = State::PROCESS;
        } break;
        
        default:
          break;
      }
    }
  }
};

struct ProperStateMachine {
  float *coeffPtr;
  int *executionCount;
  bool initCoeffBeforeStarting = false;
  int iterations;

  void operator()() const {
    State myState =
        (initCoeffBeforeStarting) ? State::LD_COEFF : State::PROCESS;
    float coeff = 1.0f;

    sycl::ext::intel::experimental::task_sequence<updateCoeff> updateCoeffTask;
    sycl::ext::intel::experimental::task_sequence<compute> computeTask;

    // [[intel::initiation_interval(1)]]
    for (int i = 0; i < iterations; i++) {
      switch (myState) {
        case State::PROCESS: {
          MyStreamingBeat beat = StreamIn_ProperStateMachine::read();
          // use task_sequence to hide long II from compiler
          computeTask.async(coeff, beat.data);
          beat.data = computeTask.get();
          StreamOut_ProperStateMachine::write(beat);
        } break;

        case State::LD_COEFF:
          // use task_sequence to hide long II from compiler
          updateCoeffTask.async(coeffPtr, executionCount);
          coeff = updateCoeffTask.get();
          myState = State::PROCESS;
          break;

        default:
          break;
      }
    }
  }
};

#endif    // STATE_MACHINE_HPP
