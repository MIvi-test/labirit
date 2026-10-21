#include "../backend/all_algorithms.h"
#include "../deter.h"
#include "benchmark_formats.h"
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "distance_metrics.h" // Общий расчёт ASPL и энтропии для двух бенчмарков.

typedef void (*Generator)(MazeTable);

typedef struct
{
	const char *name;
	const char *path;
	Generator generator;
} BenchmarkJob;

typedef enum
{
	METRIC_DIAMETER,
	METRIC_DISTANCES, // Одно задание вычисляет обе метрики расстояний.
	METRIC_SYMMETRY,
	METRIC_FRACTAL_DIMENSION,
	METRIC_LONGEST_PATH
} MetricKind;

typedef struct
{
	MazeTable table;
	MetricKind kind;
	double result;
	DistanceMetrics distances; // Здесь поток сохраняет сразу ASPL и энтропию.
	pthread_t thread;
	bool started;
} MetricTask;

static void dfs_topology_adapter(MazeTable t) { (void)dfs_algorithm(t); }

static long long now_microseconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000000LL + (long long)(ts.tv_nsec / 1000);
}

static int make_directory(const char *path) { return mkdir(path, 0755); }

bool ensure_results_dir(void)
{
	struct stat st;
	if (stat("analisys", &st) != 0 && make_directory("analisys") != 0)
	{
		return false;
	}
	if (stat(RESULTS_DIR, &st) == 0)
	{
		return true;
	}
	return make_directory(RESULTS_DIR) == 0;
}

static bool ensure_examples_dir(void)
{
	struct stat st;
	const char *path = RESULTS_DIR "/examples";
	if (stat(path, &st) == 0)
	{
		return true;
	}
	return make_directory(path) == 0;
}

/* --- Topology analysis functions: copied from topology_benchmarks.c --- */

static int cell_degree(MazeTable table, int x, int y)
{
	int degree = 0;
	if (table.data[y][x].wall.top == 0)
		degree++;
	if (table.data[y][x].wall.right == 0)
		degree++;
	if (table.data[y][x].wall.bottom == 0)
		degree++;
	if (table.data[y][x].wall.left == 0)
		degree++;
	return degree;
}

static int bfs_farthest(MazeTable table, int start, int *dist_out)
{
	int total = table.rows * table.columns;
	int *queue = (int *)malloc(sizeof(int) * total);
	int *dist = (int *)malloc(sizeof(int) * total);
	if (!queue || !dist)
	{
		free(queue);
		free(dist);
		return start;
	}

	for (int i = 0; i < total; i++)
	{
		dist[i] = -1;
	}

	int head = 0;
	int tail = 0;
	queue[tail++] = start;
	dist[start] = 0;
	int farthest = start;

	while (head < tail)
	{
		int cur = queue[head++];
		int x = cur % table.columns;
		int y = cur / table.columns;
		if (dist[cur] > dist[farthest])
		{
			farthest = cur;
		}

		int neighbors[4];
		int count = 0;
		if (table.data[y][x].wall.top == 0 && y > 0)
			neighbors[count++] = (y - 1) * table.columns + x;
		if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
			neighbors[count++] = y * table.columns + x + 1;
		if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
			neighbors[count++] = (y + 1) * table.columns + x;
		if (table.data[y][x].wall.left == 0 && x > 0)
			neighbors[count++] = y * table.columns + x - 1;

		for (int i = 0; i < count; i++)
		{
			int next = neighbors[i];
			if (dist[next] == -1)
			{
				dist[next] = dist[cur] + 1;
				queue[tail++] = next;
			}
		}
	}

	if (dist_out)
	{
		*dist_out = dist[farthest];
	}

	free(dist);
	free(queue);
	return farthest;
}

