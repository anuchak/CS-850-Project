HIP_PATH?= /opt/rocm/hip
HIPCC=$(HIP_PATH)/bin/hipcc
BIN_DIR?= ./bin
decompress: $(BIN_DIR)/decompress
$(BIN_DIR)/decompress: test_gpu_decompress.cpp $(BIN_DIR)
	$(HIPCC) --amdgpu-target=gfx801,gfx803,gfx900 $(CXXFLAGS) test_gpu_decompress.cpp -o $(BIN_DIR)/decompress
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
clean:
	rm -rf $(BIN_DIR)
.PHONY: decompress clean

