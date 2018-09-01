#include "stdafx.h"
#include <string>
#include <iostream>
#include "Neat.h"
#include <fstream>
#include "Activation.h"
#include <chrono>
#include "Network.h"
#include <mutex>
#include <sstream>
#include<ctime>
using namespace std;
using namespace chrono;


vector<pair<vector<double>, vector<double>>> dataset;
//dt is global and is used for the timestamp of files

mutex neatMutex;
void neatSample(int numTime, int populationSize, int desiredFitness, string mod) {
	for (int a = 1; a < numTime; a++) {
		Neat neat = Neat(populationSize, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

		Network winner;

		milliseconds ms = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
		vector<double> epochs = neat.start(dataset, dataset, 10, desiredFitness, winner);
		milliseconds finalTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) - ms;

		winner.write();

		neatMutex.lock();

		ofstream bestNetworks;
		bestNetworks.open(".\\results\\networkTrial\\" + mod + " " + dt + ".txt", std::ios_base::app);
		int sumi = 0;
		for (int i = 0; i < winner.input.size(); i++) {
			sumi += winner.input[i]->send.size();
		}

		int sumo = 0;
		for (int i = 0; i < winner.output.size(); i++) {
			sumo += winner.output[i]->recieve.size();
		}

		int s = 0;
		int r = 0;
		int t = 0;
		int nodeSize = winner.input.size() + winner.output.size();
		int hiddenSize = winner.nodeList.size() - nodeSize;
		for (int i = nodeSize; i < winner.nodeList.size(); i++) {
			if (winner.nodeList[i].activation == &sigmoid) {
				s++;
			}
			else if (winner.nodeList[i].activation == &lRelu) {
				r++;
			}
			else {
				t++;
			}
		}


		bestNetworks << winner.fitness << " " << hiddenSize << " " << winner.innovation.size() << " " << (double)winner.innovation.size() / (double)winner.nodeList.size() << " " << sumi << " " << sumo << " " << (double)s / hiddenSize << " " << (double)r / hiddenSize << " " << (double)t / hiddenSize << " " << to_string(epochs[1]) << endl;

		bestNetworks.flush();
		bestNetworks.close();
		neatMutex.unlock();
	}
}

mutex nsMutex;
void networkSample(Network* n, int iter, int numTime, string mod) {
	for (int a = 0; a < numTime; a++) {
		double fitness = n->trainset(dataset, dataset, iter);

		nsMutex.lock();

		ofstream o;
		o.open(".\\results\\networkTrial\\" +mod + " " + dt + ".txt" , std::ios_base::app);
		o << 1/fitness << endl;
		o.flush();
		o.close();

		nsMutex.unlock();

		n->networkId += a;
		n->write();
	}
}

void ezPrune(Network& n, double check) {
	for (int z = 0; z < 10; z++) {
		std::cout << "iter: " << z << " " << n.nodeList.size() << endl;
		for (int i = 0; i < n.nodeList.size(); i++) {
			double sum = 0;

			for (int a = 0; a < n.nodeList[i].recieve.size(); a++) {
				sum += abs(n.nodeList[i].recieve[a]->weight);
			}

			for (int a = 0; a < n.nodeList[i].recieve.size(); a++) {
				n.nodeList[i].recieve[a]->weight /= sum;

				if (abs(n.nodeList[i].recieve[a]->weight) < check) {
					std::cout << n.nodeList[i].recieve[a]->nodeFrom->id << " " << n.nodeList[i].id << " " << n.nodeList[i].recieve[a]->weight << endl;
					n.removeConnection(n.nodeList[i].recieve[a]->nodeFrom->id, n.nodeList[i].id);
				}
				else {
					n.nodeList[i].recieve[a]->weight *= sum;
				}
			}
		}

		for (int i = 0; i < n.nodeList.size(); i++) {
			if (n.nodeList[i].send.size() == 0 && n.nodeList[i].recieve.size() != 0 && n.nodeList[i].id != 0) {
				std::cout << "remove: " << n.nodeList[i].id << endl;
				n.removeNode(n.nodeList[i].id);
			}
		}
	}

	n.remap();

	n.calcFitness(dataset);

	n.write();
}

