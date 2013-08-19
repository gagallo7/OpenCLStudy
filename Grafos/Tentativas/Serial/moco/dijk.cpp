// Guilherme Alcarde Gallo.
// Baseado em:
// MC558 - Lab 4 - Djkstra
// Alexandre Luiz Brisighello Filho 

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

using namespace std;

// Classe Heap
class heap {
public:
	int size;
	vector < vector < int > > vheap;
	vector < int > map;
	
	void print(int vertices); int pop(); void heapfy (int k);
};

// Imprimi : Debug
void heap::print(int vertices) {
	int n,a,b;
	
	cout << "Size: " << size << endl;
		
	cout << "map: " << endl;
	for(n=0; n<vertices; n=n+1) cout << n << ":" << map[n] <<  " ";
	
	cout << endl << "vheap: " << endl;
	for(n=1; n<=vertices; n=n+1) cout << "(" << vheap[n][0] << " , " << vheap [n][1] << ")";
	cout << endl;	
}

// Remove o primeiro
int heap::pop() {
	int a;
	
	map[vheap[1][1]]=-1; map[vheap[size][1]]=1;
	
	a=vheap[1][1];
	vheap[1][0]=vheap[size][0]; vheap[1][1]=vheap[size][1];
	size=size-1;
	
	heapfy(1);
	return a;
}

// Heapfica
void heap::heapfy(int k) {
	int fe,fd,pai,maior, aux1, aux2, aux3;
	fe=2*k; fd=2*k+1; pai=k/2; maior=k;
	
	//cout << "fe: " << fe << " fd: " << fd << " pai:" << pai << endl;
	
	if(fe<=size) if(vheap[fe][0]<vheap[k][0]) maior=fe;
	if(fd<=size) if(vheap[fd][0]<vheap[maior][0]) maior=fd;
	if(pai!=0) if(vheap[pai][0]>vheap[maior][0]) maior=pai;
	
	if(maior!=k) {
		aux1=vheap[k][1]; aux2=vheap[maior][1]; aux3=map[aux1]; map[aux1]=map[aux2]; map[aux2]=aux3; 
		aux1=vheap[k][0]; aux2=vheap[k][1]; vheap[k][0]=vheap[maior][0]; vheap[k][1]=vheap[maior][1]; vheap[maior][0]=aux1; vheap[maior][1]=aux2;
		heapfy(maior);
	}
}

// Classe Aresta
class edge {
public:
	int destino;	int peso;
};

// Classe que representa o grafo orientado
class graph {
public:
	int vertices; 	int arestas;
	vector < vector < edge > > lista;  
	
	vector < int > pi;		vector < int > d;
	heap myheap;
	int s;
	
	void init();
	void printit();
	void relax(int u, int v, int w);
	void dijkstra();
	void printPath(int x, int psize);
	void Results(char*[]);
};
/*
void graph::Results() {
	int n;
	for (n=0; n<vertices; n=n+1) {
		cout << d[n] << " ";
		printPath(n, 1);
		cout << endl;		// Linha problematica
	}
}
*/
void graph::Results(char* argv[]) {
	FILE *fp;
	fp = fopen(argv[1],"w");

	int n;
	for (n=0; n<vertices; n=n+1) {
		cout << d[n] << " ";
		cout << endl;		// Linha problematica
		fprintf(fp, "%d\n", d[n]);
	}

	fclose(fp);
}

void graph::printPath(int x, int psize){
	if (pi[x]==-1) {
		if(d[x]!=1073741824) {
			cout << psize << " ";
			cout << x;
			if (psize > 1)	cout << " ";
		}
		else 
			cout << "0";
	}
	else {
		printPath(pi[x], psize+1);
		if (psize==1) cout <<  x;
		else cout << x << " ";	
	}
}

// Executa o algoritmo Dijkstra
void graph::dijkstra(){
	int min;
	//myheap.print(vertices);
	//myheap.print(vertices);
	while(myheap.size>0){
		
		min=myheap.pop();
		//cout << "Pop safetly Min:" << min << endl;
		//myheap.print(vertices);
		for(vector < edge >::iterator it=lista[min].begin(); it!=lista[min].end(); it++)	{
			//cout << "Relax " << min << " " << (*it).destino << " " << (*it).peso << endl; 
			relax(min, (*it).destino, (*it).peso);
		}
	}
}

// Print : Debug
void graph::printit() {
	int n,k;

	cout << endl << "HEAP" << endl;		myheap.print(vertices);
	
	cout << endl << "GRAPH: " << endl;
	for (n=0; n<vertices;n=n+1) {	cout << "v:_" << n << ":" << endl;	vector < edge >::iterator it=lista[n].begin();
		for (k=0; k<lista[n].size(); k=k+1)		{cout << "------"  << (*it).destino << " - " << (*it).peso << endl; it++;}
	} cout << endl;
	
	cout << "d: ";
	for (n=0; n<vertices;n=n+1) cout << d[n] << " , ";
	cout << endl << "pi ";
	for (n=0; n<vertices;n=n+1) cout << pi[n] << " , ";
	cout << endl;
}

void graph::init() {
	int n, k;
	
	cin >> vertices >> arestas;
	lista.resize(vertices);		pi.resize(vertices);		d.resize(vertices);
	
	for (n=0; n<arestas; n=n+1) {
		edge term;
		cin >> k >> term.destino >> term.peso;
		lista[k].push_back(term);		
	}
	
	for(n=0; n<vertices; n=n+1) {
		d[n]=1073741824;
		pi[n]=-1;
	}
	cin >> s;	d[s]=0;
	
	myheap.vheap.resize((vertices+1));	// Aumenta o heap
	myheap.map.resize(vertices);
	myheap.size=vertices;
	
	for(n=0; n<vertices; n=n+1) {	// Inicializa o Heap
		myheap.vheap[n+1].resize(2);
		myheap.vheap[n+1][0]=1073741824;
		myheap.vheap[n+1][1]=n;
		myheap.map[n]=n+1;
	}
	
	// Poe o S no inicio do Heap
	myheap.vheap[s+1][1]=0;		myheap.vheap[1][1]=s;		myheap.vheap[1][0]=0;		myheap.map[0]=s+1;		myheap.map[s]=1;
}

void graph::relax(int u, int v, int w) {
	//cout << "d[v]=" << d[v] << " d[u]:" << d[u] << " w:" << w << " (d[u]+w):" << d[u]+w << endl;
	if (d[v]>(d[u]+w)) {	// Se for menor, atualizar d e heap
		d[v]=d[u]+w;	pi[v]=u;
		
		myheap.vheap[(myheap.map[v])][0]=d[v];
		//cout << "Atualizei Heap: " << endl;
		//myheap.print(vertices);
		myheap.heapfy(myheap.map[v]);
	}
}


int main (int argc, char* argv[]) {
	double tstart, tstop, ttime;
	graph mygraph;
	mygraph.init();
	//cout << "Initialize" << endl;
	tstart = (double)clock()/CLOCKS_PER_SEC;
	mygraph.dijkstra();
	tstop = (double)clock()/CLOCKS_PER_SEC;

	ttime = tstop - tstart;
	//cout << "Dijkstra" << endl;
	//mygraph.printit();
	//mygraph.Results(argv);
	cout << ttime*1000;
}
