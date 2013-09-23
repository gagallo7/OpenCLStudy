#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#if !defined(CL_CALLBACK)
#define CL_CALLBACK
#endif



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
	cl_program programa;
	cl_kernel kernel;
	cl_mem Abuffer;
	cl_mem Bbuffer;
	cl_mem Sbuffer;
	cl_event evento;

	int* sem;

	// Constantes
	// Matrizes A, B e C
	// Tamanhos:
	// A: l x m
	// B: m x n
	// C: l x n --- C = A x B
	int a;

	if(argc > 1) 
		a = atoi(argv[1]);
	else
		a = 8;

	const unsigned int l = a;
	cl_uint A[l];
	cl_uint B[l];
	// Preenchendo as matrizes

	srand(time(NULL));

	for ( int x = 0; x < l ; x ++ ) {
		//A[x] = x + rand()%200;
		A[x] = x;
	}

	for(int x = 0; x < l; x++) {
		std::cout << A[x] << " ";
	}
	std::cout << std::endl;
	std::cout << std::endl;

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
	checkErr(
			(errNum != CL_SUCCESS) ? errNum :
			(nPlataformas <= 0 ? -1 : CL_SUCCESS),
			"clGetPlatformIDs");

	// Iterando na lista de plataformas até achar uma que suporta um dispositivo de CPU. Se isso não ocorrer, acusar erro.
	cl_uint i;
	for (i=0; i < nPlataformas; i++) {
		// Atribuindo o número de dispositivos de CPU a nDispositivos
		errNum = clGetDeviceIDs (	listaPlataformaID[i],
				CL_DEVICE_TYPE_CPU,
				0,
				NULL,
				&nDispositivos		);
		if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
			checkErr (errNum, "clGetDeviceIDs");
		}
		// Conferindo se há dispositivos de CPU
		else if (nDispositivos > 0) {
			// Atribuindo um dispositivo a uma listaDispositivoID
			listaDispositivoID = (cl_device_id *)alloca(sizeof(cl_device_id)*nDispositivos);

			errNum = clGetDeviceIDs (	
					listaPlataformaID[i],
					CL_DEVICE_TYPE_CPU,
					nDispositivos,
					&listaDispositivoID[0],
					NULL);
			checkErr(errNum, "clGetPlatformIDs");
			break;
		}
	}

	// Criando um contexto no dispositivo/plataforma selecionada
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
	std::ifstream srcFile("masterCore.cl");

	// Conferindo se ele foi aberto
	checkErr(srcFile.is_open() ? CL_SUCCESS : -1, "lendo mMatrix.cl");

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

	// Compilando programa
	errNum = clBuildProgram (
			programa,
			nDispositivos,
			listaDispositivoID,
			 "-g -s \"\"",
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
			"somatorio",
			&errNum);
	checkErr(errNum, "clCreateKernel");

	sem = (int *)malloc(sizeof(int));
	*sem = 0;
	// Alocando Buffers
	Abuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_uint)*l,
			A,
			&errNum);
	checkErr(errNum, "clCreateBuffer(A)");

	Bbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_uint)*l,
			A,
			&errNum);
	checkErr(errNum, "clCreateBuffer(B)");

	Sbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(int*),
			sem,
			&errNum);
	checkErr(errNum, "clCreateBuffer(sem)");
	// Escolhendo o primeiro dispositivo e criando a fila de comando
	fila = clCreateCommandQueue (
			contexto, 
			listaDispositivoID[0],
			CL_QUEUE_PROFILING_ENABLE,
			&errNum);
	checkErr(errNum, "clCreateCommandQueue");


	// Setando os argumentos da função do Kernel
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &Abuffer);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &Bbuffer);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &l);
	errNum |= clSetKernelArg(kernel, 3, sizeof(int*), &Sbuffer);
	checkErr(errNum, "clSetKernelArg");

	// Definindo o número de work-items globais e locais
	const size_t globalWorkSize[1] = { l };
	const size_t localWorkSize[1] = { 2 };

	// Enfileirando o Kernel para execução através da matriz
	errNum = clEnqueueNDRangeKernel (
			fila,
			kernel,
			1, // Uma dimensão,
			NULL,
			globalWorkSize,
			localWorkSize,
			0,
			NULL,
			&evento);
	checkErr(errNum, "clEnqueueNDRangeKernel");

	errNum = clEnqueueReadBuffer (
			fila,
			Bbuffer,
			CL_TRUE,
			0,
			sizeof(cl_uint)*l,
			B,
			0,
			NULL,
			NULL);
	checkErr(errNum, "clEnqueueReadBuffer");
	Bbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_uint)*l,
			B,
			&errNum);
	checkErr(errNum, "clCreateBuffer(B)");
	errNum = clSetKernelArg(kernel, 1, sizeof(cl_mem), &Bbuffer);
	checkErr(errNum, "clSetKernelArg");
	errNum = clEnqueueNDRangeKernel (
			fila,
			kernel,
			1, // Uma dimensão,
			NULL,
			globalWorkSize,
			localWorkSize,
			0,
			NULL,
			&evento);
	checkErr(errNum, "clEnqueueNDRangeKernel");

	// Contando tempo via built-in do OpenCL
	cl_ulong ev_start_time=(cl_ulong)0;     
	cl_ulong ev_end_time=(cl_ulong)0;

	clFinish(fila);
	errNum = clWaitForEvents(1, &evento);
	errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
	errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);

	double run_time_gpu = (double)(ev_end_time - ev_start_time)/1000; // in usec

	errNum = clEnqueueReadBuffer (
			fila,
			Bbuffer,
			CL_TRUE,
			0,
			sizeof(cl_uint)*l,
			B,
			0,
			NULL,
			NULL);
	checkErr(errNum, "clEnqueueReadBuffer");

	// Imprimindo saída do resultado
	for(int x = 0; x < l; x++) {
		std::cout << B[x] << " ";
	}
	std::cout << std::endl;


	int serial =0;
	for(int x = 0; x < l; x++) {
		serial += A[x];
	}

	std::cout << std::endl;
	std::cout << "Paralelo: " << B[0] << std::endl;
	std::cout << "Serial: " << serial << std::endl;


	std::cout << std::endl << std::fixed;
	std::cout << "Tempo de execução: " << std::setprecision(6) << run_time_gpu/1000000 << "ms";
	std::cout << std::endl;
	std::cout << run_time_gpu*1.0e-6;
	std::cout << std::endl;

	return 0;
}
