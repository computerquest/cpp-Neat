
#include <string>
#include <iostream>
#include "Neat.h"
#include <fstream>
#include "Activation.h"
#include <chrono>
#include <mutex>
#include <sstream>
#include <mpi.h>

using namespace std;
using namespace chrono;

int id; //the id of this process
int ierr;
int p; //number of connected nodes
int populationSize = 3;
int desiredFitness = 1;

void runSample(vector<pair<vector<double>, vector<double>>>& data, int numTime) {
    for (int a = 0; a < numTime; a++) {
        //cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << " : " << a << endl;
        Neat neat = Neat(populationSize, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);

        Network winner;

        vector<double> epochs = neat.start(data, data, 10, desiredFitness, winner);
    }
}

int main(int argc, char *argv[]) {
    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &p);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &id);

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

    /*if(id == 0) {
        cout << "I am rank 0"  << " " << p << " " << ierr << " " << id << endl;
        int data = 100;
        MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } else {
        cout << "I am not rank 0 but I am still fun" << " " << p << " " << ierr << " " << id << endl;
        int data = -1;
        MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
        cout << "id " << id << " data: " << data << endl;
    }*/
    /*Network w;
    Neat n = Neat(100, 2, 1, .3, .1, &sigmoid, &sigmoidDerivative);
    n.start(data, data, 10, 1000000, w);*/

    while(populationSize < 6){
        MPI_Barrier(MPI_COMM_WORLD);
        while(desiredFitness <= 1) {
            MPI_Barrier(MPI_COMM_WORLD);
            cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< " << populationSize << " <<<<<<<<<<<>>>>>>>>>>>>> " << desiredFitness << endl;

            if(id == 0) {
                runSample(data, 1000 / p + 1000%p);
            } else {
                runSample(data, 1000 / p);
            }

            desiredFitness *= 10;
        }

        desiredFitness = 1;
        populationSize *= 2;
    }

    cout << "done";

    MPI_Finalize();

    return 0;
}