#include "../deter.h"
#include <limits.h>
#include <stdint.h>

static void growing_tree_remove_wall(MazeTable table, int x1, int y1, int x2,
									 int y2)
{
	if (x2 - x1 == 1)
	{
		table.data[y1][x1].wall.right = 0;
		table.data[y2][x2].wall.left = 0;
	}
	else if (x2 - x1 == -1)
	{
		table.data[y1][x1].wall.left = 0;
		table.data[y2][x2].wall.right = 0;
	}
	else if (y2 - y1 == 1)
	{
		table.data[y1][x1].wall.bottom = 0;
		table.data[y2][x2].wall.top = 0;
	}
	else if (y2 - y1 == -1)
	{
		table.data[y1][x1].wall.top = 0;
		table.data[y2][x2].wall.bottom = 0;
	}
}

void growing_tree_alg(MazeTable table)
{
	LABIRIT_STEPS_DECLARE();
	if (!table.data || table.rows <= 0 || table.columns <= 0)
	{
		LABIRIT_STEPS_COMMIT();
		return;
	}

	/* Cell IDs and active positions must fit in int; allocation in size_t. */
	if (table.rows > INT_MAX / table.columns)
	{
		LABIRIT_STEPS_COMMIT();
		return;
	}
	int total = table.rows * table.columns;
	if ((size_t)total > SIZE_MAX / sizeof(int) / 4)
	{
		LABIRIT_STEPS_COMMIT();
		return;
	}
	size_t storage_bytes = (size_t)total * 4 * sizeof(int);
	bool *visited = (bool *)calloc((size_t)total, sizeof(bool));
	int *storage = (int *)malloc(storage_bytes);

	if (!visited || !storage)
	{
		free(visited);
		free(storage);
		LABIRIT_STEPS_COMMIT();
		return;
	}
	int *active = storage;
	int *position = active + total;
	int *older = position + total;
	int *newer = older + total;
	labirit_metrics_alloc((size_t)total * sizeof(bool));
	labirit_metrics_alloc(storage_bytes);

	int start_x = rand() % table.columns;
	int start_y = rand() % table.rows;
	int start = start_y * table.columns + start_x;

	visited[start] = true;
	active[0] = start;
	position[start] = 0;
	older[start] = -1;
	newer[start] = -1;
	int newest = start;
	int active_count = 1;

	while (active_count > 0)
	{
		LABIRIT_STEPS_ADD(1);
		int chosen_pos = 0;
		if (rand() % 100 < 60)
		{
			chosen_pos = position[newest];
		}
		else
		{
			chosen_pos = rand() % active_count;
		}

		int current = active[chosen_pos];
		int x = current % table.columns;
		int y = current / table.columns;

		int neighbors[4];
		int count = 0;

		if (y > 0)
		{
			LABIRIT_STEPS_ADD(1);
			int next = (y - 1) * table.columns + x;
			if (!visited[next])
			{
				neighbors[count++] = next;
			}
		}
		if (x < table.columns - 1)
		{
			LABIRIT_STEPS_ADD(1);
			int next = y * table.columns + (x + 1);
			if (!visited[next])
			{
				neighbors[count++] = next;
			}
		}
		if (y < table.rows - 1)
		{
			LABIRIT_STEPS_ADD(1);
			int next = (y + 1) * table.columns + x;
			if (!visited[next])
			{
				neighbors[count++] = next;
			}
		}
		if (x > 0)
		{
			LABIRIT_STEPS_ADD(1);
			int next = y * table.columns + (x - 1);
			if (!visited[next])
			{
				neighbors[count++] = next;
			}
		}

		if (count == 0)
		{
			/* Unlink from insertion order, including removal of newest. */
			int before = older[current];
			int after = newer[current];
			if (before != -1)
				newer[before] = after;
			if (after != -1)
				older[after] = before;
			else
				newest = before;

			/* Preserve the old dense-array order and random choices. */
			int moved = active[active_count - 1];
			active[chosen_pos] = moved;
			position[moved] = chosen_pos;
			active_count--;
			continue;
		}

		int next = neighbors[rand() % count];
		int next_x = next % table.columns;
		int next_y = next / table.columns;

		growing_tree_remove_wall(table, x, y, next_x, next_y);
		visited[next] = true;
		active[active_count] = next;
		position[next] = active_count++;
		older[next] = newest;
		newer[next] = -1;
		newer[newest] = next;
		newest = next;
	}

	free(storage);
	labirit_metrics_free(storage_bytes);
	free(visited);
	labirit_metrics_free((size_t)total * sizeof(bool));
	LABIRIT_STEPS_COMMIT();
}
