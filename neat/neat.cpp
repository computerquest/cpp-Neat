#include "Neat.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>    // std::find
#include <iostream>
#include <vector>       // std::vector
#include <utility>
#include <array>
#include "Activation.h"
#include <thread>
#include <fstream>
#include <mutex>
using namespace std;

Neat::Neat(int numNetworks, int input, int output, double mutate, double lr, double(*activation)(double value), double(*activationDerivative)(double value)) : nodeMutate(mutate)
{
	threads = vector<std::thread>(8);
	speciesThreshold = .01;
	for (int i = output; i < input + output; i++) {
		for (int a = 0; a < output; a++) {
			pair<int, int> c = { i,a };
			connectionInnovation.push_back(c);
		}
	}

	network.reserve(numNetworks);

	for (int i = 0; i < numNetworks; i++) {
		network.push_back(Network(input, output, i, 0, lr, true, activation, activationDerivative));
	}

	createSpecies(0, network.size() % 3 + network.size() / 3);
	species[0].innovationDict = &connectionInnovation;
	for (int i = network.size() % 3 + (network.size() / 3); i + (network.size() / 3) <= network.size(); i += (network.size() / 3)) {
		createSpecies(i, i + (network.size() / 3));
	}

	mutatePopulation();

	speciateAll();
}

vector<double> Neat::start(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid, int cutoff, double target, Network& bestNet)
{
	vector<double> ans;
	int strikes = cutoff;
	double bestFit = 0;

	trainNetworks(input, valid);

	int z = 0;
	int extra = 10;
	while (strikes > 0 && extra >= 1) {
		cout << "//////////////////////////////////////////////////////////////" << endl;

		mateSpecies();

		trainNetworks(input, valid);

		if (z % 5 == 0) {
			speciateAll();
		}

		//determines the best
		int bestIndex = -1;
		for (int i = 0; i < network.size(); i++) {
			if (bestFit < network[i].fitness) {
				bestFit = network[i].fitness;
				bestIndex = i;
			}
		}

		//compares the best
		if (bestIndex != -1) {
			clone(network[bestIndex], bestNet, &connectionInnovation);
			strikes = cutoff;
		}
		else {
			strikes--;
			mutatePopulation();
			if (z % 5 != 0) {
				speciateAll();
			}
		}

		cout << "best" << endl;
		bestNet.printNetwork();
		cout << "epoch:" << z << " best: " << bestFit << endl;
		cout << endl;
		z++;

		if (bestFit > target) {
			if (extra == 5) {
				ans.push_back(bestFit);
			}
			extra--;
		}
	}

	ans.push_back(z);

	if (ans.size() < 2) {
		ans.push_back(-z);
	}

	return ans;
}


void Neat::mutatePopulation()
{
	for (int i = 0; i < species.size(); i++) {
		for (int a = 0; a < species[i].network.size(); a++) {
			species[i].mutateNetwork(*species[i].network[a]);
		}
	}
}

void Neat::trainNetworks(vector<pair<vector<double>, vector<double>>>& input, vector<pair<vector<double>, vector<double>>>& valid)
{
	threads.clear();
	for (int i = 0; i < species.size(); i++) {
		threads.push_back(thread(&Species::trainNetworks, &species[i], input, valid));
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}

	/*for (int i = 0; i < species.size(); i++) {
		species[i].trainNetworks(input, input);
	}*/
}

void Neat::mateSpecies()
{
	threads.clear();

	for (int i = 0; i < species.size(); i++) {
		threads.push_back(thread(&Species::mateSpecies, &species[i]));
	}
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}

	/*for (int i = 0; i < species.size(); i++) {
		species[i].mateSpecies();
	}*/
}

void Neat::speciateAll()
{
	for (int a = 0; a < species.size(); a++) {
		for (int i = 0; i < species[a].network.size(); i++) {
			speciate(*species[a].network[i], &species[a]);
		}
	}

	checkSpecies();
}

