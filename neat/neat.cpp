#include "Neat.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
using namespace std;

Neat::Neat(int numNetworks, int input, int output, double mutate, double lr) : nodeMutate(mutate)
{
	for (int i = output; i < input + output; i++) {
		for (int a = 0; a < output; a++) {
			connectionInnovation.push_back({ i,a });
		}
	}

	for (int i = 0; i < network.size(); i++) {
		network.push_back(Network(input, output, i, 0, lr, true));
	}

	createSpecies(vector<Network>(network.begin() + 0, network.begin() + network.size() % 5 + network.size() / 5));
	for (int i = network.size() % 5 + (network.size() / 5); i + (network.size() / 5) <= network.size(); i += (network.size() / 5)) {
		createSpecies(vector<Network>(network.begin() + i, network.begin() + i + (network.size() / 5)));
	}

	for (int i = 0; i < species.size(); i++) {
		for (int a = 0; a < species[i].network.size(); a++) {
			species[i].mutateNetwork(*species[i].network[a]);
		}
	}
	speciateAll();
	checkSpecies();
}

Network Neat::start(vector<vector<vector<double>>>& input, int cutoff, double target)
{
	return Network();
}

void Neat::mutatePopulation()
{
	srand(time(NULL));
	int numNet = (rand() % network.size() - 3) / 5 + 3;
	for (int i = 0; i < numNet; i++) {
		int species = int(rand() % (this->species.size() - 1));

		this->species[species].mutateNetwork(this->species[species].getNetworkAt(rand() % this->species[species].network.size()));
	}
}

void Neat::speciateAll()
{
	for (int i = 0; i < network.size(); i++) {
		speciate(network[i]);
	}
}

void Neat::checkSpecies()
{
	for (int i = 0; i < species.size(); i++) {
		vector<double> values(species.size());
		for (int a = 0; a < species.size(); a++) {
			if (a == i) {
				values[a] = 100.0;
				continue;
			}

			values[a] = compareGenome(species[i].avgNode(), species[i].commonInnovation, species[a].avgNode(), species[a].commonInnovation);
		}

		int lValue = 1000.0;
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

void Neat::speciate(Network& network)
{
	vector<double> values(species.size());

	for (int i = 0; i < species.size(); i++) {
		values[i] = compareGenome(network.nodeList.size(), network.innovation, species[i].avgNode(), species[i].commonInnovation);
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

		int lastSpec = network.species;
		Species& newSpec = createSpecies(vector<Network>(this->network.begin() + networkIndex, this->network.begin() + networkIndex + 1));

		//remove from the old species
		Species& s = getSpecies(lastSpec);
		//removes current and checks to see if the rest need to be speciated
		s.removeNetwork(network.networkId);
		for (int i = 0; i < s.network.size(); i++) {
			if (s.network[i]->networkId != network.networkId && s.network[i]->species == s.id) {
				if (compareGenome(s.network[i]->nodeList.size(), s.network[i]->innovation, s.avgNode(), s.commonInnovation) > compareGenome(s.network[i]->nodeList.size(), s.network[i]->innovation, newSpec.avgNode(), newSpec.commonInnovation)) {
					newSpec.addNetwork(*s.network[i]);
					s.removeNetwork(s.network[i]->networkId);
					i--;
				}
			}
		}

		//checks to see if new species meets size requirement
		if (newSpec.network.size() < 2) {
			//reassign creator to next best in order to prevent a loop
			newSpec.removeNetwork(network.networkId);
			getSpecies(bestSpec).addNetwork(network); //could be problem because index changes when make new species (maybe because should be added to the end)

			removeSpecies(newSpec.id);
		}
	}
	else if (network.species != bestSpec) {
		Species& lastSpec = getSpecies(network.species);
		getSpecies(bestSpec).addNetwork(network);

		if (lastSpec.network.size() != 0) {
			lastSpec.removeNetwork(network.networkId);
		}
	}
}

double Neat::compareGenome(int node, vector<int> innovation, int nodeA, vector<int> innovationA)
{
	return 0.0;
}

int * Neat::getInnovation(int num)
{
	return nullptr;
}

int Neat::findInnovation(int search[2])
{
	return 0;
}

Species& Neat::getSpecies(int id)
{
}

Species& Neat::createSpecies(vector<Network>& possible)
{
}

void Neat::removeSpecies(int id)
{
}
