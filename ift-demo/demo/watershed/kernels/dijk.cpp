#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <list>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#if !defined(CL_CALLBACK)
#define CL_CALLBACK
#endif

#define INF 1 << 30

using namespace std;

void log (vector < unsigned int > v, char *argv[]) {
	FILE* fp;
	char local[50] = "logs/";
	strcat(local, argv[2]);
	//cout << local << endl;
	fp = fopen(local,"w");

	for (int i=0; i<v.size(); i++) {
		fprintf(fp, "%d\n", v[i]);
	}

	fclose(fp);
}

bool vazio (vector < unsigned int > M) {
	for (int i = 0; i < M.size(); i++) {
		if (M[i] == true)
			return false;
	}
	return true;
}

class grafo {
	public:
		int numV; 				// Numero de vertices
		vector < cl_uint > vert; 			// Vetor de vertices
		vector < cl_uint > arestas; 		// Vetor de arestas
		vector < cl_uint > pesos; 			// Vetor de pesos

		void make_graph (FILE *);
		void make_graph9 (FILE *);
		void make_graph558 (FILE *, int* );
		void printInfo (void);
		void prepare (vector < cl_uint >&, vector < cl_uint >&, vector < cl_uint >&);
};

// Cria grafo no modelo da disciplina de MC558
// Primeira linha tem o numero de vertices seguido do numero total
// de arestas do grafo.
void grafo::make_graph558 ( FILE* fp, int* raiz ) {
	char linha[2000];
	vector < unsigned int > a_tmp, p_tmp, v_tmp;
	vector < list < vector < unsigned int > > > la;
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return;
	}
	// Primeiro vertice comeca sua lista de adjacencias em 0
	//vert.push_back(0);

	// Cada linha tem a representacao de cada vertice
	// fgets limita-se a capturar uma linha
	int i, o, peso;
	int numA;

	// Coletando a primeira linha, a qual armazena o numero
	// de vertices do grafo
	if (fgets (linha, 2000, fp) != NULL) {
		sscanf(linha, " %d %d", &numV, &numA);
	}

	la.resize(numV+1);

	//cout << numV << endl;

	int nLinha = 0;
	// Coletando as arestas
	while (fgets (linha, 2000, fp) != NULL) {
		//int ant = -1;
		// Capturando o vertice
		if (nLinha < numA) { // Se o vertice tiver arestas
			sscanf(linha," %d %d %d", &i, &o, &peso);
			
			// Caso de fim de linha, pois nao ha arestas iguais
			// Logo o destino nao pode ser o mesmo
	//		cout << i << " " << o << " " << peso;
		//	ant = i;
			vector < unsigned int > novo;
			novo.push_back(o);
			novo.push_back(peso);
			la[i].push_back(novo);
		}
		else {
			sscanf(linha," %d", raiz);
			break;
		}
		nLinha++;
		/*
		*/
		// Apontando para a proxima lista de adjacencias
		//vert.push_back(arestas.size());
	}
	fclose(fp);

	list < vector < unsigned int > >::iterator ll;

	int j = 0;
	for (int i = 0; i < numV; i++) {
		vert.push_back(j);
		for (ll = la[i].begin(); ll != la[i].end(); ll++) {
			j++;
			arestas.push_back((*ll)[0]);
			pesos.push_back((*ll)[1]);
		}
	}	
	vert.push_back(j); // Arestas do ultimo vertice

}


