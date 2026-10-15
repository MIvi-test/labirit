.DEFAULT_GOAL := all
CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c23 -g
CPPFLAGS ?= -D_GNU_SOURCE
METRICS ?= 1
override CPPFLAGS += -DLABIRIT_METRICS_ENABLED=$(METRICS)
LDFLAGS ?=
LDLIBS ?= -lm -ldl

TARGET := main
BENCH_TARGET := benchmarks
TEST_TARGET := tests_run
TOPOLOGY_TEST_TARGET := topology_tests_run

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

METRICS_CONFIG := .metrics-config
.PHONY: FORCE
FORCE:

$(METRICS_CONFIG): FORCE
	@printf '%s\n' '$(METRICS)' | cmp -s - $@ || printf '%s\n' '$(METRICS)' > $@

# Default behavior: build and run the maze generator.
all: run

$(TARGET): $(METRICS_CONFIG) $(MAIN_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BENCH_TARGET): $(METRICS_CONFIG) $(BENCH_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread $(BENCH_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TEST_TARGET): $(METRICS_CONFIG) $(TEST_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TEST_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(TOPOLOGY_TEST_TARGET): $(METRICS_CONFIG) tests/test_topology_threads.c analisys/combined_benchmarks.c $(BACKEND_SRCS) $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread tests/test_topology_threads.c $(BACKEND_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

compile: $(TARGET)

run: $(TARGET)
	./$(TARGET)

benchmark: $(BENCH_TARGET)
	./$(BENCH_TARGET)

test: $(TEST_TARGET) $(TOPOLOGY_TEST_TARGET)
	./$(TEST_TARGET)
	./$(TOPOLOGY_TEST_TARGET)

clean:
	rm -f $(TARGET) $(BENCH_TARGET) $(TEST_TARGET) $(TOPOLOGY_TEST_TARGET) $(METRICS_CONFIG)

.PHONY: all compile run benchmark test clean
