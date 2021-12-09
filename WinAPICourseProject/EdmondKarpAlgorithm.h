
typedef struct tagNetEdge {
	int flow;
	int capacity;
} netEdge;

void initializeNetworkWithZeros(void);
void initializeResidualNetworkWithZeros(void);
void initializeNetworkWithData(int**);
void handleAntiparallelEdges(void);
void updateResidualNetwork(void);
int DijkstraAlgorithm(int source, int sink);
void updateNetwork(int minResCap);
int EdmondKarpAlgorithmFunc(int, int, int**, netEdge(**)[100]);
