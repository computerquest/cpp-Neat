#pragma once
#include <vector>
#include <map>
#include "Network.h"
using namespace std;

vector<int> transform(map<vector<int>, vector<int> > signmatrixA,
	map<vector<int>, vector<int> > signmatrixB,
	vector<int> vertexA, vector<int> vertexB,
	vector<int> isoB);

vector<int> inv(vector<int> index);

vector<vector<int> > reindex(vector<vector<int> > graph, vector<int> index);

vector<vector<int> > dijkstra(vector<vector<int> > graph);

bool calc(vector<vector<int>> rgraphA, vector<vector<int>> rgraphB);

map<int, int> calcBetween(Network& aNet, Network& bNet);