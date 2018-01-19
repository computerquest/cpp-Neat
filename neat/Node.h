#pragma once
#include "Connection.h"
#include <vector>
using namespace std;

class Node {
public:
	double value;
	int id;
	vector<Connection*> recieve;
	vector<Connection> send;
	double influence;
	int inputRecieved;
	int influenceRecieved;

	void recieveValue();
	void recieveInfluence();
	Connection* addSendCon(Connection c);
	Connection* addRecCon(Connection* c);
	Connection* getRecCon(int i);
	Connection* getSendCon(int i);
	bool connectsTo(int id);
	void setValue(double value);
	void setInfluence(double value);
};

bool isInput(Node* n);
bool isOutput(Node* n);