static bool bfs_connected(MazeTable table, int start)
{
	int total = table.rows * table.columns;
	int *queue = (int *)malloc(sizeof(int) * total);
	int *dist = (int *)malloc(sizeof(int) * total);
	if (!queue || !dist)
	{
		free(queue);
		free(dist);
		return false;
	}

	for (int i = 0; i < total; i++)
	{
		dist[i] = -1;
	}

	int head = 0;
	int tail = 0;
	queue[tail++] = start;
	dist[start] = 0;

	while (head < tail)
	{
		int cur = queue[head++];
		int x = cur % table.columns;
		int y = cur / table.columns;

		int neighbors[4];
		int count = 0;
		if (table.data[y][x].wall.top == 0 && y > 0)
			neighbors[count++] = (y - 1) * table.columns + x;
		if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
			neighbors[count++] = y * table.columns + x + 1;
		if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
			neighbors[count++] = (y + 1) * table.columns + x;
		if (table.data[y][x].wall.left == 0 && x > 0)
			neighbors[count++] = y * table.columns + x - 1;

		for (int i = 0; i < count; i++)
		{
			int next = neighbors[i];
			if (dist[next] == -1)
			{
				dist[next] = dist[cur] + 1;
				queue[tail++] = next;
			}
		}
	}

	bool ok = true;
	for (int i = 0; i < total; i++)
	{
		if (dist[i] == -1)
		{
			ok = false;
			break;
		}
	}

	free(dist);
	free(queue);
	return ok;
}

static int count_open_edges(MazeTable table)
{
	int edges = 0;
	for (int y = 0; y < table.rows; y++)
	{
		for (int x = 0; x < table.columns; x++)
		{
			if (x + 1 < table.columns && table.data[y][x].wall.right == 0)
			{
				edges++;
			}
			if (y + 1 < table.rows && table.data[y][x].wall.bottom == 0)
			{
				edges++;
			}
		}
	}
	return edges;
}

static float compute_wall_density(MazeTable table)
{
	int closed = 0;
	int total_sides = table.rows * table.columns * 4;
	for (int y = 0; y < table.rows; y++)
	{
		for (int x = 0; x < table.columns; x++)
		{
			closed += table.data[y][x].wall.top ? 1 : 0;
			closed += table.data[y][x].wall.right ? 1 : 0;
			closed += table.data[y][x].wall.bottom ? 1 : 0;
			closed += table.data[y][x].wall.left ? 1 : 0;
		}
	}
	return total_sides > 0 ? (float)closed / (float)total_sides : 0.0f;
}

static int count_components(MazeTable table)
{
	int total = table.rows * table.columns;
	bool *visited = (bool *)calloc((size_t)total, sizeof(bool));
	int *queue = (int *)malloc(sizeof(int) * (size_t)total);
	if (!visited || !queue)
	{
		free(visited);
		free(queue);
		return 0;
	}

	int components = 0;
	for (int start = 0; start < total; start++)
	{
		if (visited[start])
		{
			continue;
		}
		components++;
		int head = 0;
		int tail = 0;
		queue[tail++] = start;
		visited[start] = true;

		while (head < tail)
		{
			int cur = queue[head++];
			int x = cur % table.columns;
			int y = cur / table.columns;

			int neighbors[4];
			int count = 0;
			if (table.data[y][x].wall.top == 0 && y > 0)
				neighbors[count++] = (y - 1) * table.columns + x;
			if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
				neighbors[count++] = y * table.columns + x + 1;
			if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
				neighbors[count++] = (y + 1) * table.columns + x;
			if (table.data[y][x].wall.left == 0 && x > 0)
				neighbors[count++] = y * table.columns + x - 1;

			for (int i = 0; i < count; i++)
			{
				int next = neighbors[i];
				if (!visited[next])
				{
					visited[next] = true;
					queue[tail++] = next;
				}
			}
		}
	}

	free(queue);
	free(visited);
	return components;
}

static unsigned char wall_bits(Cell cell)
{
	return (unsigned char)((cell.wall.top ? 1 : 0) |
						   ((cell.wall.right ? 1 : 0) << 1) |
						   ((cell.wall.bottom ? 1 : 0) << 2) |
						   ((cell.wall.left ? 1 : 0) << 3));
}

static unsigned char transform_wall_bits(unsigned char bits, int transform)
{
	bool top = (bits & 1) != 0;
	bool right = (bits & 2) != 0;
	bool bottom = (bits & 4) != 0;
	bool left = (bits & 8) != 0;

	if (transform == 0)
	{
		return (unsigned char)((top ? 1 : 0) | ((left ? 1 : 0) << 1) |
							   ((bottom ? 1 : 0) << 2) |
							   ((right ? 1 : 0) << 3));
	}
	if (transform == 1)
	{
		return (unsigned char)((bottom ? 1 : 0) | ((right ? 1 : 0) << 1) |
							   ((top ? 1 : 0) << 2) | ((left ? 1 : 0) << 3));
	}
	return (unsigned char)((bottom ? 1 : 0) | ((left ? 1 : 0) << 1) |
						   ((top ? 1 : 0) << 2) | ((right ? 1 : 0) << 3));
}

