#pragma once
#include <vector>
#include "Network.h"
using namespace std;

class Species {
public:
	static vector<int*>* innovationDict;
	
	vector<Network*> network;
	vector<int> connectionInnovation;
	vector<int> commonInnovation;
	int id;
	double mutate;

	Species(int id, vector<Network*> networks, double mutate);
	Species(int id, vector<Network>& networks, double mutate);

	void addCI(int a);
	void removeCI(int a);
	int& getInovOcc(int i);
	int& incrementInov(int i);
	int& reduceInov(int i);
	void checkCI();
	int* getInnovationRef(int num);
	int createNewInnovation(int values[2]);
	void sortInnovation();

	Network& getNetworkAt(int a);
	void removeNetwork(int id);
	Network& getNetwork(int id);
	void addNetwork(Network& n);

	void updateStereotype();
	void mutateNetwork(Network& network);
	Network mateNetwork(vector<int>& nB, vector<int>& nA, int nodeNum, int nodeNumA);
	void trainNetworks(vector<vector<vector<double>>>& trainingSet);
	void mateSpecies();
	void adjustFitness();
	int avgNode();
};