#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <iomanip>

/*
#define TAM 2500

unsigned int l = TAM;
unsigned int m = TAM;
unsigned int n = TAM;
int A[TAM][TAM], B[TAM][TAM], C[TAM][TAM];

*/
using namespace std;

int main (int argc, char *argv[]) {
//int main () {
	double tStart, tEnd, tRun;

/*
	l = atoi(argv[1]);
	m = atoi(argv[1]);
	n = atoi(argv[1]);
*/

	int nn = atoi(argv[1]);
	int l = nn, m = nn, n = nn;

	int **A, **B, **C;
	A = (int **)malloc(sizeof(int *)*nn);
	B = (int **)malloc(sizeof(int *)*nn);
	C = (int **)malloc(sizeof(int *)*nn);

	for(int i=0; i<l; i++) {
		A[i] = (int *)malloc(sizeof(int)*nn);
		B[i] = (int *)malloc(sizeof(int)*nn);
		C[i] = (int *)malloc(sizeof(int)*nn);
		for(int j=0; j<n; j++) {
			C[i][j] = 0;
		}
	}


	for ( int x = 0; x < l ; x ++ ) { 
		for (int y = 0; y < m ; y ++ ) { 
			A[x][y] = x + y*2;
		}   
	}

	for ( int x = 0; x < m ; x ++ ) { 
		for (int y = 0; y < n ; y ++ ) { 
			B[x][y] = 3*x + y;
			B[x][y] |= 1;
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

/*
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
	*/
	std::cout << std::endl;

	std::cout << "Tempo de execução: " << tRun*1000 << "ms";
	std::cout << std::endl << std::setprecision(6);
	std::cout << tRun*1000 << std::endl;

	free(A);
	free(B);
	free(C);
	return 0;
}

