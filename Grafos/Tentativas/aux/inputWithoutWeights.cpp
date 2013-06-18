#include<iostream>
#include<stdio.h>
#include<vector>

using namespace std;

int main (int argc, char *argv[]) {
	FILE *fp = fopen(argv[1],"r");

	vector <int> vert, arestas;

	char linha[2000];
	int i;
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return -1;
	}
	// Primeiro vertice comeca sua lista de adjacencias em 0
	vert.push_back(0);

	// Cada linha tem a representacao de cada vertice
	// fgets limita-se a capturar uma linha
	int ant = -1;
	while (fgets (linha, 2000, fp) != NULL) {
		// Capturando o vertice
		while (1) {
			sscanf(linha," %d %[0-9 ]", &i, linha);
			// Caso de fim de linha, pois nao ha arestas iguais
			// Logo, o destino nao pode ser o mesmo
			if (ant == i) {
				break;
			}
			//edge e(i,1);		// Criando aresta de peso 1
			cout << i << " ";
			ant = i;
			arestas.push_back(i);
		}
		/*
		*/
		cout << "----------" << endl;
		// Apontando para a proxima lista de adjacencias
		vert.push_back(arestas.size());
	}
	fclose(fp);

	cout << "Vetor de vertices:" << endl;
	vector < int >::iterator it;
	for (it=vert.begin();it!=vert.end();it++) {
		cout << *it << " ";
	}
	cout << endl;

	cout << "Vetor de arestas:" << endl;
	for (it=arestas.begin();it!=arestas.end();it++) {
		cout << *it << " ";
	}
	cout << endl;

	return 0;
}
