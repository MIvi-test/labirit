CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c23 -g
CPPFLAGS ?= -D_GNU_SOURCE
LDFLAGS ?=
LDLIBS ?= -lm -ldl

TARGET := main
BENCH_TARGET := benchmarks
TEST_TARGET := tests_run

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

# Default behavior: build and run the maze generator.
all: run

$(TARGET): $(MAIN_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BENCH_TARGET): $(BENCH_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BENCH_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TEST_TARGET): $(TEST_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

compile: $(TARGET)

run: $(TARGET)
	./$(TARGET)

benchmark: $(BENCH_TARGET)
	./$(BENCH_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(BENCH_TARGET) $(TEST_TARGET)

.PHONY: all compile run benchmark test clean