static int bit_similarity_count(unsigned char a, unsigned char b)
{
	int matches = 0;
	for (int bit = 0; bit < 4; bit++)
	{
		if (((a >> bit) & 1) == ((b >> bit) & 1))
		{
			matches++;
		}
	}
	return matches;
}

static double compute_symmetry_score(MazeTable table)
{
	long long matches = 0;
	long long comparisons =
		(long long)table.rows * (long long)table.columns * 12LL;
	for (int y = 0; y < table.rows; y++)
	{
		for (int x = 0; x < table.columns; x++)
		{
			unsigned char original = wall_bits(table.data[y][x]);
			matches += bit_similarity_count(
				original,
				transform_wall_bits(
					wall_bits(table.data[y][table.columns - 1 - x]), 0));
			matches += bit_similarity_count(
				original, transform_wall_bits(
							  wall_bits(table.data[table.rows - 1 - y][x]), 1));
			matches += bit_similarity_count(
				original,
				transform_wall_bits(
					wall_bits(
						table.data[table.rows - 1 - y][table.columns - 1 - x]),
					2));
		}
	}
	return comparisons > 0 ? (double)matches / (double)comparisons : 0.0;
}

static bool wall_point_occupied(MazeTable table, int gx, int gy)
{
	int width = table.columns * 2 + 1;
	int height = table.rows * 2 + 1;
	if (gx <= 0 || gy <= 0 || gx >= width - 1 || gy >= height - 1)
	{
		return true;
	}
	if ((gy % 2) == 0 && (gx % 2) == 1)
	{
		int x = gx / 2;
		int y = gy / 2;
		return table.data[y][x].wall.top != 0;
	}
	if ((gx % 2) == 0 && (gy % 2) == 1)
	{
		int x = gx / 2;
		int y = gy / 2;
		return table.data[y][x].wall.left != 0;
	}
	if ((gx % 2) == 0 && (gy % 2) == 0)
	{
		return wall_point_occupied(table, gx - 1, gy) ||
			   wall_point_occupied(table, gx + 1, gy) ||
			   wall_point_occupied(table, gx, gy - 1) ||
			   wall_point_occupied(table, gx, gy + 1);
	}
	return false;
}

static int count_wall_boxes(MazeTable table, int box_size)
{
	int width = table.columns * 2 + 1;
	int height = table.rows * 2 + 1;
	int boxes = 0;
	for (int y0 = 0; y0 < height; y0 += box_size)
	{
		for (int x0 = 0; x0 < width; x0 += box_size)
		{
			bool occupied = false;
			for (int y = y0; y < y0 + box_size && y < height && !occupied; y++)
			{
				for (int x = x0; x < x0 + box_size && x < width; x++)
				{
					if (wall_point_occupied(table, x, y))
					{
						occupied = true;
						break;
					}
				}
			}
			if (occupied)
			{
				boxes++;
			}
		}
	}
	return boxes;
}

static double compute_fractal_dimension(MazeTable table)
{
	int max_dim =
		table.columns > table.rows ? table.columns * 2 + 1 : table.rows * 2 + 1;
	double sum_x = 0.0;
	double sum_y = 0.0;
	double sum_xx = 0.0;
	double sum_xy = 0.0;
	int n = 0;

	for (int box_size = 1; box_size <= max_dim; box_size *= 2)
	{
		int boxes = count_wall_boxes(table, box_size);
		if (boxes <= 0)
		{
			continue;
		}
		double x = log((double)max_dim / (double)box_size);
		double y = log((double)boxes);
		sum_x += x;
		sum_y += y;
		sum_xx += x * x;
		sum_xy += x * y;
		n++;
	}

	double denom = (double)n * sum_xx - sum_x * sum_x;
	if (n < 2 || fabs(denom) < 1e-12)
	{
		return 0.0;
	}
	return ((double)n * sum_xy - sum_x * sum_y) / denom;
}

