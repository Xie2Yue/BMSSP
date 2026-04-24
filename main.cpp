#include "Path.h"
#include "BBDLL.h"
#include "Graph.h"
#include "BMSSP.h"
#include <iostream>


using namespace std;

int main() {
	int a[] = {1000,2000,5000,10000,20000,30000,40000,50000,60000,70000,80000,90000,100000};
	ofstream fb("bmssp.res");
	
	BMSSPAlgo<double> A;
	double avgtB, avgcuv, avgtD, B = 100.0;
	int n, m, t = 100;
	
	for(int tt = 0; tt < 13; tt++) {
		n = a[tt];
		m = n*10;
		avgtB = 0, avgcuv = 0, avgtD = 0;

		for(int i = 1; i <= t; i ++) {
			auto [Bt, Dt, cuv] = A.calc(n, m, B, 0);
			
			double tB = Bt.count()/1000.0;
			double tD = Dt.count()/1000.0;
			
			avgtB += tB;
			avgtD += tD;
			avgcuv += 1.0*cuv/n*100;
		}
		avgtB /= t;
		avgtD /= t;
		avgcuv /= t;
		cout << "n = " << n << ", m = " << m << "\n";
		cout << "BMSSP use " << avgtB << " ms\n";
		cout << "Dijkstra use " << avgtD << " ms\n";
		cout << "use " << avgcuv << "% vertices\n";
		fb << n << " " << avgtB << " " << avgtD << " " << avgcuv << "\n";

	}
	return 0;
}