// Cria grafo baseado no modelo do DIMACS9
// DIMACS usa o seguinte modelo:
// Cada linha tem o vertice de origem e saida e também o peso de
// cada aresta
//
void grafo::make_graph9 ( FILE* fp ) {
	char linha[2000];
	vector < unsigned int > a_tmp, p_tmp, v_tmp;
	vector < list < vector < unsigned int > > > la;
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return;
	}
	// Primeiro vertice comeca sua lista de adjacencias em 0
	//vert.push_back(0);

	// Cada linha tem a representacao de cada vertice
	// fgets limita-se a capturar uma linha
	int i, o, peso;

	// Coletando a primeira linha, a qual armazena o numero
	// de vertices do grafo
	if (fgets (linha, 2000, fp) != NULL) {
		sscanf(linha, " %d", &numV);
	}

	la.resize(numV+1);

	cout << numV << endl;

	// Coletando as arestas
	while (fgets (linha, 2000, fp) != NULL) {
		//int ant = -1;
		// Capturando o vertice
		if (*linha != '-') { // Se o vertice tiver arestas
			sscanf(linha," %d %d %d", &i, &o, &peso);
			
			// Caso de fim de linha, pois nao ha arestas iguais
			// Logo o destino nao pode ser o mesmo
			cout << i << " " << o << " " << peso;
		//	ant = i;
			vector < unsigned int > novo;
			novo.push_back(o);
			novo.push_back(peso);
			la[i].push_back(novo);
		}
		/*
		*/
		// Apontando para a proxima lista de adjacencias
		//vert.push_back(arestas.size());
	}
	fclose(fp);

	list < vector < unsigned int > >::iterator ll;

	int j = 0;
	for (int i = 0; i < numV; i++) {
		vert.push_back(j);
		for (ll = la[i].begin(); ll != la[i].end(); ll++) {
			j++;
			arestas.push_back((*ll)[0]);
			pesos.push_back((*ll)[1]);
		}
	}	
	vert.push_back(j); // Arestas do ultimo vertice

}

// Cria grafo baseado no modelo do DIMACS
// DIMACS usa o seguinte modelo:
// Cada linha tem o vertice de origem e o resto da linha sao
// as arestas do mesmo vertice
void grafo::make_graph(FILE *fp) {
	char linha[2000];
	if (fp==NULL) {
		cout << "Erro ao abrir arquivo!" << endl;
		return;
	}
	// Primeiro vertice comeca sua lista de adjacencias em 0
	vert.push_back(0);

	// Cada linha tem a representacao de cada vertice
	// fgets limita-se a capturar uma linha
	int i, peso;
	while (fgets (linha, 2000, fp) != NULL) {
		int ant = -1;
		// Capturando o vertice
		if (*linha != '\n') // Se o vertice tiver arestas
			while(1) {
				sscanf(linha," %d %d %[0-9 ]", &i, &peso,  linha);
				// Caso de fim de linha, pois nao ha arestas iguais
				// Logo o destino nao pode ser o mesmo
				if (ant == i) {
					break;
				}
				cout << i << " ";
				ant = i;
				arestas.push_back(i);
				pesos.push_back(peso);
			}
		/*
		*/
		// Apontando para a proxima lista de adjacencias
		vert.push_back(arestas.size());
	}
	fclose(fp);
	numV = vert.size() - 1; // A representacao faz com que tenha mais um elemento no vetor de vertices
}

void grafo::printInfo () {
	cout << "Vetor de vertices:" << endl;
	vector < cl_uint >::iterator it, it2;
	for (it=vert.begin();it!=vert.end();it++) {
		cout << *it << " ";
	}
	cout << endl;

	cout << "Vetor de arestas:" << endl;
	it2 = pesos.begin();
	for (it=arestas.begin();it!=arestas.end();it++) {
		cout << *it << ":" << *it2 << " ";
		it2++;
	}
	cout << endl;
}

// Retorna a proxima potencia de 2 baseado numa entrada
cl_uint nextPow2 (int n) {
	cl_uint p = 2;
	while (p < n && 2*p) p *= 2;
	return p;
}

void prepare (vector < cl_uint >& M, vector < cl_uint >& C, vector < cl_uint >& U,
		int numV, int s) {
	cl_uint inf = 1 << 30; // Infinito
	M.resize(numV,0);
	C.resize(numV, inf);
	U.resize(numV, inf);
	M[s] = true;
	C[s] = 0;
	U[s] = 0;
}

