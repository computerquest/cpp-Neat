#include "stdafx.h"
#include "Connection.h"
#include "Node.h"
#include <cstdlib>
using namespace std;

Connection::Connection(Node * fromNode, Node * toNode, int inNumber)
{
	weight = 0;
	nodeFrom = fromNode;
	nodeTo = toNode;
	innovation = inNumber;

	randWeight();
}

void Connection::randWeight()
{
	srand(unsigned(time(NULL)));
	weight = rand()*.2 + .4;
}

void Connection::notifyValue()
{
	nodeTo->recieveValue();
}

void Connection::notifyInfluence()
{
	nodeFrom->recieveInfluence();
}
