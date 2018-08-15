#include "stdafx.h"
#include <iostream>
#include "Neat.h"
#include "Activation.h"
#include <fstream>
#include <string>
#include <chrono>
#include <sstream>
#include <mutex>
using namespace std;
using namespace chrono;

void split(const std::string &s, char delim, vector<string> &result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		result.push_back(item);
	}
}
void readNets(string file, vector<Network>& allNets) {
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

mutex mainMutex;
void write(Network& winner, string file) {
	mainMutex.lock();

	ofstream bestNetworks;
	bestNetworks.open(file);

	bestNetworks << winner.input.size() - 1 << " " << winner.output.size() << " " << acttoString(winner.nodeList[0].activation).c_str() << " " << winner.fitness << " " << int(epochs[1]) << endl;
	
	for (int i = 0; i < winner.nodeList.size(); i++) {
		Node& n = winner.nodeList[i];
		bestNetworks << n.id << " " << acttoString(n.activation).c_str() << endl;
	}
	for (int i = 0; i < winner.nodeList.size(); i++) {
		for (int a = 0; a < winner.nodeList[i].send.size(); a++) {
			bestNetworks << winner.nodeList[i].id << " " << winner.nodeList[i].send[a].nodeTo->id << " " << winner.nodeList[i].send[a].weight << endl;
		}
	}

	bestNetworks << endl;
	bestNetworks.flush();
	bestNetworks.close();

	mainMutex.unlock();
}

int main()
{
	vector<pair<vector<double>, vector<double>>> data;
	{
		pair<vector<double>, vector<double>> p;
		vector<double> in;
		in.push_back(0);
		in.push_back(1);
		vector<double> o;
		o.push_back(1);
		p.first = in;
		p.second = o;
		data.push_back(p);
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
		data.push_back(p);
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
		data.push_back(p);
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
		data.push_back(p);
	}

	randInit();

	Neat neat = Neat(200, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

	Network winner;
	
	neat.start(data, data, 10, 10000000, winner);
	//neat.printNeat()

	cout << endl;

	//printNetwork(&winner);
	cout << "best " << winner.fitness << " error " << 1 / winner.fitness << endl;
	cout << "result: " << winner.process(data[0].first)[0] << " " << winner.process(data[1].first)[0] << " " << winner.process(data[2].first)[0] << " " << winner.process(data[3].first)[0] << endl; //1 1 0 0
	cout << "done";
	system("pause");
	return 0;
}