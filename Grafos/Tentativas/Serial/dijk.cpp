#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

#define INF 999999999

using namespace std;

class edge {

	public:
		bool operator<( const edge & d ) const {
			return peso > d.peso;
		}
		int n, peso;
		edge (int a, int b) {
			n = a;
			peso = b;
		}
};

class grafo {

	public:
		vector < list < edge > > aL; // Lista de adjacencias
		int v; // total de vertices

		int make_graph (void);
		void print_graph (void);
		void add_edge (int, int, int);
		void prim (int);
		void initializeSingleSource 
			(int, vector <int>&, vector <int>&);
		void relax (int, int, int, vector <int>&, vector <int>&, vector <int>&, vector<int>&);
		void dijkstra (int);
		void FloydWarshall ();
		int bellmanFord (int);
};

void minHeapify (vector < int >& Q, vector < int >& map, int n, int i, vector < int >& d) {
	/*
	 */
	int l, r, menor;
	int aux, auxN;
	l = 2*i+1;
	r = 2*i+2;

	if (l<n && d[ Q[l] ]<=d[ Q[i] ]) {
		menor = l;
	}
	else {
		menor = i;
	}

	if (r<n && d[ Q[r] ]<d[ Q[menor] ]) {
		menor = r;
	}

	if (menor != i) {
		aux = Q[i];
		map[ Q[i] ] = menor;
		map[ Q[menor] ] = i;
		Q[i] = Q[menor];
		Q[menor] = aux;
		minHeapify(Q,map,n,menor,d);
	}
}

void minHeapify2 (vector < int >& Q, vector < int >& map, int n, int i, vector < int >& d) {
	/* 
	 */
	int l, r, menor;
	int aux, auxN;
	l = (i-1)/2;

	menor = i;
	if (l>=0 && d[ Q[l] ] >= d[ Q[i] ]) {
		menor = l;
	}

	if (menor != i) {
		aux = Q[i];
		map[ Q[i] ] = menor;
		map[ Q[menor] ] = i;
		Q[i] = Q[menor];
		Q[menor] = aux;
		minHeapify2(Q,map,n,menor,d);
	}
}

int inverse (vector < int > map, int v) {
	for (int i = 0; i < map.size(); i++) {
		if (map[i]==v) {
			return i;
		}
	}
	return -1;
}

void printHeap (vector < int > Q, vector < int > d, vector < int > map) {
	for (int i = 0; i < Q.size(); i++) {
		if (d[Q[i]] < 999999999) 
			cout << Q[ i ] << "(" << d[ Q[i] ] << ") ";

		else
			cout << Q[i] << "(inf) ";
	}
	cout << endl;
}

void grafo::relax(int u, int v, int w,
		vector < int >& d, vector < int >& pi,
		vector < int >& Q, vector < int >& map) {
	if (d[u] == 999999999)
		return;
	if (d[v] > d[u] + w) {
	//	cout << d[v] << " > " << d[u] << " + " << w << " d[" << v << "] = " << d[u]+w << endl;
		d[v] = d[u] + w;
		pi[v] = u;
	}
	//cout << "Map[" << v << "]:" << map [v] << endl;
	/*
	if (map[v]>=0) {
		int aux = Q[ 0 ];
		Q[0] = v;
		Q[ map[v] ] = aux;
		map[v] = 0;
		minHeapify(Q, map, Q.size(), 0, d);
	}
	*/
	minHeapify2(Q, map, Q.size(), v, d);
	minHeapify(Q, map, Q.size(), 0, d);
	/*
	*/
	//printHeap(Q, d, map);
}

int extractMin (vector < int >& Q, vector < int >& map, vector < int >& d) {
	int tmp = Q[0];

	map[Q[0]] = -1;	// Vertice fora do heap
	Q[0] = Q.back();
	map [ Q.back() ] = 0;
	Q.pop_back();

	minHeapify(Q, map, Q.size(), 0, d);

	return tmp;
}

void printTree (int v, vector < int >& pi, vector < int >& d) {
	for(int i=0;i<v;i++) {
		vector < int > path;
		cout << d[i] << " ";
		int pai = i;
		while(pai!=-1) {
			path.push_back(pai);
			pai = pi[pai];
		}
		if (d[i]!=999999999) {
			cout << path.size() << " ";

			for ( int t = path.size() -1; t>=0; t--) {
				cout << path[t] << " ";
			}
		}
		else {
			cout << "0";
		}
		cout << endl;
	}
}

void grafo::dijkstra (int s) {
	print_graph();
	vector < int > S, map(v);
	vector < int > Q, d, pi;

	// Mapeamento inicial do heap
	for(int i = 0; i<v; i++) {
		map[i] = i;
		Q.push_back(i);
	}

	initializeSingleSource(s,d,pi);

	printHeap(Q, d, map);
	cout << "FONTE >>>>>>> " << s << endl << endl;
	Q[0] = s;
	Q[s] = 0;
	map[0] = s;
	map[s] = 0;

	printHeap(Q, d, map);

	while(Q.size()>0) {
		int u = extractMin(Q, map,d);
		S.push_back(u);

		list < edge >::iterator ll;
		for (ll=aL[u].begin(); ll!=aL[u].end(); ll++) {
			relax(u, ll->n, ll->peso, d, pi, Q, map);
		}
	}

	printTree(v,pi,d);

}

void grafo::initializeSingleSource(int s, vector < int >& d, vector < int >& pi) {
	int max = 999999999; // infinito = 999999999

	d.resize(v,max);
	pi.resize(v,-1);

	d[s] = 0;		// s e a raiz
}



void grafo::add_edge (int no, int no2, int peso) {
	edge vx(no2,peso);
	aL[no].push_back(vx);
}

int grafo::make_graph () {
	int nA, tipo;
	cin >> v >> nA;

	aL.resize(v);
	for(int i=0;i<nA;i++) {
		int no, aresta, peso;
		cin >> no >> aresta >> peso;
		edge vx(aresta,peso);
		aL[no].push_back(vx);
	}
	cin >> tipo;
	return tipo;
}

void grafo::print_graph () {
	cout << endl << endl << "Imprimindo grafo... tam: " << v << endl;
	list < edge >::iterator ll;

	for(int i=0;i<v;i++) {
		cout << i;
		for (ll=aL[i].begin();ll!=aL[i].end();ll++) {
			cout << " -> " << (*ll).n << ":" << (*ll).peso;
		}
		cout << endl;
		cout << endl;
	}
	cout << endl;
}

int main () {
	grafo g;
	int s;

	s = g.make_graph();

	g.dijkstra(s);
	/*
	 */

}
