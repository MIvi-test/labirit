# ============================================================================
# Cross-platform Makefile for Maze Generation Algorithms
# Supports: Windows (MinGW/MSVC), Linux, macOS
# ============================================================================

# Platform detection
ifeq ($(OS),Windows_NT)
  PLATFORM := windows
  EXEEXT := .exe
  PATHSEP := \\
  RM := del /Q
  MKDIR := mkdir
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME),Linux)
    PLATFORM := linux
  endif
  ifeq ($(UNAME),Darwin)
    PLATFORM := macos
  endif
  EXEEXT :=
  PATHSEP := /
  RM := rm -f
  MKDIR := mkdir -p
endif

# Compiler and flags
CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c23 -g
LDFLAGS ?=
LDLIBS ?= -lm

# Platform-specific optimizations
ifeq ($(PLATFORM),windows)
  # MinGW specific flags
  CFLAGS += -D_WIN32_WINNT=0x0601
  LDLIBS += -lgdi32 -luser32
else ifeq ($(PLATFORM),linux)
  # Linux specific flags
  CFLAGS += -D_GNU_SOURCE
  LDLIBS += -ldl
else ifeq ($(PLATFORM),macos)
  # macOS specific flags
  CFLAGS += -D__APPLE__
endif

# Targets
TARGET := main$(EXEEXT)
BENCH_TARGET := benchmarks$(EXEEXT)
TEST_TARGET := tests_run$(EXEEXT)

BACKEND_SRCS := \
	backend/metrics.c \
	backend/table_init.c \
	backend/dfs_algorithm.c \
	backend/binary_tree_alg.c \
	backend/recursive_division_algorithm.c \
	backend/prim.c \
	backend/watson_alg.c \
	backend/growing_tree_alg.c

FRONTEND_SRCS := frontend/graph.c
COMMON_HEADERS := $(wildcard *.h backend/*.h frontend/*.h analisys/*.h)

MAIN_SRCS := main.c json_parser.c $(BACKEND_SRCS) $(FRONTEND_SRCS)
BENCH_SRCS := analisys/benchmarks_main.c analisys/combined_benchmarks.c $(BACKEND_SRCS)
TEST_SRCS := tests/test_algorithms.c $(BACKEND_SRCS)

all: compile

$(TARGET): $(MAIN_SRCS) $(COMMON_HEADERS)
	$(CC) $(CFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BENCH_TARGET): $(BENCH_SRCS) $(COMMON_HEADERS)
	$(CC) $(CFLAGS) $(BENCH_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TEST_TARGET): $(TEST_SRCS) $(COMMON_HEADERS)
	$(CC) $(CFLAGS) $(TEST_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

compile: $(TARGET)

test: $(TEST_TARGET)
	$(TARGET)

run: $(TARGET)
	$(TARGET)
	$(TARGET) --dump

run-benchmarks: $(BENCH_TARGET)
	$(BENCH_TARGET)

clean:
	-$(RM) $(TARGET) $(BENCH_TARGET) $(TEST_TARGET)
# 	-$(RM) analisys$(PATHSEP)json_data$(PATHSEP)*.json
# 	-$(RM) analisys$(PATHSEP)json_data$(PATHSEP)examples$(PATHSEP)*.json

# Platform-specific targets
show-platform:
	@echo Platform: $(PLATFORM)
	@echo Executable extension: $(EXEEXT)
	@echo Path separator: $(PATHSEP)

.PHONY: all compile test run clean run-benchmarks show-platform
