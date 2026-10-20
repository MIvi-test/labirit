#include "distance_metrics.h"
#include <immintrin.h> // Инструкции AVX2 для заполнения расстояний
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DISTANCE_WORKERS 2 // Общее число обработчиков, включая текущий поток.

#if DISTANCE_WORKERS < 1
#error "DISTANCE_WORKERS must be at least 1"
#endif

typedef struct // Независимые рабочие данные одного диапазона стартов
{
	MazeTable table;
	int begin;
	int end;
	int *queue;
	int *dist;
	long long *hist;
	double sum;
	bool ok;
} DistanceWorker;

static void *run_distance_worker(void *arg)
{
	DistanceWorker *worker = arg;
	MazeTable table = worker->table;
	int total = table.rows * table.columns;
	int *queue = worker->queue;
	int *dist = worker->dist;
	long long *hist = worker->hist;
	__m256i value = _mm256_set1_epi32(-1);
	for (int start = worker->begin; start < worker->end;
		 start++) // Делим только старты; каждый BFS обходит весь лабиринт
	{
		int i = 0;
		for (; total - i >= 8; i += 8)
			_mm256_storeu_si256((__m256i *)(dist + i), value);
		for (; i < total; i++)
			dist[i] = -1;

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
			if (y > 0 && table.data[y][x].wall.top == 0)
				neighbors[count++] = cur - table.columns;
			if (x < table.columns - 1 && table.data[y][x].wall.right == 0)
				neighbors[count++] = cur + 1;
			if (y < table.rows - 1 && table.data[y][x].wall.bottom == 0)
				neighbors[count++] = cur + table.columns;
			if (x > 0 && table.data[y][x].wall.left == 0)
				neighbors[count++] = cur - 1;
			for (int j = 0; j < count; j++)
			{
				int next = neighbors[j];
				if (dist[next] == -1)
				{
					dist[next] = dist[cur] + 1;
					queue[tail++] = next;
				}
			}
		}
		for (int v = 0; v < total; v++)
		{
			if (v == start)
				continue;
			if (dist[v] < 0)
				return NULL;
			worker->sum += (double)dist[v]; // расстояние в сумму для ASPL
			hist[dist[v]]++;				// энтропия
		}
	}
	worker->ok = true;
	return NULL; // Результат сохраняется в контексте, а не возвращаемом
				 // указателе
}

DistanceMetrics compute_distance_metrics(MazeTable table)
{
	const DistanceMetrics error = {-1.0, -1.0};
	if (table.rows < 0 || table.columns < 0)
		return error;
	if (table.rows != 0 && table.columns > INT_MAX / table.rows)
		return error;
	int total = table.rows * table.columns;
	if (total <= 1)
		return (DistanceMetrics){0.0, 0.0};
	if (!table.data || (size_t)total > SIZE_MAX / sizeof(int) ||
		(size_t)total > SIZE_MAX / sizeof(long long))
		return error;

	// Не создаём обработчики без стартовых клеток на маленьких сетках.
	int workers_count = total < DISTANCE_WORKERS ? total : DISTANCE_WORKERS;
	DistanceWorker workers[DISTANCE_WORKERS] = {0}; // Нулевые указатели безопасны при частичном выделении.
	pthread_t threads[DISTANCE_WORKERS]; // Дескрипторы дополнительных потоков.
	bool started[DISTANCE_WORKERS] = {false}; // Запоминаем, какие потоки нужно дождаться.
	for (int i = 0; i < workers_count; i++)
	{
		workers[i].table = table; // Общий лабиринт доступен только для чтения.
		workers[i].begin = (int)((long long)total * i / workers_count); // Начало включительно.
		workers[i].end = (int)((long long)total * (i + 1) / workers_count); // Конец не включается.
		workers[i].queue = malloc((size_t)total * sizeof(int));
		workers[i].dist = malloc((size_t)total * sizeof(int));
		workers[i].hist = calloc((size_t)total, sizeof(long long));
		if (!workers[i].queue || !workers[i].dist || !workers[i].hist)
			goto cleanup_error; // Все выделения проверяем до запуска потоков.
	}
	for (int i = 0; i < workers_count - 1; i++) // Последний диапазон оставляем текущему потоку.
		started[i] = pthread_create(&threads[i], NULL, run_distance_worker, &workers[i]) == 0;
	run_distance_worker(&workers[workers_count - 1]); // Текущий поток тоже выполняет полезную работу.
	for (int i = 0; i < workers_count - 1; i++)
	{
		if (!started[i]) // Отказ запуска компенсируем последовательным выполнением.
			run_distance_worker(&workers[i]);
	}
	for (int i = 0; i < workers_count - 1; i++)
	{
		if (started[i] && pthread_join(threads[i], NULL) != 0)
		{
			fputs("labirit: failed to join distance worker\n", stderr);
			abort(); // Нельзя освобождать контекст, пока поток может его использовать.
		}
	}
	for (int i = 0; i < workers_count; i++) // Читаем результаты только после завершения всех потоков.
	{
		if (!workers[i].ok)
			goto cleanup_error;
	}

	double sum = workers[0].sum; // Начинаем объединение с первого обработчика.
	long long *hist = workers[0].hist; // Используем его буфер без дополнительного выделения.
	for (int i = 1; i < workers_count; i++)
	{
		sum += workers[i].sum; // Суммируем расстояния всех диапазонов.
		for (int d = 1; d < total; d++)
			hist[d] += workers[i].hist[d]; // Складываем частоты перед расчётом энтропии.
	}
	long long pairs = (long long)total * (total - 1);
	DistanceMetrics result = {sum / (double)pairs,
							  0.0}; // Нормируем сумму расстояний
	const double inv_ln2 = 1.0 / log(2.0);
	for (int d = 1; d < total; d++)
	{
		if (hist[d] == 0)
			continue;
		double p =
			(double)hist[d] / (double)pairs; // Доля пар с данным расстоянием
		result.entropy -= p * (log(p) * inv_ln2);
	}
	for (int i = 0; i < workers_count; i++)
	{
		free(workers[i].queue);
		free(workers[i].dist);
		free(workers[i].hist);
	}
	return result;

cleanup_error:
	for (int i = 0; i < workers_count; i++)
	{
		free(workers[i].queue);
		free(workers[i].dist);
		free(workers[i].hist);
	}
	return error;
}
