#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h> // Вложенный запуск потоков тоже обращается к счётчику.

static atomic_int launches; // Счётчик запусков доступен из нескольких потоков.
static int fail_every;

static int test_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                              void *(*entry)(void *), void *arg) {
  int launch = atomic_fetch_add(&launches, 1) + 1; // Получаем уникальный номер запуска.
  if (fail_every && launch % fail_every == 0)
    return EAGAIN;
  return pthread_create(thread, attr, entry, arg);
}

#define pthread_create test_thread_create
#include "../analisys/combined_benchmarks.c"
#include "../analisys/distance_metrics.c" // Проверяем также отказы создания внутреннего потока.
#undef pthread_create
#include "distance_metrics_reference.h" // Сравниваем с прежними независимыми расчётами.

static void check_report(MazeTable table) {
  TopologyReport actual;
  MazeGenerationMetrics metrics = labirit_metrics_snapshot();
  bool connected = bfs_connected(table, 0);
  int diameter = 0;
  int a = bfs_farthest(table, 0, NULL);
  bfs_farthest(table, a, &diameter);
  int longest = find_longest_path(table);
  double aspl = connected ? compute_aspl_slow(table) : -1.0;
  double entropy = connected ? compute_distance_entropy_slow(table) : -1.0;
  double symmetry = compute_symmetry_score(table);
  double fractal = compute_fractal_dimension(table);
  int branches = 0, dead_ends = 0, corridors = 0, degree_sum = 0;
  int total = table.rows * table.columns;
  for (int y = 0; y < table.rows; y++)
    for (int x = 0; x < table.columns; x++) {
      int degree = cell_degree(table, x, y);
      branches += degree >= 3;
      dead_ends += degree == 1;
      corridors += degree == 2;
      degree_sum += degree;
    }

  /* Normal launch, partial failure, and complete synchronous fallback. */
  const int failures[] = {0, 2, 1};
  for (size_t i = 0; i < sizeof(failures) / sizeof(failures[0]); i++) {
    launches = 0;
    fail_every = failures[i];
    DistanceMetrics distances = compute_distance_metrics(table); // Проверяем прямой вызов и fallback.
    assert(distances.aspl == aspl && distances.entropy == entropy);
    assert(analyze_topology(table, &actual, table.columns, table.rows,
                            table.seed, metrics));
    assert(actual.columns == table.columns && actual.rows == table.rows);
    assert(actual.seed == table.seed);
    assert(actual.all_pairs_connected == connected);
    assert(actual.steps_count == metrics.steps);
    assert(actual.memory_peak_bytes == metrics.peak_memory_bytes);
    assert(actual.diameter == diameter && actual.longest_path == longest);
    assert(actual.aspl == aspl && actual.entropy == entropy);
    assert(actual.symmetry_score == symmetry);
    assert(actual.fractal_dimension == fractal);
    assert(actual.wall_density == compute_wall_density(table));
    assert(actual.branch_ratio == (float)branches / total);
    assert(actual.dead_end_count == dead_ends);
    assert(actual.dead_end_ratio == (float)dead_ends / total);
    assert(actual.corridor_ratio == (float)corridors / total);
    assert(actual.avg_degree == (float)degree_sum / total);
    assert(actual.cycle_count ==
           count_open_edges(table) - total + count_components(table));
  }
}

int main(void) {
  Generator generators[] = {prim_alg,     growing_tree_alg,
                            watson_alg,   dfs_topology_adapter,
                            binary_algos, recursive_division_algorithm};
  for (size_t i = 0; i < sizeof(generators) / sizeof(generators[0]); i++)
    for (int seed = 1; seed <= 3; seed++) {
      labirit_metrics_reset();
      MazeTable table = init_table(8, 6, seed);
      assert(table.data);
      generators[i](table);
      check_report(table);
      clear_table(&table);
    }

  /* Проверяем SIMD-хвост, полный блок и узкие сетки по старому эталону. */
  const int dimensions[][2] = {{1, 1}, {7, 1}, {8, 1}, {9, 1},
                               {1, 7}, {1, 8}, {1, 9}, {3, 5}};
  for (size_t i = 0; i < sizeof(dimensions) / sizeof(dimensions[0]); i++) {
    MazeTable narrow = init_table(dimensions[i][0], dimensions[i][1], 101);
    assert(narrow.data);
    binary_algos(narrow);
    check_report(narrow);
    clear_table(&narrow);
  }

  MazeTable table = init_table(2, 2, 101);
  assert(table.data);
  check_report(table); /* Disconnected graph. */
  table.data[0][0].wall.right = table.data[0][1].wall.left = 0;
  table.data[0][0].wall.bottom = table.data[1][0].wall.top = 0;
  table.data[0][1].wall.bottom = table.data[1][1].wall.top = 0;
  table.data[1][0].wall.right = table.data[1][1].wall.left = 0;
  check_report(table); /* Cycle: diameter 2, longest simple path 3. */
  clear_table(&table);

  table = init_table(1, 1, 101);
  assert(table.data);
  check_report(table);
  clear_table(&table);
  puts("OK: threaded topology matches sequential metrics (including launch "
       "failures)");
  return 0;
}
