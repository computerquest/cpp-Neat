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
int desiredFitness = 1000000;
int populationSize = 3;
int z = 0;
vector<Network> allNets;

mutex j;
void write(Network& winner) {
	j.lock();

	bestNetworks.open("verification.txt", fstream::app);
	int sumCon = 0;
	for (int i = 0; i < winner.nodeList.size(); i++) {
		sumCon += winner.nodeList[i].send.size();
	}

	cout << "finished " << z << endl;
	bestNetworks <<  winner.nodeList.size() << " " << sumCon << endl;

	bestNetworks.flush();
	bestNetworks.close();

	z++;
	j.unlock();
}

mutex mainMutex;
void write(vector<double> epochs, Network& winner, int time) {
	mainMutex.lock();
	z++;
	cout << "finished " << z << endl;
	rawData.open("testResults.txt", fstream::app);
	bestNetworks.open("bestNets.txt", fstream::app);

	rawData << populationSize << " " << desiredFitness << " " << epochs[0] << " " << winner.fitness << " " << int(epochs[1]) << " " << time << endl;
	rawData.flush();
	rawData.close();

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
void runSample(vector<pair<vector<double>, vector<double>>>& data, int numTime) {
	for (int a = 1; a < numTime; a++) {
		//cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << " : " << a << endl;
		Neat neat = Neat(populationSize, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

		Network winner;

		milliseconds ms = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
		vector<double> epochs = neat.start(data, data, 10, desiredFitness, winner);
		milliseconds finalTime = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()) - ms;

		//cout << endl;

		//cout << "best " << winner.fitness << " error " << 1 / winner.fitness << endl;
		//cout << "result: " << winner.process(data[0].first)[0] << " vs " << to_string(data[0].second[0]) << " " << winner.process(data[1].first)[0] << " vs " << to_string(data[1].second[0]) << " " << winner.process(data[2].first)[0] << " vs " << to_string(data[2].second[0]) << " " << winner.process(data[3].first)[0] << " vs " << to_string(data[3].second[0]) << endl; //1 1 0 0

		write(epochs, winner, finalTime.count());
	}
}
void split(const std::string &s, char delim, vector<string> &result) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		result.push_back(item);
	}
}

Species s;

mutex r;
void runTrial(Network& a, int start, int end) {
	while (start < end) {
		Network& net = allNets[start];

		Network n;
		s.mateNetwork(a.nodeList, net.nodeList, net.fitness > a.fitness, n);
		write(net);
		start++;
	}
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

	/*desiredFitness = 1000000;
	{
		cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << endl;
		vector<thread> threads;

		for (int i = 0; i < 8; i++) {
			threads.push_back(thread(runSample, data, 125));
		}

		cout << "threads launched" << endl;
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
			cout << "thread: " << i << " is finished" << endl;
		}
	}*/

	ifstream net("bestNets.txt");
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

	cout << "num nets " << allNets.size() << endl;
	s.network.push_back(&allNets[0]);

	vector<thread> threads;
	for (int i = 0; i < 7; i++) {
		threads.push_back(thread(runTrial, allNets[i], int(i+1), int(allNets.size()-i-1)));
	}

	cout << "threads launched" << endl;
	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
		cout << "thread: " << i << " is finished" << endl;
	}

	cout << "done";
	system("pause");
	return 0;
}