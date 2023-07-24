/*
Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stdio.h>
#include "hip/hip_runtime.h"

#define CHECK(cmd) \
{\
    hipError_t error  = cmd;\
    if (error != hipSuccess) { \
      fprintf(stderr, "error: '%s'(%d) at %s:%d\n", hipGetErrorString(error), error,__FILE__, __LINE__); \
    exit(EXIT_FAILURE);\
    }\
}
/*
 * Compute murmur64 hash of each element in the array A and write to array C.
 */
template <typename T>
__global__ void
hashCompute(T *C_d, const T *A_d, size_t N)
{
    size_t offset = (hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x);
    size_t stride = hipBlockDim_x * hipGridDim_x ;

    for (size_t i=offset; i<N; i+=stride) {
        C_d[i] = A_d[i];
        C_d[i] ^= C_d[i] >> 32;
        C_d[i] *= 0xd6e8feb86659fd93U;
        C_d[i] ^= C_d[i] >> 32;
        C_d[i] *= 0xd6e8feb86659fd93U;
        C_d[i] ^= C_d[i] >> 32;
    }
}
int main(int argc, char *argv[])
{
    uint64_t *A_h, *C_h;
    size_t N = 1000000;
    size_t Nbytes = N * sizeof(uint64_t);
    hipDeviceProp_t props;
    CHECK(hipGetDeviceProperties(&props, 0/*deviceID*/));
    printf ("info: running on device %s\n", props.name);
    #ifdef __HIP_PLATFORM_HCC__
      printf ("info: architecture on AMD GPU device is: %d\n",props.gcnArch);
    #endif
    printf ("info: allocate host and device mem (%6.2f MB)\n", 2*Nbytes/1024.0/1024.0);
    CHECK(hipHostMalloc(&A_h, Nbytes));
    CHECK(hipHostMalloc(&C_h, Nbytes));
    // Fill with Phi + i
    for (size_t i=0; i<N; i++)
    {
        A_h[i] = 4690518244598124380;
    }
    const unsigned blocks = 512;
    const unsigned threadsPerBlock = 256;
    printf ("info: launch 'hashCompute' kernel\n");
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    hipLaunchKernelGGL(hashCompute, dim3(blocks), dim3(threadsPerBlock), 0, 0, C_h, A_h, N);
    hipDeviceSynchronize();
    auto duration1 = std::chrono::system_clock::now().time_since_epoch();
    auto millis1 = std::chrono::duration_cast<std::chrono::microseconds>(duration1).count();
    printf("total time taken to run kernel in microseconds: %lu microsec\n", (millis1 - millis));
    return 0;
}