static void dfs_longest(MazeTable table, int current, bool *visited,
						int current_length, int *max_path)
{
	visited[current] = true;

	if (current_length > *max_path)
	{
		*max_path = current_length;
	}

	int x = current % table.columns;
	int y = current / table.columns;

	int neighbors[4];
	int count = 0;
	if (table.data[y][x].wall.top == 0 && y > 0)
		neighbors[count++] = (y - 1) * table.columns + x;
	if (table.data[y][x].wall.right == 0 && x < table.columns - 1)
		neighbors[count++] = y * table.columns + x + 1;
	if (table.data[y][x].wall.bottom == 0 && y < table.rows - 1)
		neighbors[count++] = (y + 1) * table.columns + x;
	if (table.data[y][x].wall.left == 0 && x > 0)
		neighbors[count++] = y * table.columns + x - 1;

	for (int i = 0; i < count; i++)
	{
		int next = neighbors[i];
		if (!visited[next])
		{
			dfs_longest(table, next, visited, current_length + 1, max_path);
		}
	}

	visited[current] = false; // backtrack
}

static int find_longest_path(MazeTable table)
{
	// NP-трудная задача для общих графов!
	// Для небольших лабиринтов можно использовать DFS с backtracking
	int total = table.rows * table.columns;
	bool *visited = calloc(total, sizeof(bool));
	if (!visited)
	{
		return -1;
	}
	int max_path = 0;

	for (int start = 0; start < total; start++)
	{
		dfs_longest(table, start, visited, 0, &max_path);
	}

	free(visited);
	return max_path;
}

/* Each task owns its result and temporary buffers; the maze is read-only. */

static void *run_metric(void *arg)
{
	MetricTask *task = arg;
	switch (task->kind)
	{
	case METRIC_DIAMETER:
	{
		int diameter = 0;
		int a = bfs_farthest(task->table, 0, NULL);
		(void)bfs_farthest(task->table, a, &diameter);
		task->result = diameter;
		break;
	}
	case METRIC_DISTANCES: // Единственный набор BFS обслуживает обе метрики.
		task->distances =
			compute_distance_metrics(task->table); // Сохраняем оба результата.
		break;
	case METRIC_SYMMETRY:
		task->result = compute_symmetry_score(task->table);
		break;
	case METRIC_FRACTAL_DIMENSION:
		task->result = compute_fractal_dimension(task->table);
		break;
	case METRIC_LONGEST_PATH:
		task->result = find_longest_path(task->table);
		break;
	}
	return NULL;
}

