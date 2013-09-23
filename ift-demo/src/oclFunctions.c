#include <oclFunctions.h>
#define MAX_SOURCE_SIZE (0x100000)

void infoPlataforma (cl_platform_id * listaPlataformaID, cl_uint i) {
	cl_int err;
	size_t size;

	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_NAME, 0, NULL, &size);
	char * name = (char *)alloca(sizeof(char) * size);

	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_VENDOR, 0, NULL, &size);
	char * vname = (char *)alloca(sizeof(char) * size);

	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_NAME, size, name, NULL);
	err = clGetPlatformInfo(listaPlataformaID[i], CL_PLATFORM_VENDOR, size, vname, NULL);

/*
	std::cout << "Platform name: " << name << std::endl
		<< "Vendor name : " << vname << std::endl;
*/
    printf ( "Platform Name: %s\nVendor Name: %s\n", name, vname );
}

// Checagem de erros
inline void checkErr(cl_int err, const char *name) {
	if(err!=CL_SUCCESS) {
//		std::cerr << "ERRO: " << name << " (" << err << ")" << std::endl;
        printf ( "ERROR: %s (%d)\n", name, err);
		exit(EXIT_FAILURE);
	}
}


void CL_CALLBACK contextCallback (
		const char *errInfo,
		const void *private_info,
		size_t cb,
		void *user_data			) {
//	std::cout << "Ocorreu um erro durante o uso do context: " << errInfo << std::endl;
    printf ( "An error happenned during context use: %s\n", errInfo );
	// Ignorando limpeza de memória e saindo diretamente
	exit(1);
}

void prepareAllDataForDevice (  cl_int errNum,
                                cl_uint nPlataformas,
                                cl_uint nDispositivos,
                  //              cl_platform_id *listaPlataformaID,
                                cl_device_id *listaDispositivoID,
                                cl_context* pContexto,
                                cl_command_queue fila,
                                cl_program programa, cl_program programa2,
                                cl_kernel* pKernel, cl_kernel* pKernel2
                                ) {

    FILE* fp;
    char* source_str; 
    size_t source_size;
//    cl_device_id* listaDispositivoID;
    cl_platform_id* listaPlataformaID;
    cl_kernel kernel = *pKernel;
    cl_kernel kernel2 = *pKernel2;
    cl_context contexto = *pContexto;
    
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
	//std::cout << "#Plataformas: " << nPlataformas << std::endl;
    printf ( "#Platforms: %d\n", nPlataformas );
	checkErr(
			(errNum != CL_SUCCESS) ? errNum :
			(nPlataformas <= 0 ? -1 : CL_SUCCESS),
			"clGetPlatformIDs");


	// Iterando na lista de plataformas até achar uma que suporta um dispositivo de CPU. Se isso não ocorrer, acusar erro.
	cl_uint i;
	for (i=0; i < nPlataformas; i++) {
		// Atribuindo o número de dispositivos de GPU a nDispositivos
		errNum = clGetDeviceIDs (	listaPlataformaID[i],
								//CL_DEVICE_TYPE_ALL,
				CL_DEVICE_TYPE_CPU,
				//			CL_DEVICE_TYPE_GPU,
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
				//					CL_DEVICE_TYPE_ALL,
					CL_DEVICE_TYPE_CPU,
			//				CL_DEVICE_TYPE_GPU,
					nDispositivos,
					&listaDispositivoID[0],
					NULL);
			checkErr(errNum, "clGetPlatformIDs");
			break;
		}
	}

	// Crindo um contexto no dispositivo/plataforma selecionada
	//std::cout << "Adicionando dispositivos OpenCL de numero " << i << std::endl;
    printf ( " Adding OpenCL devices -- number %d\n", i );
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

/*
	// Carregando o arquivo-fonte cl para póstuma compilação feita em runtime
	std::ifstream srcFile("dijkstra.cl");

	// Conferindo se ele foi aberto
	checkErr(srcFile.is_open() ? CL_SUCCESS : -1, "lendo dijkstra.cl");

	std::string srcProg (
			std::istreambuf_iterator<char>(srcFile),
			(std::istreambuf_iterator<char>()));

	const char *fonte = srcProg.c_str();
	size_t tamanho = srcProg.length();
*/
    /* Load the source code containing the kernel*/
    fp = fopen("dijkstra.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    programa = clCreateProgramWithSource(contexto, 1, (const char **)&source_str,
(const size_t *)&source_size, &errNum);

	//free(source_str);
	// Criando programa da fonte
/*
	programa = clCreateProgramWithSource (
			contexto,
			1,
			&fonte,
			&tamanho,
			&errNum);
	checkErr(errNum, "clCreateProgramWithSource");

*/
    fp = fopen("kernel2.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    programa2 = clCreateProgramWithSource(contexto, 1, (const char **)&source_str,
(const size_t *)&source_size, &errNum);

/*
	// Criando programa da fonte do kernel2 TODO
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
*/
	/*
	*/

	// Compilando programa
    printf ( "Compiling the kernel1 ... \n" );
	errNum = clBuildProgram (
			programa,
			nDispositivos,
			listaDispositivoID,
			NULL,
			NULL,
			NULL);

    printf ( "Compiling the kernel2 ... \n" );
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

		//std::cerr << "Erro no kernel: " << std::endl;
		printf ( " Build error : %s\n", logCompilacao );

//		std::cerr << logCompilacao;
		checkErr(errNum, "clBuildProgram");
	}

	printf ( "KERNEL 1\n" );
	// Criando o objeto do Kernel
	kernel = clCreateKernel (
			programa,
			"dijkstra",
			&errNum);
	checkErr(errNum, "clCreateKernel1");

	printf ( "KERNEL 2\n" );
	// Criando o objeto do Kernel2
	kernel2 = clCreateKernel (
			programa2,
			"dijkstra2",
			&errNum);
	checkErr(errNum, "clCreateKernel2");
}
