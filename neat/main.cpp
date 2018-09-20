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
vector<pair<vector<double>, vector<double>>> valid;

//dt is global and is used for the timestamp of files

void startfunc() {
	randInit();
	initDt();
}

mutex neatMutex;
void neatSample(int numTime, int populationSize, int desiredFitness, string mod) {
	for (int a = 1; a <= numTime; a++) {
		Neat neat = Neat(populationSize, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

		Network winner;

		milliseconds ms = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
		vector<double> epochs = neat.start(dataset, valid, 10, desiredFitness, winner);
		milliseconds finalTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) - ms;

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

		neatMutex.lock();
		ofstream bestNetworks;
		bestNetworks.open(".\\results\\networkTrial\\" + mod + " " + dt + ".txt", std::ios_base::app);
		bestNetworks << winner.fitness << " " << hiddenSize << " " << winner.innovation.size() << " " << (double)winner.innovation.size() / (double)winner.nodeList.size() << " " << sumi << " " << sumo << " " << (double)s / hiddenSize << " " << (double)r / hiddenSize << " " << (double)t / hiddenSize << " " << to_string(epochs[1]) << endl;

		bestNetworks.flush();
		bestNetworks.close();
		neatMutex.unlock();

		winner.write();
	}
}

mutex nsMutex;
void networkSample(Network* n, int iter, int numTime, string mod) {
	for (int a = 1; a <= numTime; a++) {
		double fitness = n->trainset(dataset, valid, iter);

		nsMutex.lock();

		ofstream o;
		o.open(".\\results\\networkTrial\\" + mod + " " + dt + ".txt", std::ios_base::app);
		o << n->fitness << " " << n->networkId << endl;
		o.flush();
		o.close();

		nsMutex.unlock();

		n->networkId += a;
		n->write();
	}
}
void networkSampleV(vector<Network>* allNet, int start, int end, int iter, string mod) {
	for (int a = start; a < end; a++) {
		(*allNet)[a].networkId = a;

		double fitness = (*allNet)[a].trainset(dataset, valid, iter);

		nsMutex.lock();

		ofstream o;
		o.open(".\\results\\networkTrial\\" + mod + " " + dt + ".txt", std::ios_base::app);
		o << 1 / fitness << " " << (*allNet)[a].networkId << endl;
		o.flush();
		o.close();

		nsMutex.unlock();

		(*allNet)[a].write();
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
	ifstream net(".\\results\\" + file);
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

void generateFullyConnected(vector<int> layers, Network& n) { //just needs hidden
	n = Network((int)dataset[0].first.size(), (int)dataset[0].second.size(), 0, 0, 1, false, &sigmoid, &sigmoidDerivative); //this is the base network that all the threads run off of (through clones)
	vector<Node*> lastLayer = n.input;
	int biasId = n.input.size() - 1 + n.output.size();
	for (int i = 0; i < layers.size(); i++) {
		vector<Node*> currentLayer;
		for (int a = 0; a < layers[i]; a++) {
			currentLayer.push_back(&n.createNode(100, &sigmoid, &sigmoidDerivative));
		}

		for (int a = 0; a < lastLayer.size(); a++) {
			for (int b = 0; b < currentLayer.size(); b++) {
				n.mutateConnection(lastLayer[a]->id, currentLayer[b]->id, 0);
			}
		}

		if (i > 0) {
			for (int b = 0; b < currentLayer.size(); b++) {
				n.mutateConnection(biasId, currentLayer[b]->id, 0);
			}
		}

		lastLayer = currentLayer;
	}

	for (int a = 0; a < lastLayer.size(); a++) {
		for (int b = 0; b < n.output.size(); b++) {
			n.mutateConnection(lastLayer[a]->id, n.output[b]->id, 0);
		}
	}

	for (int b = 0; b < n.output.size(); b++) {
		n.mutateConnection(biasId, n.output[b]->id, 0);
	}
}

void networkTrial(int trialSize, int iter, string mod) {
	vector<int> dem;
	for (int i = 0; i < 5; i++) {
		dem.push_back(20);
	}

	Network n;
	generateFullyConnected(dem, n);

	vector<Node*>lastLayer = n.input;

	vector<thread> threads;
	vector<Network> nets;
	nets.reserve(20);

	{
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());

		threads.push_back(thread(networkSample, &nets.back(), iter, (trialSize / 8) + (trialSize % 8), mod));
	}

	for (int i = 1; i < 8; i++) {
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());
		nets.back().networkId = 1000 * i;
		threads.push_back(thread(networkSample, &nets.back(), iter, trialSize / 8, mod));
	}

	std::cout << "threads launched" << endl;
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		std::cout << "thread: " << i << " is finished" << endl;
	}
}

void networkTrial(vector<Network>& allNets, int iter, string mod) {
	vector<thread> threads;
	int last = (allNets.size() / 8) + (allNets.size() % 8);

	{
		cout << "we are starting up " << " " << last << endl;
		threads.push_back(thread(networkSampleV, &allNets, 0, last, iter, mod));
	}

	for (int i = 1; i < 8; i++) {
		int temp = last + (allNets.size() / 8);
		cout << "and continueing " << last << " " << temp << endl;
		threads.push_back(thread(networkSampleV, &allNets, last, temp, iter, mod));
		last = temp;
	}

	std::cout << "threads launched" << endl;
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		std::cout << "thread: " << i << " is finished" << endl;
	}
}