static bool analyze_topology(MazeTable table, TopologyReport *out, int columns,
							 int rows, int seed, MazeGenerationMetrics metrics)
{
	if (!out || !table.data)
	{
		return false;
	}

	int total = rows * columns;
	int branch = 0;
	int dead_end = 0;
	int corridor = 0;
	int degree_sum = 0;
	int open_edges = count_open_edges(table);
	int components = count_components(table);
	if (components <= 0)
		return false;
	bool connected = bfs_connected(table, 0);
	bool is_tree = components == 1 && open_edges == total - 1;
	MetricTask tasks[] = {
		{.table = table, .kind = METRIC_DIAMETER},
		{.table = table,
		 .kind = METRIC_DISTANCES,
		 .distances =
			 {-1.0, -1.0}}, // Несвязный граф сохраняет прежние значения ошибки.
		{.table = table, .kind = METRIC_SYMMETRY},
		{.table = table, .kind = METRIC_FRACTAL_DIMENSION},
		{.table = table, .kind = METRIC_LONGEST_PATH},
	};
	size_t task_count = sizeof(tasks) / sizeof(tasks[0]);
	for (size_t i = 0; i < task_count; i++)
	{
		// Пропускаем метрики расстояний для несвязного графа.
		if ((!connected && tasks[i].kind == METRIC_DISTANCES) ||
			(is_tree && tasks[i].kind == METRIC_LONGEST_PATH))
			continue;
		tasks[i].started =
			pthread_create(&tasks[i].thread, NULL, run_metric, &tasks[i]) == 0;
		if (!tasks[i].started)
			run_metric(&tasks[i]);
	}

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < columns; x++)
		{
			int degree = cell_degree(table, x, y);
			degree_sum += degree;
			if (degree >= 3)
				branch++;
			if (degree == 1)
				dead_end++;
			if (degree == 2)
				corridor++;
		}
	}

	/* Join every worker before reading results or releasing stack contexts. */
	for (size_t i = 0; i < task_count; i++)
	{
		if (tasks[i].started && pthread_join(tasks[i].thread, NULL) != 0)
		{
			/* Returning would expose live workers to expired stack storage. */
			fputs("labirit: failed to join metric worker\n", stderr);
			abort();
		}
	}

	out->columns = columns;
	out->rows = rows;
	out->seed = seed;
	out->all_pairs_connected = connected;
	out->steps_count = metrics.steps;
	out->memory_peak_bytes = metrics.peak_memory_bytes;
	out->wall_density = compute_wall_density(table);
	out->branch_ratio = (float)branch / (float)total;
	out->dead_end_count = dead_end;
	out->dead_end_ratio = (float)dead_end / (float)total;
	out->corridor_ratio = (float)corridor / (float)total;
	out->avg_degree = (float)degree_sum / (float)total;
	out->cycle_count = open_edges - total + components;
	out->diameter = (int)tasks[METRIC_DIAMETER].result;
	out->longest_path =
		is_tree ? out->diameter : (int)tasks[METRIC_LONGEST_PATH].result;
	out->symmetry_score = tasks[METRIC_SYMMETRY].result;
	out->fractal_dimension = tasks[METRIC_FRACTAL_DIMENSION].result;
	out->aspl = tasks[METRIC_DISTANCES]
					.distances.aspl; // Берём среднее расстояние общего расчёта.
	out->entropy = tasks[METRIC_DISTANCES]
					   .distances.entropy; // Берём энтропию того же расчёта.
	return true;
}

/* --- Combined benchmark --- */

static bool run_case_combined(FILE *out, Generator generator, int columns,
							  int rows, int seed, bool is_first)
{
	labirit_metrics_reset();
	MazeTable table = generator == recursive_division_algorithm
                          ? init_table_open(columns, rows, seed)
                          : init_table(columns, rows, seed);
	if (!table.data)
	{
		return false;
	}

	labirit_metrics_reset_steps();
	long long started = now_microseconds();
	if (generator == recursive_division_algorithm)
	{
	    recursive_division_prepared(table);
	}
	else
	{
	    generator(table);
	}
	long long finished = now_microseconds();
	MazeGenerationMetrics metrics = labirit_metrics_snapshot();

	TopologyReport report;
	if (!analyze_topology(table, &report, columns, rows, seed, metrics))
	{
		clear_table(&table);
		return false;
	}

	long long microseconds = finished - started;
	if (fprintf(
			out,
			"%s  {\"columns\":%d,\"rows\":%d,\"seed\":%d,\"microseconds\":%lld,"
			"\"all_pairs_connected\":%s,\"branch_ratio\":%.6f,"
			"\"steps_count\":%lld,\"memory_peak_bytes\":%zu,\"wall_density\":%."
			"6f,"
			"\"dead_end_count\":%d,\"dead_end_ratio\":%.6f,\"corridor_ratio\":%"
			".6f,"
			"\"avg_degree\":%.6f,\"cycle_count\":%d,\"diameter\":%d,"
			"\"longest_path\":%d,\"symmetry_score\":%.6f,\"fractal_dimension\":"
			"%.6f,"
			"\"aspl\":%.6f,\"entropy\":%.6f,\"metrics_enabled\":%s}",
			is_first ? "" : ",\n", report.columns, report.rows, report.seed,
			microseconds, report.all_pairs_connected ? "true" : "false",
			report.branch_ratio, report.steps_count, report.memory_peak_bytes,
			report.wall_density, report.dead_end_count, report.dead_end_ratio,
			report.corridor_ratio, report.avg_degree, report.cycle_count,
			report.diameter, report.longest_path, report.symmetry_score,
			report.fractal_dimension, report.aspl, report.entropy,
			LABIRIT_METRICS_ENABLED ? "true" : "false") < 0)
	{
		clear_table(&table);
		return false;
	}

	clear_table(&table);
	return true;
}

