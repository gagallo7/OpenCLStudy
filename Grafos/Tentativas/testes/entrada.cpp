#include<iostream>
#include<stdio.h>

using namespace std;

int main (int argc, char *argv[]) {
	FILE *fp = fopen(argv[1],"r");
	char linha[2000];
	int i;
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return -1;
	}
	int count = 0;
	while (fgets (linha, 2000, fp) != NULL) {
		cout << endl << endl << ++count << " ";
		int ant = -1;
		while (1) {
			sscanf(linha," %d %[0-9 ]", &i, linha);
			if (ant == i) {
				break;
			}
			cout << i << " ";
			ant = i;
		}
		/*
		*/
		cout << "----------" << endl;
	}
	fclose(fp);
	return 0;
}