void infoPlataforma (cl_platform_id * listaPlataformaID, cl_uint i) {
	cl_int err;
	size_t size;
	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_NAME, 0, NULL, &size);
	char * name = (char *)alloca(sizeof(char) * size);
	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_NAME, size, name, NULL);
	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_VENDOR, 0, NULL, &size);
	char * vname = (char *)alloca(sizeof(char) * size);
	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_VENDOR, size, vname, NULL);
	std::cout << "Platform name: " << name << std::endl
		<< "Vendor name : " << vname << std::endl;
}

// Checagem de erros
inline void checkErr(cl_int err, const char *name) {
	if(err!=CL_SUCCESS) {
		std::cerr << "ERRO: " << name << " (" << err << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void CL_CALLBACK contextCallback (
		const char *errInfo,
		const void *private_info,
		size_t cb,
		void *user_data			) {
	std::cout << "Ocorreu um erro durante o uso do context: " << errInfo << std::endl;
	// Ignorando limpeza de memória e saindo diretamente
	exit(1);
}

int main(int argc, char *argv[]) {
	cl_int errNum;
	cl_uint nPlataformas;
	cl_uint nDispositivos;
	cl_platform_id *listaPlataformaID;
	cl_device_id *listaDispositivoID;
	cl_context contexto = NULL;
	cl_command_queue fila;
	cl_program programa, programa2;
	cl_kernel kernel, kernel2;
	cl_mem Vbuffer;
	cl_mem Abuffer;
	cl_mem Pbuffer;
	cl_mem Mbuffer;
	cl_mem Cbuffer;
	cl_mem Ubuffer;
	cl_mem SEMbuffer;
	cl_mem numVbuffer;
	cl_event evento;
	int raiz;

	cout << "Abrindo arquivo... " << endl;
	// Abrindo arquivo de entrada
	FILE *fp = fopen(argv[1], "r");
	if (argc > 2)
		raiz = atoi(argv[2]);
	grafo g;
	// Criando vetores auxiliares
	vector < cl_uint > C, U, M;
	// Criando Grafo
	cout << "Montando grafo... " << endl;
	//g.make_graph9(fp);
	g.make_graph558(fp, &raiz);
	// Preparando vetores auxiliares para o Dijkstra
	cout << endl << "Preparando... raiz em: " << raiz << endl;
	prepare(M, C, U, g.numV, raiz);
	//g.printInfo();
	cout << "Vetores prontos. " <<  endl;
	cout << "Numero de workitems: " << nextPow2(g.arestas.size()) << endl;

	///// Selecionando uma plataforma OpenCL para rodar

	// Atribuindo a nPlataformas o número de plataformas disponíveis
	errNum = clGetPlatformIDs(0, NULL, &nPlataformas);
	checkErr( (errNum != CL_SUCCESS) ? errNum : 
			(nPlataformas <= 0 ? -1 : CL_SUCCESS),
			"clGetPlataformsIDs");
	// Se não houve erro, alocar memória para cl_platform_id
	listaPlataformaID = (cl_platform_id *)alloca(sizeof(cl_platform_id)*nPlataformas);
	// Atribuindo uma plataforma ao listaPlataformaID
	errNum = clGetPlatformIDs(nPlataformas, listaPlataformaID, NULL);
	std::cout << "#Plataformas: " << nPlataformas << std::endl;
	checkErr(
			(errNum != CL_SUCCESS) ? errNum :
			(nPlataformas <= 0 ? -1 : CL_SUCCESS),
			"clGetPlatformIDs");


	// Iterando na lista de plataformas até achar uma que suporta um dispositivo de CPU. Se isso não ocorrer, acusar erro.
	cl_uint i;
	for (i=0; i < nPlataformas; i++) {
		// Atribuindo o número de dispositivos de GPU a nDispositivos
		errNum = clGetDeviceIDs (	listaPlataformaID[i],
				//				CL_DEVICE_TYPE_ALL,
				//CL_DEVICE_TYPE_CPU,
							CL_DEVICE_TYPE_GPU,
				0,
				NULL,
				&nDispositivos		);
		if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
			infoPlataforma(listaPlataformaID, i);
			checkErr (errNum, "clGetDeviceIDs");
		}
		// Conferindo se há dispositivos de CPU
		else if (nDispositivos > 0) {
			// Atribuindo um dispositivo a uma listaDispositivoID
			listaDispositivoID = (cl_device_id *)alloca(sizeof(cl_device_id)*nDispositivos);

			errNum = clGetDeviceIDs (	
					listaPlataformaID[i],
					//				CL_DEVICE_TYPE_ALL,
			//		CL_DEVICE_TYPE_CPU,
							CL_DEVICE_TYPE_GPU,
					nDispositivos,
					&listaDispositivoID[0],
					NULL);
			checkErr(errNum, "clGetPlatformIDs");
			break;
		}
	}

	// Crindo um contexto no dispositivo/plataforma selecionada
	std::cout << "Adicionando dispositivos OpenCL de numero " << i << std::endl;
	infoPlataforma(listaPlataformaID, i);
	cl_context_properties propContexto[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)listaPlataformaID[i],
		0
	};

	contexto = clCreateContext(
			propContexto,
			nDispositivos,
			listaDispositivoID,
			&contextCallback,
			NULL,
			&errNum			);

	checkErr(errNum, "clCreateContext");

	// Carregando o arquivo-fonte cl para póstuma compilação feita em runtime
	std::ifstream srcFile("dijkstra.cl");

	// Conferindo se ele foi aberto
	checkErr(srcFile.is_open() ? CL_SUCCESS : -1, "lendo dijkstra.cl");

	std::string srcProg (
			std::istreambuf_iterator<char>(srcFile),
			(std::istreambuf_iterator<char>()));

	const char *fonte = srcProg.c_str();
	size_t tamanho = srcProg.length();

	// Criando programa da fonte
	programa = clCreateProgramWithSource (
			contexto,
			1,
			&fonte,
			&tamanho,
			&errNum);
	checkErr(errNum, "clCreateProgramWithSource");

	// Criando programa da fonte do kernel2
	// Carregando o arquivo-fonte cl para póstuma compilação feita em runtime
	std::ifstream srcFile2("kernel2.cl");

	// Conferindo se ele foi aberto
	checkErr(srcFile2.is_open() ? CL_SUCCESS : -1, "lendo kernel2.cl");

	std::string srcProg2 (
			std::istreambuf_iterator<char>(srcFile2),
			(std::istreambuf_iterator<char>()));

	const char *fonte2 = srcProg2.c_str();
	tamanho = srcProg2.length();
	programa2 = clCreateProgramWithSource (
			contexto,
			1,
			&fonte2,
			&tamanho,
			&errNum);
	checkErr(errNum, "clCreateProgramWithSource");
	/*
	*/

	// Compilando programa
	errNum = clBuildProgram (
			programa,
			nDispositivos,
			listaDispositivoID,
			NULL,
			NULL,
			NULL);

	errNum = clBuildProgram (
			programa2,
			nDispositivos,
			listaDispositivoID,
			NULL,
			NULL,
			NULL);

	if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
		// Determinando o motivo do erro
		char logCompilacao[16384];
		clGetProgramBuildInfo (
				programa,
				listaDispositivoID[0],
				CL_PROGRAM_BUILD_LOG,
				sizeof(logCompilacao),
				logCompilacao,
				NULL);

		std::cerr << "Erro no kernel: " << std::endl;

		std::cerr << logCompilacao;
		checkErr(errNum, "clBuildProgram");
	}

	// Criando o objeto do Kernel
	kernel = clCreateKernel (
			programa,
			"dijkstra",
			&errNum);
	checkErr(errNum, "clCreateKernel");


	// Criando o objeto do Kernel2
	kernel2 = clCreateKernel (
			programa2,
			"dijkstra2",
			&errNum);
	checkErr(errNum, "clCreateKernel2");

	// Alocando Buffers
	Vbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * g.vert.size(),
			g.vert.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(V)");

	Abuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * g.arestas.size(),
			g.arestas.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(A)");

	Pbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * g.pesos.size(),
			g.pesos.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(P)");

	Ubuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * U.size(),
			U.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(U)");

	Mbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * M.size(),
			M.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(M)");

	Cbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * C.size(),
			C.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(C)");

	int semaforo = 0;
	SEMbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int),
			&semaforo,
			&errNum);
	checkErr(errNum, "clCreateBuffer(semaforo)");

	int numV = g.vert.size();
	numVbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int),
			&numV,
			&errNum);
	checkErr(errNum, "clCreateBuffer(numero_de_vertices)");

	/*
	   Bbuffer = clCreateBuffer (
	   contexto,
	   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	   sizeof(cl_uint)*m*n,
	   (B),
	   &errNum);
	   checkErr(errNum, "clCreateBuffer(B)");

	   Cbuffer = clCreateBuffer (
	   contexto,
	   CL_MEM_WRITE_ONLY,
	   sizeof(cl_uint)*l*n,
	   NULL,
	   &errNum);
	   checkErr(errNum, "clCreateBuffer(C)");
	   */

	// Escolhendo o primeiro dispositivo e criando a fila de comando
	fila = clCreateCommandQueue (
			contexto, 
			listaDispositivoID[0],
			CL_QUEUE_PROFILING_ENABLE,
			&errNum);
	checkErr(errNum, "clCreateCommandQueue");

	// Setando os argumentos da função do Kernel
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &Vbuffer);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &Abuffer);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &Pbuffer);
	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &Mbuffer);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &Cbuffer);
	errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &Ubuffer);
	errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &SEMbuffer);
	errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &numVbuffer);
	/*
	*/
	// Setando os argumentos da função do Kernel2
	errNum = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &Vbuffer);
	errNum |= clSetKernelArg(kernel2, 1, sizeof(cl_mem), &Abuffer);
	errNum |= clSetKernelArg(kernel2, 2, sizeof(cl_mem), &Pbuffer);
	errNum |= clSetKernelArg(kernel2, 3, sizeof(cl_mem), &Mbuffer);
	errNum |= clSetKernelArg(kernel2, 4, sizeof(cl_mem), &Cbuffer);
	errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &Ubuffer);
	errNum |= clSetKernelArg(kernel2, 6, sizeof(cl_mem), &SEMbuffer);
	errNum |= clSetKernelArg(kernel2, 7, sizeof(cl_mem), &numVbuffer);
	checkErr(errNum, "clSetKernelArg");

	// Definindo o número de work-items globais e locais
	const size_t globalWorkSize[1] = { g.numV };
	const size_t localWorkSize[1] = { 1 };

	// releituraFeita e um evento que sincroniza a atualizacao feita
	// por cada chamada aos kernels
	
	cl_event releituraFeita;
	errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, sizeof(int) * M.size(), M.data(), 0, NULL, &releituraFeita);
        checkErr(errNum, CL_SUCCESS);
        clWaitForEvents(1, &releituraFeita);

	while(!vazio(M)) {

/*
		std::cout << endl << "Mask: " << std::endl;
		for(int i=0; i<M.size(); i++) {
			cout << M[i] << " ";
		}
*/
		// Enfileirando o Kernel para execução através da matriz
		errNum = clEnqueueNDRangeKernel (
				fila,
				kernel,
				1,
				NULL,
				globalWorkSize,
				localWorkSize,
				0,
				NULL,
				&evento);
		checkErr(errNum, "clEnqueueNDRangeKernel");


		// Enfileirando o Kernel2 para execução através da matriz
		errNum = clEnqueueNDRangeKernel (
				fila,
				kernel2,
				1,
				NULL,
				globalWorkSize,
				localWorkSize,
				0,
				NULL,
				&evento);
		checkErr(errNum, "clEnqueueNDRangeKernel2");

		errNum  = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, sizeof(int) * M.size(), M.data(), 0, NULL, &releituraFeita);
		checkErr(errNum, CL_SUCCESS);
		clWaitForEvents(1, &releituraFeita);

	}

	cl_ulong ev_start_time=(cl_ulong)0;     
	cl_ulong ev_end_time=(cl_ulong)0;

	clFinish(fila);
	errNum = clWaitForEvents(1, &evento);
	errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
	errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);

	double run_time_gpu = (double)(ev_end_time - ev_start_time); // in usec

	errNum = clEnqueueReadBuffer (
			fila,
			Mbuffer,
			CL_TRUE,
			0,
			sizeof(cl_uint) * M.size(),
			M.data(),
			0,
			NULL,
			NULL);
	checkErr(errNum, "clEnqueueReadBuffer");

	/*
	   errNum = clEnqueueReadBuffer (
	   fila,
	   Vbuffer,
	   CL_TRUE,
	   0,
	   sizeof(cl_uint) * g.vert.size(),
	   g.vert.data(),
	   0,
	   NULL,
	   NULL);
	   checkErr(errNum, "clEnqueueReadBuffer");

	   errNum = clEnqueueReadBuffer (
	   fila,
	   Abuffer,
	   CL_TRUE,
	   0,
	   sizeof(cl_uint) * g.arestas.size(),
	   g.arestas.data(),
	   0,
	   NULL,
	   NULL);
	   checkErr(errNum, "clEnqueueReadBuffer");

	   errNum = clEnqueueReadBuffer (
	   fila,
	   Pbuffer,
	   CL_TRUE,
	   0,
	   sizeof(cl_uint) * g.pesos.size(),
	   g.pesos.data(),
	   0,
	   NULL,
	   NULL);
	   checkErr(errNum, "clEnqueueReadBuffer");
	   */

	errNum = clEnqueueReadBuffer (
			fila,
			Cbuffer,
			CL_TRUE,
			0,
			sizeof(cl_uint) * C.size(),
			C.data(),
			0,
			NULL,
			NULL);
	checkErr(errNum, "clEnqueueReadBuffer");

	errNum = clEnqueueReadBuffer (
			fila,
			Ubuffer,
			CL_TRUE,
			0,
			sizeof(cl_uint) * U.size(),
			U.data(),
			0,
			NULL,
			NULL);
	checkErr(errNum, "clEnqueueReadBuffer");
