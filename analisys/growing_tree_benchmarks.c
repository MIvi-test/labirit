/* Compile with METRICS=0 for timing, METRICS=1 for memory/operation counts. */
#include "../backend/all_algorithms.h"
#include "../tests/growing_tree_reference.h"

static long long now_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main(void)
{
	const int sizes[][2] = {{32, 32},  {64, 64},  {128, 128}, {256, 256},
							{16, 256}, {256, 16}, {1, 4096},  {4096, 1}};
	const unsigned seeds[] = {101, 202, 303};
	void (*generators[])(MazeTable) = {growing_tree_reference,
									   growing_tree_alg};
	puts("variant,columns,rows,seed,repeat,generations,nanoseconds,peak_bytes,"
		 "steps");
	for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++)
		for (size_t k = 0; k < sizeof(seeds) / sizeof(seeds[0]); k++)
			for (int repeat = -1; repeat < 5; repeat++)
				for (int turn = 0; turn < 2; turn++)
				{
					/* Alternate order, warm up each (size, seed, variant). */
					int variant = (turn + repeat + 1) % 2;
					long long elapsed = 0;
					unsigned generations = 0;
					MazeGenerationMetrics metrics;
					do
					{
						labirit_metrics_reset();
						MazeTable table =
							init_table(sizes[s][0], sizes[s][1], seeds[k]);
						if (!table.data)
							return 1;
						long long start = now_ns();
						generators[variant](table);
						elapsed += now_ns() - start;
						metrics = labirit_metrics_snapshot();
						clear_table(&table);
						generations++;
					} while (!LABIRIT_METRICS_ENABLED && elapsed < 20000000LL);
					if (repeat >= 0)
						printf("%s,%d,%d,%u,%d,%u,%lld,%zu,%lld\n",
							   variant ? "optimized" : "baseline", sizes[s][0],
							   sizes[s][1], seeds[k], repeat, generations,
							   elapsed, metrics.peak_memory_bytes,
							   metrics.steps);
				}
	return 0;
}
