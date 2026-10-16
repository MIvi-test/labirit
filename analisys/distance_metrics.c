#include "distance_metrics.h"
#include <immintrin.h> // Инструкции AVX2 для заполнения расстояний.
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

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

	int *queue = malloc((size_t)total * sizeof(*queue));
	int *dist = malloc((size_t)total * sizeof(*dist));
	long long *hist = calloc((size_t)total, sizeof(*hist));
	if (!queue || !dist || !hist)
		goto cleanup_error;

	double sum = 0.0;
	__m256i value =
		_mm256_set1_epi32(-1); // Восемь 32-битных значений «не посещено».
	for (int start = 0; start < total; start++)
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
				goto cleanup_error;
			sum += (double)dist[v]; // Добавляем расстояние в сумму для ASPL.
			hist[dist[v]]++;		// Одновременно учитываем его в гистограмме
									// энтропии.
		}
	}
	long long pairs =
		(long long)total *
		(total - 1); // Расширяем тип до умножения; считаем обе ориентации пар.
	DistanceMetrics result = {sum / (double)pairs,
							  0.0}; // Нормируем сумму расстояний.
	const double inv_ln2 =
		1.0 /
		log(2.0); // Сохраняем способ вычисления логарифма исходной версии.
	for (int d = 1; d < total; d++)
	{
		if (hist[d] == 0) // Нулевая вероятность не вносит вклада в энтропию.
			continue;
		double p =
			(double)hist[d] / (double)pairs; // Доля пар с данным расстоянием.
		result.entropy -= p * (log(p) * inv_ln2);
	}
	free(queue);
	free(dist);
	free(hist);
	return result;

cleanup_error:
	free(queue);
	free(dist);
	free(hist);
	return error;
}