void networkTrial(int trialSize, int iter, Network n, string mod) {
	vector<Node*>lastLayer = n.input;

	vector<thread> threads;
	vector<Network> nets;
	nets.reserve(20);

	{
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());

		threads.push_back(thread(networkSample, &nets.back(), iter, (trialSize / 8) + (trialSize % 8), mod));
	}

	for (int i = 1; i < 8; i++) {
		Network na;
		nets.push_back(na);

		Network::clone(n, nets.back());
		nets.back().networkId = 1000 * i;
		threads.push_back(thread(networkSample, &nets.back(), iter, trialSize / 8, mod));
	}

	std::cout << "threads launched" << endl;
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		std::cout << "thread: " << i << " is finished" << endl;
	}
}

void generatexor(int n, int dsize, double vp) {
	int vsize = vp * dsize;


	auto generate = [&n](vector<pair<vector<double>, vector<double>>>& add) {
		auto round = [](double in) -> int {
			if (in >= .5) {
				return 1;
			}
			else {
				return 0;
			}
		};

		vector<double> in;
		for (int a = 0; a < n; a++) {
			in.push_back(random(0., .99));
		}

		vector<int> xv;
		for (int a = 0; a < in.size(); a++) {
			xv.push_back(round(in[a]));
		}

		bool f = xv[0] ^ xv[1];
		for (int a = 2; a < xv.size(); a++) {
			f = f ^ xv[a];
		}

		vector<double> ans;
		ans.push_back(f);

		add.push_back(pair<vector<double>, vector<double>>(in, ans));
	};

	for (int i = 0; i < dsize; i++) {
		generate(dataset);
	}

	for (int i = 0; i < vsize; i++) {
		generate(valid);
	}
}

void writeData(string name) {
	ofstream o(".\\results\\dataset\\" + name + "_test.txt");
	for (int i = 0; i < dataset.size(); i++) {
		vector<double>& in = dataset[i].first;
		o << in[0];
		for (int a = 1; a < in.size(); a++) {
			o << " " << in[a];
		}
		o << endl;

		vector<double>& res = dataset[i].second;
		o << res[0];
		for (int a = 1; a < res.size(); a++) {
			o << " " << res[a];
		}
		o << endl;
	}

	o.flush();
	o.close();

	ofstream oa(".\\results\\dataset\\" + name + "_valid.txt");
	for (int i = 0; i < valid.size(); i++) {
		vector<double>& in = valid[i].first;
		oa << in[0];
		for (int a = 1; a < in.size(); a++) {
			oa << " " << in[a];
		}
		oa << endl;

		vector<double>& res = valid[i].second;
		oa << res[0];
		for (int a = 1; a < res.size(); a++) {
			oa << " " << res[a];
		}
		oa << endl;
	}

	oa.flush();
	oa.close();
}

void readData(string file) {
	ifstream net(".\\results\\dataset\\" + file + "_test.txt");
	string line = "";
	int inSize = -1;

	{
		ifstream net(".\\results\\dataset\\" + file + "_test.txt");
		string line = "";
		vector<double> currentInput;
		for (int i = 0; getline(net, line); i++) {
			vector<string> in;
			split(line, ' ', in);

			if (i == 0) {
				inSize = in.size();
			}

			if (in.size() == inSize) {
				currentInput.clear();
				for (int i = 0; i < in.size(); i++) {
					currentInput.push_back(stod(in[i]));
				}
			}
			else if (in.size() != inSize) {
				vector<double> id;
				for (int i = 0; i < in.size(); i++) {
					id.push_back(stod(in[i]));
				}

				dataset.push_back(pair<vector<double>, vector<double>>(currentInput, id));
			}
		}

		net.close();
	}

	{
		ifstream net(".\\results\\dataset\\" + file + "_valid.txt");
		string line = "";
		vector<double> currentInput;
		for (int i = 0; getline(net, line); i++) {
			vector<string> in;
			split(line, ' ', in);

			if (in.size() == inSize) {
				currentInput.clear();
				for (int i = 0; i < in.size(); i++) {
					currentInput.push_back(stod(in[i]));
				}
			}
			else if (in.size() != inSize) {
				vector<double> id;
				for (int i = 0; i < in.size(); i++) {
					id.push_back(stod(in[i]));
				}

				valid.push_back(pair<vector<double>, vector<double>>(currentInput, id));
			}
		}

		net.close();
	}
}

void validatexor(string net, string set) {
	readData(set);
	Network n;
	Network::read(net, n);

	n.calcFitness(valid);

	int c = 0;
	int nc = 0;
	for (int i = 0; i < valid.size(); i++) {
		double out = n.process(valid[i].first)[0];

		if (out >= .5 && valid[i].second[0] == 1 || out < .5 && valid[i].second[0] == 0) {
			c++;
		}
		else {

			for (int a = 0; a < valid[i].first.size(); a++) {
				cout << valid[i].first[a] << " ";
			}
			cout << valid[i].second[0] << " " << out << endl;

			nc++;
		}
	}

	cout << "valid: " << c << " " << nc << " " << c + nc << " " << (double)c / (nc + c) * 100 << "%" << endl;

	{
		c = 0;
		nc = 0;
		for (int i = 0; i < dataset.size(); i++) {
			double out = n.process(dataset[i].first)[0];

			if (out >= .5 && dataset[i].second[0] == 1 || out < .5 && dataset[i].second[0] == 0) {
				c++;
			}
			else {
				nc++;
			}
		}

		cout << "dataset: " << c << " " << nc << " " << c + nc << " " << (double)c / (nc + c) * 100 << "%" << endl;

	}
}

int main()
{
	startfunc();

	std::cout << "done";
	std::system("pause");
	return 0;
}