void Neat::checkSpecies()
{
	for (int i = 0; i < species.size(); i++) {
		vector<double> values;
		for (int a = 0; a < species.size(); a++) {
			if (a == i) {
				values.push_back(100.0);
				continue;
			}

			values.push_back(compareGenome(species[i].avgNode(), species[i].commonInnovation, species[a].avgNode(), species[a].commonInnovation));
		}

		double lValue = 1000.0;
		for (int a = 0; a < values.size(); a++) {
			if (values[a] < lValue) {
				lValue = values[a];
			}
		}

		if (lValue < speciesThreshold || species[i].network.size() < 2) { //switched direction if sign because %dif < difthreshold for it to be the same
			removeSpecies(species[i].id); //could say continue if similar so that the smaller does the hard workbut might screw with eliminating empties
			i--;
		}
	}
}

void Neat::speciate(Network& network, Species* s)
{
	vector<double> values;

	for (int i = 0; i < species.size(); i++) {
		values.push_back(compareGenome(network.nodeList.size(), network.innovation, species[i].avgNode(), species[i].commonInnovation));
	}

	//this should be faster than sorting the whole thing (it also retains position information)
	int bestSpec = -1;
	double lValue = 1000.0;
	for (int i = 0; i < values.size(); i++) {
		if (values[i] < lValue) {
			bestSpec = species[i].id;
			lValue = values[i];
		}
	}

	//s := n.getSpecies(network.species)
	if (lValue > speciesThreshold) { //&& s != nil && len(s.network) > 2 { //i flipped this sign i think it works better %different > differentThreshold
									 //finds the position
		int networkIndex = 0;
		for (int i = 0; i < this->network.size(); i++) {
			if (this->network[i].networkId == network.networkId) {
				networkIndex = i;
			}
		}
		if (s != nullptr) {
			s->removeNetwork(network.networkId);
		}
		int lastSpec = network.species;
		vector<Network*> pass;
		pass.push_back(&this->network[networkIndex]);
		Species& newSpec = createSpecies(pass);
		//TODO: there will be a problem when og species can't be found

		if (s != nullptr) {
			//removes current and checks to see if the rest need to be speciated
			s->removeNetwork(network.networkId);
			for (int i = 0; i < s->network.size(); i++) {
				if (s->network[i]->networkId != network.networkId) { // && s->network[i]->species == s->id) {
																	 //	compareGenome(len(s.network[i].nodeList), s.network[i].innovation, s.avgNode(), s.commonInnovation) > compareGenome(len(s.network[i].nodeList), s.network[i].innovation, newSpec.avgNode(), newSpec.commonInnovation) {
					if (compareGenome(s->network[i]->nodeList.size(), s->network[i]->innovation, s->avgNode(), s->commonInnovation) > compareGenome(s->network[i]->nodeList.size(), s->network[i]->innovation, newSpec.avgNode(), newSpec.commonInnovation)) {
						newSpec.addNetwork(*s->network[i]);
						s->removeNetwork(s->network[i]->networkId);
						i--;
					}
				}
			}
		}

		/* THIS IS FOR CHECKING ALL NETWORKS//cannot have more than one species removed at the same time
		for (int i = 0; i < this->network.size(); i++) {
		Network& comp = this->network[i];
		if (comp.networkId != network.networkId && (s != nullptr && comp.species == lastSpec)) { // && comp.species == s->id) {
		Species& cs = getSpecies(comp.species);												 //	compareGenome(len(s.network[i].nodeList), s.network[i].innovation, s.avgNode(), s.commonInnovation) > compareGenome(len(s.network[i].nodeList), s.network[i].innovation, newSpec.avgNode(), newSpec.commonInnovation) {
		if (compareGenome(comp.nodeList.size(), comp.innovation, cs.avgNode(), cs.commonInnovation) > compareGenome(comp.nodeList.size(), comp.innovation, newSpec.avgNode(), newSpec.commonInnovation)) {
		newSpec.addNetwork(comp);
		cs.removeNetwork(comp.networkId);
		i--;
		}
		}
		}*/
		//checks to see if new species meets size requirement
		if (newSpec.network.size() < 2) {
			//reassign creator to next best in order to prevent a loop
			//newSpec.removeNetwork(network.networkId);

			getSpecies(bestSpec).addNetwork(network); //could be problem because index changes when make new species (maybe because should be added to the end)
			removeSpecies(newSpec.id);

		}
	}
	else if (network.species != bestSpec) {
		getSpecies(bestSpec).addNetwork(network);

		if (s != nullptr) {
			s->removeNetwork(network.networkId);
		}
	}
}

