#!/bin/bash
source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1
/bin/echo "##" $(whoami) is compiling SYCL_Essentials Module7 -- oneDPL Extension APIs - 9 of 12 minimum_function.cpp
icpx -fsycl lab/minimum_function.cpp -w -o bin/minimum_function
bin/minimum_function
