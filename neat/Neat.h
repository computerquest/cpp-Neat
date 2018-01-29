#pragma once
#include "Network.h"
#include "Species.h"
using namespace std;

class Neat {
public:
	double nodeMutate;
	vector<Network> network;
	vector<int[2]> connectionInnovation;
	double speciesThreshold;
	vector<Species> species;
	int speciesId;

	Neat(int numNetworks, int input, int output, double mutate, double lr);

	Network start(vector<vector<vector<double>>>& input, int cutoff, double target);
	//void printNeat();
	void mutatePopulation();
	
	void speciateAll();
	void checkSpecies();
	void speciate(Network& n);
	double compareGenome(int node, vector<int> innovation, int nodeA, vector<int> innovationA);

	int* getInnovation(int num);
	int findInnovation(int search[2]);
	Species& getSpecies(int id);
	Species& createSpecies(vector<Network>& possible);
	void removeSpecies(int id);
};