#pragma once
#include <vector>
#include "Network.h"
using namespace std;

class Species {
public:
	vector<Network*> network;
	vector<int> connectionInnovation;
	vector<int[2]>* innovationDict;
	int id;
	double mutate;

	Species();

	void addCI(int a);
	void removeCI(int a);
	int* getInovOcc(int i);
	int* incrementInov(int i);
	int* reduceInov(int i);
	void checkCI();
	int* getInnovationRef(int num);
	int createNewInnovation(int values[]);
	void sortInnovation();

	Network* getNetworkAt(int a);
	void removeNetwork(int id);
	Network* getNetwork(int id);
	void addNetwork(Network* n);

	void updateStereotype();
	void mutateNetwork(Network* network);
	Network mateNetwork(Network nB, Network nA);
	void trainNetworks(double* trainingSet);
	void mateSpecies();
	void adjustFitness();
	int angNode();
};