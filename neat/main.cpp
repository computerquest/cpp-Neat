#include "stdafx.h"
#include <string>
#include <iostream>
#include "Neat.h"
#include <fstream>
#include "Activation.h"
#include <chrono>
#include <mutex>
#include <sstream>
using namespace std;
using namespace chrono;


ofstream rawData;
ofstream bestNetworks;
int z = 0;
vector<Network> allNets;
vector<pair<vector<double>, vector<double>>> dataset;

void neatSample(int numTime, int populationSize, int desiredFitness) {
	for (int a = 1; a < numTime; a++) {
		Neat neat = Neat(populationSize, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

		Network winner;

		milliseconds ms = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
		vector<double> epochs = neat.start(dataset, dataset, 10, desiredFitness, winner);
		milliseconds finalTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) - ms;
	}
}


void mateSample(Network& a, int start, int end) {
	Species s;
	while (start < end) {
		Network& net = allNets[start];

		Network n;
		s.mateNetwork(a.nodeList, net.nodeList, net.fitness > a.fitness, n);
		start++;
	}
}

void networkSample(int id) {
	Network n(2, 1, id, 0, .01, false, &sigmoid, &sigmoidDerivative);

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

	n.trainset(dataset, dataset, 100000);
}

void ezPrune(Network& n) {
	double check = .005;
	for (int z = 0; z < 10; z++) {
		cout << "iter: " << z << " " << n.nodeList.size() << endl;
		for (int i = 0; i < n.nodeList.size(); i++) {
			double sum = 0;

			for (int a = 0; a < n.nodeList[i].recieve.size(); a++) {
				sum += abs(n.nodeList[i].recieve[a]->weight);
			}

			for (int a = 0; a < n.nodeList[i].recieve.size(); a++) {
				n.nodeList[i].recieve[a]->weight /= sum;

				if (abs(n.nodeList[i].recieve[a]->weight) < check) {
					cout << n.nodeList[i].recieve[a]->nodeFrom->id << " " << n.nodeList[i].id << " " << n.nodeList[i].recieve[a]->weight << endl;
					n.removeConnection(n.nodeList[i].recieve[a]->nodeFrom->id, n.nodeList[i].id);
				}
				else {
					n.nodeList[i].recieve[a]->weight *= sum;
				}
			}
		}

		for (int i = 0; i < n.nodeList.size(); i++) {
			if (n.nodeList[i].send.size() == 0 && n.nodeList[i].recieve.size() != 0 && n.nodeList[i].id != 0) {
				cout << "remove: " << n.nodeList[i].id << endl;
				n.removeNode(n.nodeList[i].id);
			}
		}
	}

	n.remap();

	n.write("testingStuff.txt");
}

void loadAllNets(vector<Network>& allNets, string file) {
	ifstream net(file);
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
				cout << fixed << stod(in[3]) << endl;
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

void neatTrial() {
	int desiredFitness = 1;
	int populationSize = 3;
	{
		cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << endl;
		vector<thread> threads;

		for (int i = 0; i < 8; i++) {
			threads.push_back(thread(neatSample, dataset, 125, populationSize, desiredFitness));
		}

		cout << "threads launched" << endl;
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
			cout << "thread: " << i << " is finished" << endl;
		}
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


	cout << "num nets " << allNets.size() << endl;
	
	vector<thread> threads;
	for (int i = 0; i < 8; i++) {
		//s.network.push_back(&allNets[0]);
		/*vector<thread> threads;
		for (int i = 0; i < 7; i++) {
			threads.push_back(thread(networkTrial, data, i));
		}

		cout << "threads launched" << endl;
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
			cout << "thread: " << i << " is finished" << endl;
		}*/
		cout << allNets[0].calcFitness(dataset) << endl;

		ezPrune(allNets[0]);
	}
	cout << allNets[0].calcFitness(dataset) << endl;
	cout << "done";
	system("pause");
	return 0;
}