/*
node: number of nodes in a network
innovation: list of innovation numbers for a network
nodeA/innovationA: the same as node and innovation but for a second network
*/
double Neat::compareGenome(int node, vector<int>& innovation, int nodeA, vector<int>& innovationA)
{
	vector<int>* larger; //larger number of innovations
	vector<int>*smaller; //smaller number of innovations

	//set the larger and smaller pointers
	if (innovation.size() > innovationA.size()) {
		larger = &innovation;
		smaller = &innovationA;
	}
	else {
		larger = &innovationA;
		smaller = &innovation;
	}


	int missing = 0; //number of differences in innovations
	//in case smaller has no innovations
	if (smaller->size() == 0) {
		missing = larger->size();
	}
	else {
		//counts the number of differences in genome
		for (int b = 0; b < larger->size(); b++) {
			if (find(smaller->begin(), smaller->end(), (*larger)[b]) == smaller->end()) {
				missing++;
			}
		}
	}

	//calculates the final double for similarity
	return (missing / (double)larger->size()) + 3*(abs(node - nodeA) / (double)(node + nodeA) / 2);
}

/*mutex writing;
//mutex reading;
//might be rare issue of writing lock right after a try_lock
pair<int, int> Neat::getInnovation(int num)
{
pair<int, int> ans;
writing.lock();
ans = connectionInnovation[num];
writing.unlock();
return ans;
}
int Neat::addInnovation(int a, int b)
{
lock_guard<mutex> w(writing);
connectionInnovation.push_back(pair<int, int>(a, b));
return connectionInnovation.size() - 1;
}
int Neat::innovationSize(int a)
{
lock_guard<mutex> m(writing);
return connectionInnovation.size();
}*/

void Neat::printNeat() {
	cout << endl;
	for (int i = 0; i < species.size(); i++) {
		cout << "Spec Id: " << species[i].id << " network size: " << species[i].network.size() << " " << endl;
		for (int a = 0; a < species[i].commonInnovation.size(); a++) {
			cout << species[i].commonInnovation[a] << " ";
		}
		cout << endl;
		for (int a = 0; a < species[i].network.size(); a++) {
			species[i].network[a]->printNetwork();
		}
	}
	cout << endl;
}
/*int * Neat::getInnovation(int num)
{
return nullptr;
}*/

/*int Neat::findInnovation(int search[2])
{
return 0;
}*/

Species& Neat::getSpecies(int id)
{
	for (int i = 0; i < species.size(); i++) {
		if (species[i].id == id) {
			return species[i];
		}
	}
	//TODO: need default return
}

Species& Neat::createSpecies(vector<Network*>& possible)
{
	for (int i = 0; i < possible.size(); i++) {
		possible[i]->species = speciesId;
	}

	species.push_back(Species(speciesId, possible, &connectionInnovation, nodeMutate));

	speciesId++;

	return species.back();
}

Species & Neat::createSpecies(int startIndex, int endIndex)
{
	vector<Network*> possible;
	for (int i = startIndex; i < endIndex; i++) {
		network[i].species = speciesId;
		possible.push_back(&network[i]);
	}

	species.push_back(Species(speciesId, possible, &connectionInnovation, nodeMutate));

	speciesId++;

	return species.back();
}

void Neat::removeSpecies(int id)
{
	for (int i = 0; i < species.size(); i++) {
		if (species[i].id == id) {
			vector<Network*> currentSpecies = species[i].network;

			species.erase(species.begin() + i);
			for (int a = 0; a < currentSpecies.size(); a++) {
				if (currentSpecies[a]->species == id) {
					speciate(*currentSpecies[a], nullptr);
				}
			}
		}
	}
}