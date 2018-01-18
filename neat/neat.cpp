#include "Neat.h"
using namespace std;

Neat::Neat()
{
}

Network Neat::start(double * input, int cutoff, double target)
{
	return Network();
}

void Neat::printNeat()
{
}

void Neat::mutatePopulation()
{
}

void Neat::speciateAll()
{
}

void Neat::checkSpecies()
{
}

void Neat::speciate()
{
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

Species * Neat::getSpecies(int id)
{
	return nullptr;
}

Species * Neat::createSpecies(Network possible[])
{
	return nullptr;
}

void Neat::removeSpecies(int id)
{
}