/*
	std::cout << endl << "Vertices: " << std::endl;
	for(int i=0; i<g.vert.size(); i++) {
		cout << g.vert[i] << " ";
	}

	std::cout << endl << "Arestas: " << std::endl;
	for(int i=0; i<g.arestas.size(); i++) {
		cout << g.arestas[i] << " ";
	}

	std::cout << endl << "Mask: " << std::endl;
	for(int i=0; i<g.vert.size() - 1; i++) {
		cout << M[i] << " ";
	}

	std::cout << endl << "Pesos: " << std::endl;
	for(int i=0; i<g.arestas.size(); i++) {
		cout << g.pesos[i] << " ";
	}
	std::cout << std::endl;


	cout << setw(6) << setfill('0');
	std::cout << "Update: " << std::endl;
	for(int i=0; i<g.numV; i++) {
		if (i%8==0) cout << std::endl;
		if (U[i] < INF) {
			cout << setw(6) << setfill(' ') << i << ":" << U[i] << "  ";
		}
		else {
			cout << setw(6) << setfill(' ') << i << ":" << "INF   " << "  ";
		}
	}
	std::cout << std::endl;

	std::cout << "Cost: " << std::endl;
	for(int i=0; i<g.numV; i++) {
		if (i%8==0) cout << std::endl;
		if (C[i] < INF) {
			cout <<  setw(6) << setfill(' ')<< i << ":" << C[i] << "  ";
		}
		else {
			cout << setw(6) << setfill(' ') << i << ":" << "INF   " << "  ";
		}
	}
	std::cout << std::endl;
*/

	std::cout << std::endl << std::fixed;
	std::cout << "Tempo de execução: " << std::setprecision(6) << run_time_gpu/1000000 << " ms";
	std::cout << std::endl;
	std::cout << run_time_gpu*1.0e-6;
	std::cout << std::endl;

	log(U, argv);

	return 0;
}
