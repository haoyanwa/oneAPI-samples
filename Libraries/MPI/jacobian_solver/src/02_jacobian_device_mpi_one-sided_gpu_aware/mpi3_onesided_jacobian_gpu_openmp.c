/*==============================================================
 * Copyright © 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 * ============================================================= */

/* Distributed Jacobian computation sample using OpenMP GPU offload and MPI-3 one-sided.
 */

#include "../include/common.h"
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    double t_start;
    struct subarray my_subarray = { };
    /* Here we use double buffering to allow the overlap of the compute and communication phases.
     * Odd iterations use buffs[0] as input and buffs[1] as output, and vice versa.
     * The same scheme is used for MPI_Win objects.
     */
    double *buffs[2] = { NULL, NULL };
    MPI_Win win[2] = { MPI_WIN_NULL, MPI_WIN_NULL };

    /* Initialization of runtime and initial state of data. */
    MPI_Init(&argc, &argv);
    /* Initialize the subarray owned by the current process
     * and create RMA windows for MPI-3 one-sided communications.
     *  - For this sample, we use GPU memory for buffers and windows.
     *  - This sample uses MPI_Win_fence for synchronization.
     */
    InitSubarryAndWindows(&my_subarray, buffs, win, "device", false);

    /* Start the RMA exposure epoch. */   
    MPI_Win_fence(0, win[0]);
    MPI_Win_fence(0, win[1]);

    const int row_size = ROW_SIZE(my_subarray);
    /* Number of iterations to perform between norm calculations. */
    const int iterations_batch = (NormIteration <= 0) ? Niter : NormIteration;
    /* Auxiliary variables used to let OMP capture pointers. */
    double *b1 = buffs[0], *b2 = buffs[1];

    /* Timestamp the start time to measure overall execution time. */
    BEGIN_PROFILING
    /* Main computation loop partly offloaded to the device:
     * "#pragma omp target data" maps the data to the device memory for the following code region. */
    #pragma omp target data map(to: my_subarray) use_device_ptr(b1,b2)
    {
        for (int passed_iters = 0; passed_iters < Niter; passed_iters += iterations_batch) {
            /* Perform a batch of iterations before checking the norm. */
            for (int k = 0; k < iterations_batch; ++k) {
                /* Double buffering: pick buffer and MPI_Win based on the iteration. */
                int i = passed_iters + k;
                double *in = (i % 2) ? b1 : b2;
                double *out = ((1 + i) % 2) ? b1 : b2;
                MPI_Win current_win = win[(i + 1) % 2];

                /* Offload compute loop to the device:
                 * "#pragma omp target" offloads the code to the device 
                 * "#pragma omp parallel for" parallelizes the loop on the device using a single team.
                 *
                 * NOTE: For simplification and unification across samples, we use a single team
                 *       to avoid extra synchronization across teams in the future. */ 
                #pragma omp target is_device_ptr(in, out) thread_limit(1024)
                #pragma omp parallel loop
                /* Calculate values on the borders to initiate communications early. */
                for (int column = 0; column < my_subarray.x_size; ++column) {
                    RECALCULATE_POINT(out, in, column, 0, row_size);
                    RECALCULATE_POINT(out, in, column, my_subarray.y_size - 1, row_size);
                }

                /* Perform 1D halo-exchange with neighbors. */
                if (my_subarray.up_neighbour != MPI_PROC_NULL) {
                    int idx = XY_2_IDX(0, 0, row_size);
                    MPI_Put(&out[idx], my_subarray.x_size, MPI_DOUBLE,
                            my_subarray.up_neighbour, my_subarray.l_nbh_offt,
                            my_subarray.x_size, MPI_DOUBLE, current_win);
                }

                if (my_subarray.dn_neighbour != MPI_PROC_NULL) {
                    int idx = XY_2_IDX(0, my_subarray.y_size - 1, row_size);
                    MPI_Put(&out[idx], my_subarray.x_size, MPI_DOUBLE,
                            my_subarray.dn_neighbour, 1,
                            my_subarray.x_size, MPI_DOUBLE, current_win);
                }

                /* Offload compute loop to the device. */
                #pragma omp target is_device_ptr(in, out) thread_limit(1024)
                #pragma omp parallel loop collapse(2)
                /* Recalculate internal points in parallel with communication. */
                for (int row = 1; row < my_subarray.y_size - 1; ++row) {
                    for (int column = 0; column < my_subarray.x_size; ++column) {
                        RECALCULATE_POINT(out, in, column, row, row_size);
                    }
                }

                /* Ensure all communications are complete before the next iteration. */
                MPI_Win_fence(0, current_win);
            }

            /* Calculate the norm value after the given number of iterations. */
            if (NormIteration > 0) {
                double result_norm = 0.0;
                double norm = 0.0;

                #pragma omp target teams distribute parallel for is_device_ptr(b1, b2) reduction(+:norm) collapse(2)
                for (int row = 0; row < my_subarray.y_size; ++row) {
                    for (int column = 0; column < my_subarray.x_size; ++column) {
                        int idx = XY_2_IDX(column, row, row_size);
                        double diff = b1[idx] - b2[idx];
                        norm += diff*diff;
                    }
                }
                MPI_Reduce(&norm, &result_norm, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
                if (my_subarray.rank == 0) {
                    printf("NORM value on iteration %d: %f\n", passed_iters+iterations_batch, sqrt(result_norm));
                }
            }
        }
    }
    /* Timestamp the end time to measure overall execution time and report average compute time. */
    END_PROFILING

    /* Close the RMA exposure epoch and free resources. */
    MPI_Win_fence(0, win[0]);
    MPI_Win_fence(0, win[1]);
    MPI_Win_free(&win[1]);
    MPI_Win_free(&win[0]);

    if (my_subarray.rank == 0) {
        printf("SUCCESS\n");
    }
    MPI_Finalize();

    return 0;
}