static bool benchmark_algorithm_combined(const char *name, const char *path,
										 Generator generator,
										 const BenchmarkGridSize *sizes,
										 size_t sizes_count, const int *seeds,
										 size_t seeds_count)
{
	FILE *out = fopen(path, "w");
	if (!out)
	{
		return false;
	}

	if (fprintf(out, "[\n") < 0)
	{
		fclose(out);
		return false;
	}

	const size_t total = sizes_count * seeds_count;
	size_t completed = 0;
	const long long started = now_microseconds();
	bool first_entry = true;
	for (size_t i = 0; i < sizes_count; i++)
	{
		for (size_t s = 0; s < seeds_count; s++)
		{
			if (!run_case_combined(out, generator, sizes[i].columns,
								   sizes[i].rows, seeds[s], first_entry))
			{
				fclose(out);
				return false;
			}
			first_entry = false;
			completed++;
			if (completed % 10 == 0 || completed == total)
			{
				long long elapsed = (now_microseconds() - started) / 1000000;
				/* One short write keeps concurrent process logs readable. */
				char line[512];
				int length =
					snprintf(line, sizeof(line),
							 "[%s] %zu/%zu (%.1f%%) | elapsed %02lld:%02lld | "
							 "last %dx%d seed=%d\n",
							 name, completed, total, 100.0 * completed / total,
							 elapsed / 60, elapsed % 60, sizes[i].columns,
							 sizes[i].rows, seeds[s]);
				if (length > 0 && (size_t)length < sizeof(line))
				{
					ssize_t written;
					do
					{
						written = write(STDOUT_FILENO, line, (size_t)length);
					} while (written == -1 && errno == EINTR);
				}
			}
		}
	}

	if (fprintf(out, "\n]\n") < 0)
	{
		fclose(out);
		return false;
	}

	return fclose(out) == 0;
}

static char maze_ascii_at(MazeTable table, int gx, int gy)
{
	int width = table.columns * 2 + 1;
	int height = table.rows * 2 + 1;
	if (gx == 0 || gy == 0 || gx == width - 1 || gy == height - 1)
	{
		return '#';
	}
	if ((gx % 2) == 1 && (gy % 2) == 1)
	{
		return ' ';
	}
	if ((gx % 2) == 0 && (gy % 2) == 0)
	{
		return '#';
	}
	if ((gx % 2) == 0)
	{
		int cell_x = gx / 2 - 1;
		int cell_y = gy / 2;
		return table.data[cell_y][cell_x].wall.right ? '#' : ' ';
	}

	int cell_x = gx / 2;
	int cell_y = gy / 2 - 1;
	return table.data[cell_y][cell_x].wall.bottom ? '#' : ' ';
}

static bool write_ascii_lines(FILE *out, MazeTable table)
{
	int width = table.columns * 2 + 1;
	int height = table.rows * 2 + 1;

	if (fprintf(out, "\"ascii\":[") < 0)
	{
		return false;
	}
	for (int gy = 0; gy < height; gy++)
	{
		if (fprintf(out, "%s\"", gy == 0 ? "" : ",") < 0)
		{
			return false;
		}
		for (int gx = 0; gx < width; gx++)
		{
			if (fputc(maze_ascii_at(table, gx, gy), out) == EOF)
			{
				return false;
			}
		}
		if (fputc('"', out) == EOF)
		{
			return false;
		}
	}
	return fprintf(out, "]") >= 0;
}

static bool write_example_maze(FILE *out, const char *algorithm,
							   Generator generator, bool is_first)
{
	int columns = 24;
	int rows = 24;
	int seed = 101;

	labirit_metrics_reset();
	MazeTable table = generator == recursive_division_algorithm
                          ? init_table_open(columns, rows, seed)
                          : init_table(columns, rows, seed);
	if (!table.data)
	{
		return false;
	}

	if (generator == recursive_division_algorithm)
	{
	    recursive_division_prepared(table);
	}
	else
	{
	    generator(table);
	}

	bool ok =
		fprintf(
			out,
			"%s  {\"algorithm\":\"%s\",\"columns\":%d,\"rows\":%d,\"seed\":%d,",
			is_first ? "" : ",\n", algorithm, columns, rows, seed) >= 0 &&
		write_ascii_lines(out, table) && fprintf(out, "}") >= 0;

	clear_table(&table);
	return ok;
}

