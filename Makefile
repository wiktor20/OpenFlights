CXX       := g++
CXX_FLAGS := -std=c++17

BIN_DIR  	:= bin
SRC_DIR  	:= src

TARGETS 	:= PortRank Dijkstra


all : $(TARGETS)

clean:
		@echo "🧹 Emptying bin"
		-rm -rf $(BIN_DIR)/*
		touch $(BIN_DIR)/binaries_here

run:
		@echo "🚀 Running..."
		$(addprefix ./$(BIN_DIR)/, $(TARGETS))
		
$(TARGETS) : $(SRCS_O)
		@echo "🏗 Building $@"
		$(CXX) $(CXX_FLAGS) -o $(BIN_DIR)/$@ $(SRC_DIR)/$@.cpp

.PHONY: clean run
