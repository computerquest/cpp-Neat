#pragma once

class Node;

class Connection {
public:
	double weight;
	double nextWeight;
	Node* nodeTo;
	Node* nodeFrom;
	int innovation;
	
	Connection(Node* fromNode, Node* toNode, int inNumber);

	void randWeight();
	void notifyValue();
	void notifyInfluence();
};