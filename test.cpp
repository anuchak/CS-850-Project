#include <stdio.h>
#include <chrono>
#include <stdlib.h>
#include <omp.h>

int main() {
    // ADDING CPU CODE FOR TESTING
    size_t N = 211520000;
    uint64_t *input = (uint64_t *) malloc(N * sizeof(uint64_t));
    uint64_t *output = (uint64_t *) malloc(N * sizeof(uint64_t));
    for (size_t i=0; i<N; i++) {
      input[i] = 4690518244598124380;
    }

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    #pragma omp parallel for
    for(int i = 0; i < N; i++) {
      output[i] = input[i];
      output[i] ^= output[i] >> 32;
      output[i] *= 0xd6e8feb86659fd93U;
      output[i] ^= output[i] >> 32;
      output[i] *= 0xd6e8feb86659fd93U;
      output[i] ^= output[i] >> 32;
    }

    auto duration1 = std::chrono::system_clock::now().time_since_epoch();
    auto millis1 = std::chrono::duration_cast<std::chrono::microseconds>(duration1).count();
    printf("total time taken to run cpu code in microseconds: %lu microsec\n", (millis1 - millis));
    return 0;
}