void loadAllNets(vector<Network>& allNets, string file) {
	ifstream net("./Result Files/" + file);
	string line = "";
	bool newNet = true;
	for (int i = 0; getline(net, line); i++) {
		if (line == "" & !newNet) {
			newNet = true;
		}
		else {
			vector<string> in;
			split(line, ' ', in);
			if (newNet) {
				allNets.push_back(Network(stoi(in[0]), stoi(in[1]), 0, 0, .1, false, stringtoAct(in[2]), stringtoDeriv(in[2])));
				allNets.back().fitness = stod(in[3]);
				std::cout << fixed << stod(in[3]) << endl;
				newNet = false;
			}
			else if (in.size() == 3) {
				allNets.back().mutateConnection(stoi(in[0]), stoi(in[1]), 0, stod(in[2]));
			}
			else if (in.size() == 2) {
				if (stoi(in[0]) < allNets.back().input.size() + allNets.back().output.size()) {
					continue;
				}
				allNets.back().createNode(100, stringtoAct(in[1]), stringtoDeriv(in[1]));
				allNets.back().nodeList.back().id = stoi(in[0]);
			}
		}
	}

}

void neatTrial(string mod) {
	int desiredFitness = 1;
	int populationSize = 3;
	{
		std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << endl;
		vector<thread> threads;

		for (int i = 0; i < 8; i++) {
			threads.push_back(thread(neatSample, 125, populationSize, desiredFitness, mod));
		}

		std::cout << "threads launched" << endl;
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
			std::cout << "thread: " << i << " is finished" << endl;
		}
	}
}

void networkTrial(int trialSize, int iter, string mod) {
	Network n(2, 1, 0, 0, .01, false, &sigmoid, &sigmoidDerivative); //this is the base network that all the threads run off of (through clones)

	vector<Node*>lastLayer = n.input;
	for (int i = 0; i < 5; i++) {
		vector<Node*> currentLayer;
		for (int a = 0; a < 10; a++) {
			currentLayer.push_back(&n.createNode(100, &sigmoid, &sigmoidDerivative));
		}

		for (int a = 0; a < lastLayer.size(); a++) {
			for (int b = 0; b < currentLayer.size(); b++) {
				n.mutateConnection(lastLayer[a]->id, currentLayer[b]->id, 0);
			}
		}

		lastLayer = currentLayer;
	}
	for (int a = 0; a < lastLayer.size(); a++) {
		for (int b = 0; b < n.output.size(); b++) {
			n.mutateConnection(lastLayer[a]->id, n.output[b]->id, 0);
		}
	}

	vector<thread> threads;
	vector<Network> nets;
	nets.reserve(20);

	{
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());

		threads.push_back(thread(networkSample, &nets.back(), iter, (trialSize / 8) + (trialSize % 8), mod));
	}

	for (int i = 0; i < 7; i++) {
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());
		nets.back().networkId += ((trialSize / 8) + 1)*i;
		threads.push_back(thread(networkSample, &nets.back(), iter, trialSize / 8, mod));
	}

	std::cout << "threads launched" << endl;
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		std::cout << "thread: " << i << " is finished" << endl;
	}
}

int main()
{

	{
		pair<vector<double>, vector<double>> p;
		vector<double> in;
		in.push_back(0);
		in.push_back(1);
		vector<double> o;
		o.push_back(1);
		p.first = in;
		p.second = o;
		dataset.push_back(p);
	}
	{
		pair<vector<double>, vector<double>> p;
		vector<double> in;
		in.push_back(1);
		in.push_back(0);
		vector<double> o;
		o.push_back(1);
		p.first = in;
		p.second = o;
		dataset.push_back(p);
	}
	{
		pair<vector<double>, vector<double>> p;
		vector<double> in;
		in.push_back(1);
		in.push_back(1);
		vector<double> o;
		o.push_back(0);
		p.first = in;
		p.second = o;
		dataset.push_back(p);
	}
	{
		pair<vector<double>, vector<double>> p;
		vector<double> in;
		in.push_back(0);
		in.push_back(0);
		vector<double> o;
		o.push_back(0);
		p.first = in;
		p.second = o;
		dataset.push_back(p);
	}

	randInit();
	initDt();

	networkTrial(100, 1000000, "");

	std::cout << "done";
	system("pause");
	return 0;
}