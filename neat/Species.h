#pragma once
#include <vector>
#include "Network.h"
using namespace std;

class Species {
public:
	vector<Network*> network;
	vector<int> connectionInnovation;
	vector<int> commonInnovation;
	vector<int[2]>& innovationDict;
	int id;
	double mutate;

	Species(int id, vector<Network*> networks, vector<int[2]> &innovations, double mutate);
	Species(int id, vector<Network>& networks, vector<int[2]>& innovations, double mutate);

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
	void trainNetworks(vector<pair<vector<double>, vector<double>>>& trainingSet);
	void mateSpecies();
	void adjustFitness();
	int avgNode();
};