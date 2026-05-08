CC ?= cc
CFLAGS ?= -Wall -Wextra -Werror -std=c23 -g
LDFLAGS ?=
LDLIBS ?= -lm

ifeq ($(OS),Windows_NT)
  EXEEXT := .exe
else
  EXEEXT :=
endif

TARGET := main$(EXEEXT)
BENCH_TARGET := benchmarks$(EXEEXT)
TEST_TARGET := tests_run$(EXEEXT)

RM ?= rm -f

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
	./$(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)
	./$(TARGET) --dump

run-benchmarks: $(BENCH_TARGET)
	./$(BENCH_TARGET)

clean:
	-$(RM) "$(TARGET)" "$(BENCH_TARGET)" "$(TEST_TARGET)"
# 	-$(RM) analisys/json_data/*.json
# 	-$(RM) analisys/json_data/examples/*.json

.PHONY: all compile test run clean run-benchmarks
