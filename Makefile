#INFO := $(shell cd src/goldilocks && ./configure.sh && cd ../.. && sleep 2)
#include src/goldilocks/CudaArch.mk
NVCC := /usr/local/cuda/bin/nvcc

TARGET_ZKP := zkProver
TARGET_ZKP_GPU := zkProver
TARGET_BCT := bctree
TARGET_MNG := mainGenerator
TARGET_MNG_10 := mainGenerator10
TARGET_PLG := polsGenerator
TARGET_PLD := polsDiff
TARGET_TEST := zkProverTest
TARGET_W2DB := witness2db

BUILD_DIR := ./build
BUILD_DIR_GPU := ./build-gpu
SRC_DIRS := ./src ./test ./tools

GRPCPP_FLAGS := $(shell pkg-config grpc++ --cflags)
GRPCPP_LIBS := $(shell pkg-config grpc++ --libs) -lgrpc++_reflection
ifndef GRPCPP_LIBS
$(error gRPC++ could not be found via pkg-config, you need to install them)
endif

CXX := g++
AS := nasm
CXXFLAGS := -std=c++17 -Wall -pthread -flarge-source-files -Wno-unused-label -rdynamic -mavx2 $(GRPCPP_FLAGS) #-Wfatal-errors

LDFLAGS_GPU := -lprotobuf -lsodium -lgpr -lpthread -lpqxx -lpq -lgmp -lstdc++ -lgmpxx -lsecp256k1 -lcrypto -luuid -liomp5 $(GRPCPP_LIBS)
LDFLAGS := $(LDFLAGS_GPU) -fopenmp
CXXFLAGS_W2DB := -std=c++17 -Wall -pthread -flarge-source-files -Wno-unused-label -rdynamic -mavx2
LDFLAGS_W2DB := -lgmp -lstdc++ -lgmpxx

CFLAGS := -fopenmp
ASFLAGS := -felf64

# Debug build flags
ifeq ($(dbg),1)
      CXXFLAGS += -g -D DEBUG
else
      CXXFLAGS += -O3
endif

ifdef PROVER_FORK_ID
	  CXXFLAGS += -DPROVER_FORK_ID=$(PROVER_FORK_ID)
endif

# Verify if AVX-512 is supported
# for now disabled, to enable it, you only need to uncomment these lines
#AVX512_SUPPORTED := $(shell cat /proc/cpuinfo | grep -E 'avx512' -m 1)

#ifneq ($(AVX512_SUPPORTED),)
#	CXXFLAGS += -mavx512f -D__AVX512__
#endif

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(sort $(dir))
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

SRCS_ZKP := $(shell find $(SRC_DIRS) ! -path "./tools/starkpil/bctree/*" ! -path "./test/prover/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/tests/*" ! -path "./src/main_generator/*" ! -path "./src/pols_generator/*" ! -path "./src/pols_diff/*" ! -path "./src/witness2db/*" -name *.cpp -or -name *.c -or -name *.asm -or -name *.cc)

SRCS_ZKP_GPU := $(shell find $(SRC_DIRS) ! -path "./tools/starkpil/bctree/*" ! -path "./test/prover/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/tests/*" ! -path "./src/main_generator/*" ! -path "./src/pols_generator/*" ! -path "./src/pols_diff/*" ! -path "./src/goldilocks/utils/timer.cpp" -name *.cpp -or -name *.c -or -name *.asm -or -name *.cc -or -name *.cu ! -path "./src/goldilocks/utils/deviceQuery.cu" ! -path "./src/goldilocks/tests/*.cu")

OBJS_ZKP := $(SRCS_ZKP:%=$(BUILD_DIR)/%.o)
OBJS_ZKP_GPU := $(SRCS_ZKP_GPU:%=$(BUILD_DIR_GPU)/%.o)
DEPS_ZKP := $(OBJS_ZKP:.o=.d)

SRCS_BCT := ./tools/starkpil/bctree/build_const_tree.cpp ./tools/starkpil/bctree/main.cpp ./src/goldilocks/src/goldilocks_base_field.cpp ./src/ffiasm/fr.cpp ./src/ffiasm/fr.asm ./src/starkpil/merkleTree/merkleTreeBN128.cpp ./src/poseidon_opt/poseidon_opt.cpp ./src/goldilocks/src/poseidon_goldilocks.cpp
OBJS_BCT := $(SRCS_BCT:%=$(BUILD_DIR)/%.o)
DEPS_BCT := $(OBJS_BCT:.o=.d)

SRCS_TEST := $(shell find $(SRC_DIRS) ! -path "./src/main.cpp" ! -path "./tools/starkpil/bctree/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/benchs/*" ! -path "./src/goldilocks/tests/*" ! -path "./src/main_generator/*" ! -path "./src/pols_generator/*" ! -path "./src/pols_diff/*" ! -path "./src/witness2db/*" -name *.cpp -or -name *.c -or -name *.asm -or -name *.cc)
OBJS_TEST := $(SRCS_TEST:%=$(BUILD_DIR)/%.o)
DEPS_TEST := $(OBJS_TEST:.o=.d)


