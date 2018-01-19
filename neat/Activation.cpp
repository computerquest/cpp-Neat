#include <math.h>
using namespace std;

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