static bool write_example_mazes(void)
{
	if (!ensure_examples_dir())
	{
		return false;
	}

	FILE *out = fopen(RESULTS_DIR "/examples/maze_examples.json", "w");
	if (!out)
	{
		return false;
	}

	bool ok =
		fprintf(out, "[\n") >= 0 &&
		write_example_maze(out, "prim", prim_alg, true) &&
		write_example_maze(out, "growing_tree", growing_tree_alg, false) &&
		write_example_maze(out, "watson", watson_alg, false) &&
		write_example_maze(out, "dfs", dfs_topology_adapter, false) &&
		write_example_maze(out, "binary_tree", binary_algos, false) &&
		write_example_maze(out, "recursive_division",
						   recursive_division_algorithm, false) &&
		fprintf(out, "\n]\n") >= 0;

	fclose(out);
	return ok;
}

/* The caller must be single-threaded when forking. Workers create their
 * metric threads only after fork, and each worker owns a separate JSON file. */
static bool run_benchmark_jobs(const BenchmarkJob *jobs, size_t jobs_count,
							   const BenchmarkGridSize *sizes,
							   size_t sizes_count, const int *seeds,
							   size_t seeds_count)
{
	pid_t *children = calloc(jobs_count, sizeof(*children));
	if (!children)
		return false;

	size_t started = 0;
	bool success = true;
	fflush(NULL);
	for (size_t i = 0; i < jobs_count; i++)
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			perror("labirit: fork");
			success = false;
			break;
		}
		if (pid == 0)
		{
			bool ok = benchmark_algorithm_combined(
				jobs[i].name, jobs[i].path, jobs[i].generator, sizes,
				sizes_count, seeds, seeds_count);
			_exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
		}
		children[started++] = pid;
		printf("Started %s (PID %ld)\n", jobs[i].name, (long)pid);
		fflush(stdout);
	}

	/* All children run concurrently, even though we collect statuses in order.
	 */
	size_t completed = 0;
	for (size_t i = 0; i < started; i++)
	{
		int status;
		pid_t result;
		do
		{
			result = waitpid(children[i], &status, 0);
		} while (result == -1 && errno == EINTR);

		if (result == -1)
		{
			perror("labirit: waitpid");
			success = false;
		}
		else if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
		{
			completed++;
			printf("Completed %s (%zu/%zu)\n", jobs[i].name, completed,
				   jobs_count);
			fflush(stdout);
		}
		else
		{
			if (WIFSIGNALED(status))
				fprintf(stderr, "labirit: %s terminated by signal %d\n",
						jobs[i].name, WTERMSIG(status));
			else
				fprintf(stderr, "labirit: %s failed (status %d)\n",
						jobs[i].name, WEXITSTATUS(status));
			success = false;
		}
	}
	free(children);
	return success;
}

bool run_combined_benchmarks(void)
{
	const int seeds[] = {101, 202, 303, 404, 505, 606, 707, 808, 909, 1010};
	const BenchmarkJob jobs[] = {
		{"prim", RESULTS_DIR "/prim.json", prim_alg},
		{"growing_tree", RESULTS_DIR "/growing_tree.json", growing_tree_alg},
		{"watson", RESULTS_DIR "/watson.json", watson_alg},
		{"dfs", RESULTS_DIR "/dfs.json", dfs_topology_adapter},
		{"binary_tree", RESULTS_DIR "/binary_tree.json", binary_algos},
		{"recursive_division", RESULTS_DIR "/recursive_division.json",
		 recursive_division_algorithm},
	};
	if (!ensure_results_dir())
		return false;

	puts("Running benchmarks in 6 processes...");
	if (!run_benchmark_jobs(
			jobs, sizeof(jobs) / sizeof(jobs[0]), TIME_BENCH_CASES,
			sizeof(TIME_BENCH_CASES) / sizeof(TIME_BENCH_CASES[0]), seeds,
			sizeof(seeds) / sizeof(seeds[0])))
		return false;
	if (!write_example_mazes())
		return false;
	puts("Benchmarks completed.");
	return true;
}
