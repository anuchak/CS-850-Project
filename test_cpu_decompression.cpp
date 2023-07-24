#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct packed_metadata {
  uint8_t prev_idx;          // 7 bits
  uint8_t significant_bytes; // 3 bits
  uint8_t trailing_zeros;    // 6 bits
};

int main() {
  packed_metadata packedMetadata[4]{
      {0, 0, 0}, {1, 1, 48}, {1, 1, 47}, {1, 1, 47}};
  uint8_t *byte_input = (uint8_t *)malloc(998250 * sizeof(uint8_t));
  uint64_t *byte_output = (uint64_t *)malloc(363000 * sizeof(uint64_t));
  for (int i = 0; i < 90750; i++) {
    uint64_t val = 4626885667169763328;
    memcpy(byte_input + 11 * i, &val, sizeof(val));
    val = 3;
    memcpy(byte_input + 11 * i + 8, &val, 1);
    val = 231;
    memcpy(byte_input + 11 * i + 9, &val, 1);
    val = 29;
    memcpy(byte_input + 11 * i + 10, &val, 1);
  }
  uint64_t prevVal = 0u;
  uint64_t index = 0u;
  auto duration = std::chrono::system_clock::now().time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
  for (int i = 0; i < 363000; i++) {
    auto metadata = packedMetadata[i % 4];
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
      byte_output[i] = temp;
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
  auto duration1 = std::chrono::system_clock::now().time_since_epoch();
  auto millis1 =
      std::chrono::duration_cast<std::chrono::microseconds>(duration1).count();
  printf("total time taken to run cpu code in microseconds: %lu microsec\n",
         (millis1 - millis));
}
