#include "Path.h"
#include "BBDLL.h"
#include "Graph.h"
#include "BMSSP.h"
#include <iostream>

const double INF = 1e18;

using namespace std;

int main() {
	BMSSPAlgo<double> A;
	A.calc(10000, 20000, 2000.0, 0);
	return 0;
}