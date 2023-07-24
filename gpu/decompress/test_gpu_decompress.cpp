#include "hip/hip_runtime.h"
#include <chrono>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct packed_metadata {
  uint8_t prev_idx;          // 7 bits
  uint8_t significant_bytes; // 3 bits
  uint8_t trailing_zeros;    // 6 bits
} hipLaunchKernelStruct_1;

#define CHECK(cmd)                                                             \
  {                                                                            \
    hipError_t error = cmd;                                                    \
    if (error != hipSuccess) {                                                 \
      fprintf(stderr, "error: '%s'(%d) at %s:%d\n", hipGetErrorString(error),  \
              error, __FILE__, __LINE__);                                      \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  }

__global__ void decompress_func(hipLaunchKernelStruct_1 *packedMetadata,
                                uint8_t *byte_input, uint64_t *byte_output,
                                size_t total_column_segments,
                                size_t column_segment_size) {
  size_t offset = (hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x) % total_column_segments;
  size_t final_idx = max(total_column_segments * column_segment_size, (offset + 1) * column_segment_size);
  
  uint64_t prevVal = 0u;
  uint64_t index = 0u;
  for (auto i = offset * column_segment_size; i < final_idx; i++) {
    auto metadata = packedMetadata[i];
    uint64_t temp = 0;
    switch (metadata.significant_bytes) {
    case 1:
      memcpy(&temp, byte_input + index, 1);
      index += 1;
      break;
    case 2:
      memcpy(&temp, byte_input + index, 2);
      index += 2;
      break;
    case 3:
      memcpy(&temp, byte_input + index, 3);
      index += 3;
      break;
    case 4:
      memcpy(&temp, byte_input + index, 4);
      index += 4;
      break;
    case 5:
      memcpy(&temp, byte_input + index, 5);
      index += 5;
      break;
    case 6:
      memcpy(&temp, byte_input + index, 6);
      index += 6;
      break;
    case 7:
      memcpy(&temp, byte_input + index, 7);
      index += 7;
      break;
    default:
      if (metadata.trailing_zeros < 8) {
        memcpy(&temp, byte_input + index, 8);
        index += 8;
        break;
      }
      temp = 0;
    }
    temp = (temp << metadata.trailing_zeros) ^ prevVal;
    prevVal = temp;
    byte_output[i] = temp;
  }
}

int main() {
  hipLaunchKernelStruct_1 *packedMetadata;
  uint8_t *byte_input;
  uint64_t *byte_output;
  size_t input_N = 998250;
  size_t input_NBytes = input_N * sizeof(uint8_t);
  size_t output_N = 363000;
  size_t output_NBytes = output_N * sizeof(uint64_t);
  hipDeviceProp_t props;
  CHECK(hipGetDeviceProperties(&props, 0 /*deviceID*/));
  printf("info: running on device %s\n", props.name);
#ifdef __HIP_PLATFORM_HCC__
  printf("info: architecture on AMD GPU device is: %d\n", props.gcnArch);
#endif
  CHECK(hipHostMalloc(&byte_input, input_NBytes));
  CHECK(hipHostMalloc(&byte_output, output_NBytes));
  CHECK(hipHostMalloc(&packedMetadata,
                      output_N * sizeof(hipLaunchKernelStruct_1)));
  printf("starting initialization of input arrays ...\n");
  for (auto i = 0u; i < 90750; i++) {
    uint64_t val = 4626885667169763328;
    packedMetadata[4*i] = {0, 0, 0};
    hipMemcpy(byte_input + 11 * i, &val, 8, hipMemcpyDefault);
    val = 3;
    packedMetadata[4*i + 1] = {1, 1, 48};
    hipMemcpy(byte_input + 11 * i + 8, &val, 1, hipMemcpyDefault);
    val = 231;
    packedMetadata[4*i + 2] = {1, 1, 47};
    hipMemcpy(byte_input + 11 * i + 9, &val, 1, hipMemcpyDefault);
    val = 29;
    packedMetadata[4*i + 3] = {1, 1, 47};
    hipMemcpy(byte_input + 11 * i + 10, &val, 1, hipMemcpyDefault);
    printf("finished %lu \n", i);
  }
  const unsigned blocks = 512;
  const unsigned threadsPerBlock = 512;
  printf("info: launching kernel decompress ...\n");
  auto duration = std::chrono::system_clock::now().time_since_epoch();
  auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  hipLaunchKernelGGL(decompress_func, dim3(blocks), dim3(threadsPerBlock), 0, 0,
                     packedMetadata, byte_input, byte_output, output_N);
  hipDeviceSynchronize();
  auto duration1 = std::chrono::system_clock::now().time_since_epoch();
  auto millis1 = std::chrono::duration_cast<std::chrono::microseconds>(duration1).count();
  printf("total time taken to run kernel in microseconds: %lu microsec\n", (millis1 - millis));
  return 0;
}
