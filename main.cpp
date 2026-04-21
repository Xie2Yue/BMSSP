#include "Path.h"
#include "BBDLL.h"
#include "Graph.h"
#include "BMSSP.h"
#include <iostream>


using namespace std;

int main() {
	BMSSPAlgo<double> A;
	for(int i = 1; i <= 10; i ++) cout << A.calc(10000, 20000, 2000.0, 0).count()  << "ms\n";
	return 0;
}