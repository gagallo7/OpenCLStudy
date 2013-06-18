#include<iostream>
#include<stdio.h>
#include<vector>

using namespace std;

/*
class edge {
	public:
	int dest, peso;
	edge (int d, int p) {
		this->dest = d;
		this->peso = p;
	}
};

typedef struct Edge {
	int dest, peso;
} edge;
*/

int main (int argc, char *argv[]) {
	FILE *fp = fopen(argv[1],"r");

	vector <int> vert;
	vector < int > arestas, pesos;


	char linha[2000];
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return -1;
	}
	// Primeiro vertice comeca sua lista de adjacencias em 0
	vert.push_back(0);

	// Cada linha tem a representacao de cada vertice
	// fgets limita-se a capturar uma linha
	int ant = -1;
	int i, peso;
	while (fgets (linha, 2000, fp) != NULL) {
		// Capturando o vertice
		while (1) {
			sscanf(linha," %d %d %[0-9 ]", &i, &peso,  linha);
			// Caso de fim de linha, pois nao ha arestas iguais
			// Logo, o destino nao pode ser o mesmo
			if (ant == i) {
				break;
			}
//			edge e(i,peso);		// Criando aresta de peso 1
			cout << i << " ";
			/*
			e = (edge *)alloca(sizeof(edge));
			e->dest = i;
			e->peso = peso;
			*/
			ant = i;
			arestas.push_back(i);
			pesos.push_back(peso);
		}
		/*
		*/
		cout << "----------" << endl;
		// Apontando para a proxima lista de adjacencias
		vert.push_back(arestas.size());
	}
	fclose(fp);

	cout << "Vetor de vertices:" << endl;
	vector < int >::iterator it, it2;
	for (it=vert.begin();it!=vert.end();it++) {
		cout << *it << " ";
	}
	cout << endl;

//	vector < edge >::iterator ee;
	cout << "Vetor de arestas:" << endl;
	it2 = pesos.begin();
	for (it=arestas.begin();it!=arestas.end();it++) {
		cout << *it << ":" << *it2 << " ";
		it2++;
	}
	cout << endl;

	return 0;
}
