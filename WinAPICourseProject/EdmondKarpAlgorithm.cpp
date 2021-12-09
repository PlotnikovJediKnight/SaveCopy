#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "EdmondKarpAlgorithm.h"



#define MAX_ARR_LENGTH 100
netEdge network[MAX_ARR_LENGTH][MAX_ARR_LENGTH];
int		 resNet[MAX_ARR_LENGTH][MAX_ARR_LENGTH];

const int MAX_VERTEXES = 20;

void initializeNetworkWithZeros(void) {
	for (int i = 0; i < MAX_ARR_LENGTH; i++)
		for (int j = 0; j < MAX_ARR_LENGTH; j++) {
			network[i][j].flow = 0;
			network[i][j].capacity = 0;
		}
}

void initializeResidualNetworkWithZeros(void) {
	for (int i = 0; i < MAX_ARR_LENGTH; i++)
		for (int j = 0; j < MAX_ARR_LENGTH; j++)
			resNet[i][j] = 0;
}

int SINK_VERTEX = 0;
int N = 0;
int M = 0;

void initializeNetworkWithData(int **w) {
	for (int i = 0; i < M; i++) {
		int u = w[i][0], v = w[i][1], c = w[i][2];

		u--;
		v--;

		network[u][v].capacity = c;
	}
}

void handleAntiparallelEdges(void) {
	for (int u = 0; u < N; u++) {
		for (int v = 0; v < N; v++) {
			if (network[u][v].capacity > 0 && 
				network[v][u].capacity > 0) {
				int c = network[v][u].capacity;
				network[v][u].capacity = 0;

				network[v][N].capacity = c;
				network[N][u].capacity = c;

				N++;
			}
		}
	}
}

void updateResidualNetwork(void) {
	for (int u = 0; u < N; u++) {
		for (int v = 0; v < N; v++) {
			int cuv = network[u][v].capacity;
			if (cuv > 0) {
				resNet[u][v] = cuv - network[u][v].flow;
			}
			else {
				resNet[u][v] = network[v][u].flow;
			}
		}
	}
}

short int   visited[MAX_ARR_LENGTH];
int		   distance[MAX_ARR_LENGTH];
int		   shortest[MAX_ARR_LENGTH];
#define INFINITY 9999999

int DijkstraAlgorithm(int source, int sink) {
	for (int i = 0; i < N; i++) {
		distance[i] = INFINITY;
		 visited[i] = 0;
		shortest[i] = -1;
	}

	distance[source] = 0;

	for (int i = 0; i < N; i++) {
		int min = INFINITY;
		int u = 0;

		for (int j = 0; j < N; j++)
			if (!visited[j] && distance[j] < min) {
				min = distance[j];
				u = j;
			}

		visited[u] = 1;

		for (int j = 0; j < N; j++)
			if (!visited[j] && resNet[u][j] > 0 && distance[u] + 1 < distance[j]) {
				distance[j] = distance[u] + 1;
				shortest[j] = u;
			}
	}

	if (distance[sink] == INFINITY) {
		return 0;
	}
	else {
		int minResCap = INFINITY;
		int curr = sink, prev = sink;

		while (shortest[curr] != -1) {
			curr = shortest[curr];
			minResCap = (minResCap > resNet[curr][prev]) ? resNet[curr][prev] : minResCap;
			prev = curr;
		}

		return minResCap;
	}
}

void updateNetwork(int minResCap) {
	int curr = SINK_VERTEX, prev = SINK_VERTEX;

	while (shortest[curr] != -1) {
		curr = shortest[curr];

		if (network[curr][prev].capacity > 0)
			network[curr][prev].flow += minResCap;
		else
			network[prev][curr].flow -= minResCap;

		prev = curr;
	}
}

int checkForNetworkness(void) {
	int flag = 0;
	for (int i = 0; i < N; i++) {
		if (network[0][i].capacity > 0) {
			flag = 1;
			break;
		}
	}
	if (!flag) return -1;

	flag = 0;
	for (int i = 0; i < N; i++) {
		if (network[i][N - 1].capacity > 0) {
			flag = 1;
			break;
		}
	}

	if (!flag) return -2;

	flag = 0;
	for (int i = 1; i < N - 1; i++) {

		int flag1 = 0;
		for (int j = 0; j < N; j++) {
			if (network[i][j].capacity > 0) {
				flag1 = 1;
				break;
			}
		}

		int flag2 = 0;
		for (int j = 0; j < N; j++) {
			if (network[j][i].capacity > 0) {
				flag2 = 1;
				break;
			}
		}

		if (!(flag1 && flag2)) {
			flag = 1;
			break;
		}
	}

	if (flag) return -3;

	return 0;
}

int EdmondKarpAlgorithmFunc(int _N, int _M, int **w, netEdge (**globPointer)[100]) {
	initializeNetworkWithZeros();
	initializeResidualNetworkWithZeros();

	N = _N;
	M = _M;
	SINK_VERTEX = N - 1;
	int SOURCE_VERTEX = 0;
	initializeNetworkWithData(w);

	int res = checkForNetworkness();

	switch (res) {
	case -1:
		return -1;
		break;
	case -2:
		return -2;
		break;
	case -3:
		return -3;
		break;
	case 0:;
	}

	handleAntiparallelEdges();
	updateResidualNetwork();

	while (1) {
		int minResCap = DijkstraAlgorithm(SOURCE_VERTEX, SINK_VERTEX);
		if (minResCap == 0) break;
		updateNetwork(minResCap);
		updateResidualNetwork();
	}

	int MFLOW = 0;
	for (int i = 0; i < N; i++)
		MFLOW += network[i][SINK_VERTEX].flow;

	*globPointer = network;

	return MFLOW;
}