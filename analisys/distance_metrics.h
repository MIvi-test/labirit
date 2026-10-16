#ifndef LABIRIT_DISTANCE_METRICS_H
#define LABIRIT_DISTANCE_METRICS_H

#include "../deter.h"

typedef struct
{
	double aspl;	// Среднее кратчайшее расстояние между различными клетками.
	double entropy; // Энтропия распределения расстояний в битах.
} DistanceMetrics;

DistanceMetrics compute_distance_metrics(
	MazeTable table); // Реализация находится в distance_metrics.c.

#endif // LABIRIT_DISTANCE_METRICS_H
