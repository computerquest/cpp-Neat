#include "stdafx.h"
#include "Network.h"
Network::Network(int inputI, int outputI, int id, int species, double learningRate, bool addCon)
{
	networkId = id;
	this->learningRate = learningRate;
	this->species = species;

	//create output nodes
	for (int i = 0; i < outputI; i++) {
		output[i] = createNode(0);
	}

	//creates the input nodes and adds them to the network
	int startInov = 0; //this should work
	for (int i = 0; i < inputI; i++) {
		input[i] = createNode(100);
		if (addCon) {
			for (int a = 0; a < outputI; a++) {
				mutateConnection(input[i]->id, output[a]->id, startInov);
				startInov++;
			}
		}
	}
	input[inputI] = createNode(100); //bias starts unconnected and will form connections over time
}

void Network::printNetwork()
{
}

vector<double> Network::process(double input[]) {
	//set input values
	for (int i = 0; i < sizeof(input) / sizeof(input[0]); i++) {
		if (i < this->input.size()) {
			this->input[i]->setValue(input[i]);
		}
		else {
			this->input[i]->setValue(1);
		}
	}

	vector<double> ans;
	//values are calculated via connections and nodes signalling
	for (int i = 0; i < output.size(); i++) {
		ans[i] = output[i]->value;
	}

	return ans;
}

double Network::backProp(double input[], double desired[])
{
	return 0.0;
}

double Network::trainset(double * input, int lim)
{
	return 0.0;
}

int Network::getInnovation(int pos)
{
	return 0;
}

void Network::addInnovation(int num)
{
}

Network clone(Network * n)
{
	return Network();
}

bool Network::containsInnovation(int num)
{
	return false;
}

void Network::removeInnovation(int num)
{
}

void Network::mutateConnection(int from, int to, int innovation)
{
}

int Network::numConnection()
{
	return 0;
}

void Network::resetWeight()
{
}

Node * Network::getNode(int i)
{
	return nullptr;
}

Node * Network::createNode(int send)
{
	return nullptr;
}

int Network::getNextNodeId()
{
	return 0;
}

int Network::mutateNode(int from, int to, int innovationA, int innovationB)
{
	return 0;
}

bool Network::checkCircleMaster(Node * n, int goal)
{
	return false;
}

bool Network::checkCircle(Node * n, int goal, int preCheck[])
{
	return false;
}

Network::~Network()
{
	delete[] output;
	delete[] input;
}
