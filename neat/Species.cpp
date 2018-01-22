#include "stdafx.h"
#include "Species.h"
#include <algorithm>
#include<ctime> //for time function
#include<cstdlib>// for srand function.
using namespace std;

Species::Species(int id, vector<Network*> networks, vector<int[2]>& innovations, double mutate): innovationDict(innovations)
{
	this->id = id;
	this->mutate = mutate;
	this->network = networks;

	updateStereotype();
}

void Species::addCI(int a)
{
	for (int i = 0; i < commonInnovation.size(); i++) {
		if (commonInnovation[i] == a) {
			return;
		}
	}

	commonInnovation.push_back(a);
}

void Species::removeCI(int a)
{
	for (int i = 0; i < commonInnovation.size(); i++) {
		if (commonInnovation[i] == a) {
			commonInnovation.erase(commonInnovation.begin() + i);
		}
	}
}

int& Species::getInovOcc(int i)
{
	return &(connectionInnovation[i]);
}

int& Species::incrementInov(int i)
{
	int& ans = getInovOcc(i);
	(ans)++;

	if (double((ans) / network.size()) >= .5) {
		addCI(i);
	}

	return ans;
}

int& Species::reduceInov(int i)
{
	int& ans = getInovOcc(i);
	(ans)--;

	if (double((ans) / network.size()) < .5) {
		removeCI(i);
	}

	return ans;
}

void Species::checkCI()
{
	for (int i = 0; i < commonInnovation.size(); i++) {
		removeCI(commonInnovation[i]);
	}

	for (int i = 0; i < connectionInnovation.size(); i++) {
		if (double(connectionInnovation[i] / network.size()) >= .5) {
			addCI(i);
		}
	}
}

int* Species::getInnovationRef(int num)
{
	return (innovationDict)[num];
}

int Species::createNewInnovation(int values[2])
{
	//dictControl.Lock() TODO: fix the multithread
	(innovationDict).push_back(values);
	//defer dictControl.Unlock()

	return innovationDict.size() - 1;
}

void Species::sortInnovation()
{
	for (int i = 0; i < network.size(); i++) {
		sort(network[i]->innovaton.begin(), network[i]->innovaton.end());
	}
}

Network& Species::getNetworkAt(int a)
{
	return network[a];
}

void Species::removeNetwork(int id)
{
	for (int i = 0; i < network.size(); i++) {
		if (network[i].networkId == id) {
			vector<int> inn = network[i].innovation;
			network.erase(network.begin() + i);

			for (int a = 0; a < inn.size(); a++) {
				reduceInov(inn[a]);
			}

			checkCI();
		}
	}
}

Network& Species::getNetwork(int id)
{
	for (int i = 0; i < network.size(); i++) {
		if (network[i].networkId == id) {
			return network[i];
		}
	}
}

void Species::addNetwork(Network& n)
{
	network.push_back(&n);
	int species = id;

	for (int i = 0; i < n.innovation.size(); i++) {
		incrementInov(n.innovation[i]);
	}

	checkCI();
}

void Species::updateStereotype()
{
	int numNodes = 0;

	for (int i = 0; i < connectionInnovation.size(); i++) {
		connectionInnovation[i] = 0;
	}

	for (int i = 0; i < commonInnovation.size(); i++) {
		removeCI(commonInnovation[i]);
	}

	for (int i = 0; i < network.size(); i++) {
		numNodes += network[i].nodeList.size();
		for (int a = 0; a < network[i].innovation.size(); a++) {
			incrementInov(network[i].innovation[a]);
		}
	}
}

void Species::mutateNetwork(Network& network)
{
	srand((unsigned)time(0)); 
	int nodeRange = network.nodeList.size();

	//finds or adds innovation numbers and returns the innovation
	auto addConnectionInnovation = [&](int numFrom, int numTo) {
		//checks to see if preexisting innovation
		int maxPos = innovationDict.size();
		for (int i = 0; i < maxPos; i++) {
			int* pos = getInnovationRef(i);
			if (pos[1] == numTo && pos[0] == numFrom) {
				incrementInov(i);

				return i;
			}
		}

		//checks to see if needs to grow
		int arr[] = { numFrom, numTo };
		int num = createNewInnovation(arr);

		incrementInov(num);

		return num;
	};

	//mutates a node
	auto nodeMutate = [&]() {
		int  firstNode = -1;
		int secondNode = -1;
		bool ans = false;

		//picks a node
		while (!ans) {
			firstNode = int(rand() *nodeRange);

			if (network.getNode(firstNode).send.size() > 0) {
				ans = true;
			}
		}

		//picks a random connection from firstNode and gets the id
		secondNode = network.getNode(firstNode).send[int(rand()*network.getNode(firstNode).send.size())].nodeTo.id; //int(r.Int63n(int64(nodeRange)));

		network.mutateNode(firstNode, secondNode, addConnectionInnovation(firstNode, network.getNextNodeId()), addConnectionInnovation(network.getNextNodeId(), secondNode));
	};

	//randomly picks if node or connection mutate
	if (rand() <= mutate) {
		nodeMutate();
	}
	else {
		int firstNode;
		int secondNode;
		//ans := true
		int attempts = 0; //attempts at finding nodes

							   //find 2 unconnected nodes
							   //for ans && attempts <= 10 {
		while(attempts <= 10) {
			firstNode = rand()*nodeRange;
			secondNode = rand()*nodeRange;

			if (firstNode == secondNode || isOutput(network.getNode(firstNode)) || isInput(network.getNode(secondNode))) {
				continue;
			}

			if (network.getNode(firstNode).connectsTo(secondNode) || network.getNode(secondNode).connectsTo(firstNode) || network.checkCircleMaster(network.getNode(firstNode), secondNode)) {
				attempts++;
				continue;
			}
		}

		//muateNode if attempts exceeded else add the connection
		if (attempts > 10) {
			nodeMutate();
		}
		else {
			network.mutateConnection(firstNode, secondNode, addConnectionInnovation(firstNode, secondNode));
		}
	}
}

Network Species::mateNetwork(Network& nB, Network& nA)
{
	return Network();
}

void Species::trainNetworks(vector<vector<vector<double>>>& trainingSet)
{
}

void Species::mateSpecies()
{
}

void Species::adjustFitness()
{
}

int Species::angNode()
{
	return 0;
}
