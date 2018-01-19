#include "Node.h"
#include "Activation.h"
using namespace std;

void Node::recieveValue()
{
	inputRecieved++;

	double sum = 0;
	for (int i = 0; i < recieve.size(); i++) {
		sum += (recieve[i]->nodeFrom->value) * recieve[i]->weight;
	}

	setValue(tanh(sum));
	inputRecieved = 0;
}

void Node::recieveInfluence()
{
	influenceRecieved++;

	if (influenceRecieved == send.size()) {
		influence = 0;
		for (int i = 0; i < send.size(); i++) {
			influence += send[i].nodeTo->influence * send[i].weight;
		}
		setInfluence(influence);
		influenceRecieved = 0;
	}
}

Connection * Node::addSendCon(Connection c)
{
	send.push_back(c);

	return &(send.back());
}

Connection * Node::addRecCon(Connection * c)
{
	recieve.push_back(c);

	return recieve.back();
}

Connection * Node::getRecCon(int i)
{
	return recieve[i];
}

Connection * Node::getSendCon(int i)
{
	return &send[i];
}

bool Node::connectsTo(int id)
{
	for (int i = 0; i < send.size(); i++) {
		if (send[i].nodeTo->id == id) {
			return true;
		}
	}

	return false;
}

void Node::setValue(double a)
{
	value = a;

	for (int i = 0; i < send.size(); i++) {
		send[i].notifyValue();
	}
}

void Node::setInfluence(double a)
{
	influence = a * tanhDerivative(value);
	for (int i = 0; i < recieve.size(); i++) {
		recieve[i]->notifyInfluence();
	}
}
