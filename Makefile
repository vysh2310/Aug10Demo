CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

BIN_DIR := bin
SRCS := $(filter-out car_media_polyspace_demo_pstunit.cpp,$(wildcard *.cpp))
TARGETS := $(patsubst %.cpp,$(BIN_DIR)/%,$(SRCS))

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/%: %.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
