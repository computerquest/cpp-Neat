#include "stdafx.h"
#include <iostream>
#include "Neat.h"
#include "Activation.h"
using namespace std;

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

	for (int i = 0; i < 100; i++) {
		cout << random(-1.0, 1.0) << endl;
	}
	Network winner(0, 0, 0, 0, 0.0, false);
	Neat neat = Neat(250, 2, 1, .3, .1);

	winner = neat.start(data, 20, 100000);
	//neat.printNeat()

	cout << endl;

	//printNetwork(&winner);
	cout << "best " << winner.fitness << "error" << 1 / winner.fitness << endl;
	//cout << "result: " << winner.process(data[0].first) << winner.process(data[1].first) << winner.process(data[2].first) << winner.process(data[3].first) << endl; //1 1 0 0
	cout << "done";
	system("pause");
	return 0;
}
