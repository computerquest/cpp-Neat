#include <math.h>
#include <random>
#include <ctime>
#include "Activation.h"
using namespace std;

typedef std::mt19937 MyRNG;  // the Mersenne Twister with a popular choice of parameters
uint32_t seed_val;           // populate somehow

MyRNG rng;                   // e.g. keep one global instance (per thread)

double tanh(double value) {
	return (pow(2.71, value) - pow(2.71, -1 * value)) / (pow(2.71, value) + pow(2.71, -1 * value));
}
double tanhDerivative(double value) {
	return 1 - pow(tanh(value), 2);
}
double sigmoid(double value) {
	return 1 / (1 + (1 / pow(2.71, value)));
}
double sigmoidDerivative(double value) {
	return sigmoid(value) * (1 - sigmoid(value));
}

void randInit() {
	rng.seed(time(NULL));
}
//generates number [f,t]
int random(int f, int t)
{
	std::uniform_int_distribution<uint32_t> uint_dist10(f, t);

	return uint_dist10(rng);
}
