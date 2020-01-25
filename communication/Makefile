CXX = g++
CXXFLAGS = -std=c++17 -g #-Wall #-Werror
LDFLAGS = -lrplidar_sdk -lpthread
MKDIR = mkdir -p
CXXFLAGS += -w


# SOURCES:
# Add all sources that should be compiled here.
# Each source listed must have a file of the same name ending with .cpp in the src/ directory
SOURCES = main communication module sensor steering serial socket map rplidar pc

# TESTS:
# Add all tests that should be compiled when running 'make tests' here.
# All tests must end with '_test'
# Each test listed must have a file of the same name ending with .cpp in the src/ directory
TESTS = map_test socket_test rplidar_test debug_rplidar_test positioning_test pc_test icp_test
SIMULATIONS = simulation_test centroid_test vectorization_test print_data


TARGET = communication
INC_DIR = include
LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
VND_DIR = vendor
TST_DIR = tests
SIM_DIR = simulation

INCS = -I$(INC_DIR)
LIBS = -L$(LIB_DIR)
SRCS = $(patsubst %,$(SRC_DIR)/%.cpp,$(SOURCES))
OBJS = $(patsubst %,$(OBJ_DIR)/%.o,$(SOURCES))
TSTS = $(patsubst %,$(TST_DIR)/%,$(TESTS))
SIM = $(patsubst %,$(SIM_DIR)/%,$(SIMULATIONS))

.PHONY: directories clean


all: directories $(BIN_DIR)/$(TARGET)


directories: 
	$(MKDIR) $(INC_DIR) $(LIB_DIR) $(BIN_DIR) $(SRC_DIR) $(OBJ_DIR) $(VND_DIR) $(TST_DIR)


$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(LIBS) $(OBJS) $(LDFLAGS) -o $(BIN_DIR)/$(TARGET)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCS) $(LIBS) $(LDFLAGS) -c $< -o $@


tests: $(TSTS)


$(TST_DIR)/%_test: $(TST_DIR)/%_test.cpp $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
	$(CXX) $(CXXFLAGS) $(INCS) $(LIBS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(LDFLAGS) $< -o $@


sim: $(SIM)

$(SIM_DIR)/%: $(SIM_DIR)/%.cpp $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
	$(CXX) $(CXXFLAGS) $(INCS) $(LIBS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $(LDFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(BIN_DIR)/$(TARGET) $(TSTS)
