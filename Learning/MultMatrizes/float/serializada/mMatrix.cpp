#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <iomanip>

using namespace std;

int main (int argc, char *argv[]) {
	int l,m,n;
	double tStart, tEnd, tRun;
	float tmp;

	l = atoi(argv[1]);
	m = atoi(argv[1]);
	n = atoi(argv[1]);

	float A[l][m], B[m][n], C[l][n];
	srand(time(NULL));

	for(int i=0; i<l; i++) {
		for(int j=0; j<n; j++) {
			C[i][j] = 0;
		}
	}


	for ( int x = 0; x < l ; x ++ ) { 
		for (int y = 0; y < m ; y ++ ) { 
			tmp = (float)(rand()%100)/3;
			A[x][y] = x + y*2 + tmp;
		}   
	}

	for ( int x = 0; x < m ; x ++ ) { 
		for (int y = 0; y < n ; y ++ ) { 
			tmp = (float)(rand()%100)/3;
			B[x][y] = 3*x + y - tmp;
		}   
	}

	tStart = (double)clock()/CLOCKS_PER_SEC;

	for(int i=0; i<l; i++) {
		for(int j=0; j<m; j++) {
			for(int k=0; k<n; k++) {
				C[i][k] += A[i][j] * B[j][k];
			}
		}
	}

	tEnd = (double)clock()/CLOCKS_PER_SEC;

	tRun = tEnd - tStart;

	for(int x = 0; x < l; x++) {
		for( int y=0; y<m; y++) {
			std::cout << A[x][y] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	for(int x = 0; x < m; x++) {
		for( int y=0; y<n; y++) {
			std::cout << B[x][y] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	for(int x = 0; x < l; x++) {
		for( int y=0; y<n; y++) {
			std::cout << C[x][y] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Tempo de execução: " << tRun*1000 << "ms";
	std::cout << std::endl << std::setprecision(6);
	std::cout << tRun*1000 << std::endl;
	return 0;
}

