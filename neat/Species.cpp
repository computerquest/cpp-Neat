#include "stdafx.h"
#include "Species.h"

Species::Species()
{
}

void Species::addCI(int a)
{
}

void Species::removeCI(int a)
{
}

int * Species::getInovOcc(int i)
{
	return nullptr;
}

int * Species::incrementInov(int i)
{
	return nullptr;
}

int * Species::reduceInov(int i)
{
	return nullptr;
}

void Species::checkCI()
{
}

int * Species::getInnovationRef(int num)
{
	return nullptr;
}

int Species::createNewInnovation(int values[])
{
	return 0;
}

void Species::sortInnovation()
{
}

Network * Species::getNetworkAt(int a)
{
	return nullptr;
}

void Species::removeNetwork(int id)
{
}

Network * Species::getNetwork(int id)
{
	return nullptr;
}

void Species::addNetwork(Network * n)
{
}

void Species::updateStereotype()
{
}

void Species::mutateNetwork(Network * network)
{
}

Network Species::mateNetwork(Network nB, Network nA)
{
	return Network();
}

void Species::trainNetworks(double * trainingSet)
{
}

void Species::mateSpecies()
{
}

void Species::adjustFitness()
{
}

int Species::angNode()
{
	return 0;
}
