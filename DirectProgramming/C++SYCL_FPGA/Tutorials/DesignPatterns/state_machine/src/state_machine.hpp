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
class IDOptimizedStateMachine;
class IDStreamDataIn_NaiveStateMachine;
class IDStreamDataIn_OptimizedStateMachine;
class IDStreamDataOut_NaiveStateMachine;
class IDStreamDataOut_OptimizedStateMachine;

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
using StreamIn_OptimizedStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataIn_OptimizedStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>; 
using StreamOut_NaiveStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataOut_NaiveStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>;
using StreamOut_OptimizedStateMachine =
    sycl::ext::intel::experimental::pipe<IDStreamDataOut_OptimizedStateMachine, MyStreamingBeat, 0,
                                         StreamingPipeProps>;

// function for task_sequence
float UpdateCoeff(float *coeff_ptr, int *execution_count) {
  float coeff_new = coeff_ptr[0];
  // read-modify-write of external memory will result in a poor II.
  execution_count[0] = execution_count[0] + 1;
  return coeff_new;
}

// function for task_sequence
float Compute(float coeff, float data) {
  return coeff * data;
}

struct NaiveStateMachine {
  float *coeff_ptr;
  int *execution_count;
  bool init_coeff_before_starting = false;
  int iterations;

  void operator()() const {
    State my_state = 
        (init_coeff_before_starting) ? State::LD_COEFF : State::PROCESS;
    float coeff = 1.0f;

    for (int i = 0; i < iterations; i++) {
      switch (my_state) {
        case State::PROCESS: {
          MyStreamingBeat beat = StreamIn_NaiveStateMachine::read();
          // multiplication of input data with a coefficient can occur with II=1
          beat.data = beat.data * coeff;
          StreamOut_NaiveStateMachine::write(beat);
        } break;
        
        case State::LD_COEFF: {
          float coeff_new = coeff_ptr[0];
          execution_count[0] = execution_count[0] + 1;
          coeff = coeff_new;
          my_state = State::PROCESS;
        } break;
        
        default:
          break;
      }
    }
  }
};

struct OptimizedStateMachine {
  float *coeff_ptr;
  int *execution_count;
  bool init_coeff_before_starting = false;
  int iterations;

  void operator()() const {
    State my_state =
        (init_coeff_before_starting) ? State::LD_COEFF : State::PROCESS;
    float coeff = 1.0f;

    sycl::ext::intel::experimental::task_sequence<UpdateCoeff> update_coeff_task;
    sycl::ext::intel::experimental::task_sequence<Compute> compute_task;

    [[intel::initiation_interval(1)]] // No-format: Attribute
    for (int i = 0; i < iterations; i++) {
      switch (my_state) {
        case State::PROCESS: {
          MyStreamingBeat beat = StreamIn_OptimizedStateMachine::read();
          // use task_sequence to hide long II from compiler
          compute_task.async(coeff, beat.data);
          beat.data = compute_task.get();
          StreamOut_OptimizedStateMachine::write(beat);
        } break;

        case State::LD_COEFF:
          // use task_sequence to hide long II from compiler
          update_coeff_task.async(coeff_ptr, execution_count);
          coeff = update_coeff_task.get();
          my_state = State::PROCESS;
          break;

        default:
          break;
      }
    }
  }
};

#endif    // STATE_MACHINE_HPP