SRCS_W2DB := ./src/witness2db/witness2db.cpp  ./src/goldilocks/src/goldilocks_base_field.cpp ./src/goldilocks/src/poseidon_goldilocks.cpp
OBJS_W2DB := $(SRCS_W2DB:%=$(BUILD_DIR)/%.o)
DEPS_W2DB := $(OBJS_W2DB:.o=.d)

cpu: $(BUILD_DIR)/$(TARGET_ZKP)
gpu: $(BUILD_DIR_GPU)/$(TARGET_ZKP_GPU)

bctree: $(BUILD_DIR)/$(TARGET_BCT)

test: $(BUILD_DIR)/$(TARGET_TEST)

$(BUILD_DIR)/$(TARGET_ZKP): $(OBJS_ZKP)
	$(CXX) $(OBJS_ZKP) $(CXXFLAGS) -o $@ $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR_GPU)/$(TARGET_ZKP_GPU): $(OBJS_ZKP_GPU)
	$(NVCC) $(OBJS_ZKP_GPU) -O3 -arch=$(CUDA_ARCH) -o $@ $(LDFLAGS_GPU)

$(BUILD_DIR)/$(TARGET_BCT): $(OBJS_BCT)
	$(CXX) $(OBJS_BCT) $(CXXFLAGS) -o $@ $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)

$(BUILD_DIR)/$(TARGET_TEST): $(OBJS_TEST)
	$(CXX) $(OBJS_TEST) $(CXXFLAGS) -o $@ $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.asm.o: %.asm
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# assembly
$(BUILD_DIR_GPU)/%.asm.o: %.asm
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# c++ source
$(BUILD_DIR_GPU)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) -D__USE_CUDA__ $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR_GPU)/%.cc.o: %.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) -D__USE_CUDA__ $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# cuda source
$(BUILD_DIR_GPU)/%.cu.o: %.cu
	$(MKDIR_P) $(dir $@)
	$(NVCC) -D__USE_CUDA__ -Isrc/goldilocks/utils -Xcompiler -fopenmp -Xcompiler -fPIC -Xcompiler -mavx2 -Xcompiler -O3 -O3 -arch=$(CUDA_ARCH) -O3 $< -dc --output-file $@

main_generator: $(BUILD_DIR)/$(TARGET_MNG)

$(BUILD_DIR)/$(TARGET_MNG): ./src/main_generator/main_generator.cpp ./src/config/definitions.hpp
	$(MKDIR_P) $(BUILD_DIR)
	g++ -g ./src/main_generator/main_generator.cpp -o $@ -lgmp

main_generator_10: $(BUILD_DIR)/$(TARGET_MNG_10)

$(BUILD_DIR)/$(TARGET_MNG_10): ./src/main_generator/main_generator_10.cpp ./src/config/definitions.hpp
	$(MKDIR_P) $(BUILD_DIR)
	g++ -g $(CXXFLAGS) ./src/main_generator/main_generator_10.cpp ./src/config/fork_info.cpp -o $@ -lgmp

generate: main_generator main_generator_10
	$(BUILD_DIR)/$(TARGET_MNG) all
	$(BUILD_DIR)/$(TARGET_MNG_10) all

pols_generator: $(BUILD_DIR)/$(TARGET_PLG)

$(BUILD_DIR)/$(TARGET_PLG): ./src/pols_generator/pols_generator.cpp ./src/config/definitions.hpp
	$(MKDIR_P) $(BUILD_DIR)
	g++ -g ./src/pols_generator/pols_generator.cpp -o $@ -lgmp

pols: pols_generator
	$(BUILD_DIR)/$(TARGET_PLG)

pols_diff: $(BUILD_DIR)/$(TARGET_PLD)

$(BUILD_DIR)/$(TARGET_PLD): ./src/pols_diff/pols_diff.cpp
	$(MKDIR_P) $(BUILD_DIR)
	g++ -g ./src/pols_diff/pols_diff.cpp $(CXXFLAGS) $(INC_FLAGS) -o $@ $(LDFLAGS) 

witness2db: $(BUILD_DIR)/$(TARGET_W2DB)

$(BUILD_DIR)/$(TARGET_W2DB): $(OBJS_W2DB)
	$(CXX) $(OBJS_W2DB) $(CXXFLAGS_W2DB) -o $@ $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS_W2DB) $(LDFLAGS_W2DB)

.PHONY: clean

clean:
	$(RM) -rf $(BUILD_DIR)
	$(RM) -rf $(BUILD_DIR_GPU)
	find . -name main_exec_generated*pp -delete

-include $(DEPS_ZKP)
-include $(DEPS_BCT)

MKDIR_P ?= mkdir -p
