#pragma once
#include <vector>
#include <string>
using namespace std;

double tanH(double value);
double tanhDerivative(double value);

double sigmoid(double value);
double sigmoidDerivative(double value);

double lRelu(double value);
double lReluDerivative(double value);

int random(int f, int t);
double random(double f, double t);
void randInit();

typedef double(*activation)(double value);
typedef double(*activationDerivative)(double value);

activation stringtoAct(string in);
activationDerivative stringtoDeriv(string in);

string acttoString(activation a);

pair<int, int> safeRead(vector<pair<int, int>>& connectionInnovation, int a);
int safeWrite(vector<pair<int, int>>& connectionInnovation, int a, int b);