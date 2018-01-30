#include "stdafx.h"
#include <iostream>
#include "Neat.h"
using namespace std;

int main()
{
	vector<pair<vector<double>, vector<double>>> data;
	data.push_back(pair<vector<double>, vector<double>>(vector<double>(0, 1), vector<double>(1)));
	data.push_back(pair<vector<double>, vector<double>>(vector<double>(1, 1), vector<double>(0)));
	data.push_back(pair<vector<double>, vector<double>>(vector<double>(0, 0), vector<double>(0)));
	data.push_back(pair<vector<double>, vector<double>>(vector<double>(1, 0), vector<double>(1)));

	  Network winner(0,0,0,0,0.0, false);
	  Neat neat = Neat(10, 2, 1, .3, .01);

	  winner = neat.start(data, 100, 100000);
	  //neat.printNeat()

	  cout << endl;

	  //printNetwork(&winner);
	  cout << "best " << winner.fitness << "error" << 1 / winner.fitness << endl;
	  //cout << "result: " << winner.process(data[0].first) << winner.process(data[1].first) << winner.process(data[2].first) << winner.process(data[3].first) << endl; //1 1 0 0
	  cout << "done";
	  return 0;
}