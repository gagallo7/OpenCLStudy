#include<iostream>
#include<stdio.h>

using namespace std;

int main (int argc, char *argv[]) {
	FILE *fp = fopen(argv[1],"r");
	char linha[1000];
	int i;
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return -1;
	}
	while (fgets (linha, 1000, fp) != NULL) {
		sscanf(linha," %d", &i);
		cout << i << endl;
		i = -1;
		while (i) {
			sscanf(linha," %d", &i);
			cout << i << endl;
				i = -1;
		}
		cout << "----------" << endl;
	}
	fclose(fp);
	return 0;
}
