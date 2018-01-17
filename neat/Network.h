#pragma once
#include <vector>
#include "Node.h"
using namespace std;

class Network {
public:
	vector<Node> nodeList;
	vector<int> innovation;
	double learningRate;
	Node *input;
	Node *output;
};