/*
   Copyright (C) <2010> <Alexandre Xavier FalcÃ£o and Thiago Vallin Spina>

   This file is part of IFT-demo.

   IFT-demo is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   IFT-demo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with IFT-demo.  If not, see <http://www.gnu.org/licenses/>.

   please see full copyright in COPYING file.
   -------------------------------------------------------------------------

   written by A.X. Falcão <afalcao@ic.unicamp.br> and by T.V. Spina
   <tvspina@liv.ic.unicamp.br>, 2010

*/

#include "ift.h"
#define DEVICE CL_DEVICE_TYPE_GPU
#define NLOOP 1

/* Paper related to this program:

   @article{Falcao02,
   author     = "A.X. Falc{\~{a}}o and L.F. Costa and B.S. da Cunha",
   title      = "Multiscale skeletons by image foresting transform and its applications to neuromorphometry",
   journal    = "Pattern Recognition",
   publisher  = "Elsevier ",
   pages      = "1571-1582",
   volume     = "35",
   number     = "7",
   month      = "Apr",
   year       = "2002"
   }

*/

// Creates Empty Forest

typedef struct _forest {
    Image *P; // predecessor map
    Image *R; // root map
    Image *V; // distance (cost or connectivity) map
} Forest;

Forest *CreateForest(int ncols, int nrows)
{
    Forest *F=(Forest *)calloc(1,sizeof(Forest));

    F->P = CreateImage(ncols,nrows);
    F->R = CreateImage(ncols,nrows);
    F->V = CreateImage(ncols,nrows);

    return(F);
}

// Destroys Forest

void DestroyForest(Forest **F)
{
    Forest *tmp=*F;

    if (tmp != NULL) {
        DestroyImage(&(tmp->P));
        DestroyImage(&(tmp->R));
        DestroyImage(&(tmp->V));
        free(tmp);
        *F = NULL;
    }
}

bool vazio (cl_int Mask[], cl_int n) {
    cl_int i;
    for (i = 0; i < n; i++) {
        if (Mask[i] == true)
            return false;
    }
    return true;
}

void CL_CALLBACK contextCallback (
        const char *errInfo,
        const void *private_info,
        size_t cb,
        void *user_data			) {
    printf ( "An error happenned during context use: %s\n", errInfo );
    exit(1);
}

// Euclidean distance transform

Forest *DistTrans(Image *I)
{
    /*--------------------------------------------------------*/
    /* OpenCL variables --------------------------------------*/
    timer    *tS1=NULL,*tS2=NULL;
    FILE* fp;
    char* source_str; 
    size_t source_size;
    cl_int errNum;
    cl_uint nPlataformas;
    cl_uint nDispositivos;
    cl_platform_id *listaPlataformaID;
    cl_device_id *listaDispositivoID;
    cl_context contexto = NULL;
    cl_command_queue fila;
    cl_program programa0, programa, programa2, programa3;
    cl_kernel kernel0, kernel, kernel2, kernel3;
    cl_event evento, evento0, evento1, evento2, evento3;

    cl_mem  costBuffer, costvalBuffer, costtbrowBuffer,
            numVbuffer, SEMbuffer, extraBuffer,
            Abuffer, Adxbuffer, Adybuffer,
            imgtbrowBuffer, labelvalBuffer, labeltbrowBuffer,
            Mbuffer, MvalBuffer, MtbrowBuffer,
            /*
            Ubuffer, Ulabelbuffer,
            Cbuffer, Clabelbuffer,
            UPredbuffer, CPredbuffer,
            */
            IrBuffer, IcBuffer, IvalBuffer, FVvalBuffer,
            FRvalBuffer, FPvalBuffer,
            cacheBuffer;


    /*--------------------------------------------------------*/
    int p,q,n=I->ncols*I->nrows,i,tmp;
    Pixel u,v,w;
    AdjRel *A=Circular(1.5),*A4=Circular(1.0);
    Forest *F=CreateForest(I->ncols,I->nrows);
    //GQueue *Q=CreateGQueue(1024,n,F->V->val);


    /* Preparing device for OpenCL program */
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

    cl_uint j;
    for (j=0; j < nPlataformas; j++) {
        // Atribuindo o número de dispositivos de GPU a nDispositivos
        errNum = clGetDeviceIDs (	listaPlataformaID[j],
                //								CL_DEVICE_TYPE_ALL,
                DEVICE,
                0,
                NULL,
                &nDispositivos		);
        if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
            infoPlataforma(listaPlataformaID, j);
            checkErr (errNum, "clGetDeviceIDs");
        }
        // Conferindo se há dispositivos de CPU
        else if (nDispositivos > 0) {
            // Atribuindo um dispositivo a uma listaDispositivoID
            listaDispositivoID = (cl_device_id *)alloca(sizeof(cl_device_id)*nDispositivos);

            errNum = clGetDeviceIDs (	
                    listaPlataformaID[j],
                    //									CL_DEVICE_TYPE_ALL,
                    DEVICE,
                    nDispositivos,
                    &listaDispositivoID[0],
                    NULL);
            checkErr(errNum, "clGetPlatformIDs");
            break;
        }
    }

    // Crindo um contexto no dispositivo/plataforma selecionada
    //std::cout << "Adicionando dispositivos OpenCL de numero " << i << std::endl;
    printf ( " Adding OpenCL devices -- number %d\n", j );
    infoPlataforma(listaPlataformaID, j);
    cl_context_properties propContexto[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)listaPlataformaID[j],
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

    fp = fopen("init.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    programa0 = clCreateProgramWithSource(
            contexto, 
            1, 
            (const char **)&source_str,
            (const size_t *)&source_size, 
            &errNum);
    checkErr(errNum, "clCreateProgramWithSource 0");

    fp = fopen("dijkstra.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    //    printf ( "Source code of kernel 1:\n%s\n\n", source_str );

    programa = clCreateProgramWithSource(
            contexto, 
            1, 
            (const char **)&source_str,
            (const size_t *)&source_size, 
            &errNum);
    checkErr(errNum, "clCreateProgramWithSource");

    /*
    fp = fopen("kernel2.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel2.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    programa2 = clCreateProgramWithSource(contexto, 1, (const char **)&source_str,
            (const size_t *)&source_size, &errNum);

    fp = fopen("kernel3.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel3.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    programa3 = clCreateProgramWithSource(contexto, 1, (const char **)&source_str,
            (const size_t *)&source_size, &errNum);
*/
    printf ( "Compiling the kernel 0 ... \r" );
    errNum = clBuildProgram (
            programa0,
            nDispositivos,
            listaDispositivoID,
            "-cl-fast-relaxed-math -cl-mad-enable",
            NULL,
            NULL);

    printf ( "kernel0 compiled!          \n" );
    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        // Determinando o motivo do erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa0,
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

    printf ( "Compiling the kernel 1 ... \r" );
    errNum = clBuildProgram (
            programa,
            nDispositivos,
            listaDispositivoID,
            "-cl-fast-relaxed-math -cl-mad-enable",
            NULL,
            NULL);

    printf ( "kernel1 compiled!         \n" );

    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa,
                listaDispositivoID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);

        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }
    /*
    printf ( "Compiling the kernel2 ... \n" );
    errNum = clBuildProgram (
            programa2,
            nDispositivos,
            listaDispositivoID,
            NULL,
            NULL,
            NULL);
    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
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


    printf ( "Compiling the kernel 3 ... \n" );
    errNum = clBuildProgram (
            programa3,
            nDispositivos,
            listaDispositivoID,
            NULL,
            NULL,
            NULL);

    printf ( "kernel3 compiled! \n" );

    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa,
                listaDispositivoID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);
        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }
*/
    printf ( "KERNEL 0\n" );
    // Criando o objeto do Kernel
    kernel0 = clCreateKernel (
            programa0,
            "initCache",
            &errNum);
    checkErr(errNum, "clCreateKernel0");

    printf ( "KERNEL 1\n" );
    // Criando o objeto do Kernel
    kernel = clCreateKernel (
            programa,
            "dijkstra",
            &errNum);
    checkErr(errNum, "clCreateKernel1");

    /*
    printf ( "KERNEL 2\n" );
    // Criando o objeto do Kernel2
    kernel2 = clCreateKernel (
            programa2,
            "dijkstra2",
            &errNum);
    checkErr(errNum, "clCreateKernel2");

    printf ( "KERNEL 3\n" );
    // Criando o objeto do Kernel3
    kernel3 = clCreateKernel (
            programa3,
            "dijkstra3",
            &errNum);
    checkErr(errNum, "clCreateKernel3");
*/
    // Calculating the next 2 power of n
    int N = n;
    int t = 0;
    while ( N > 0 ) {
        N >>= 1;
        t++;
    }

    N = 1 << t;

    printf ("n=%d ... N=%d (2^%d)##\n", n, N, t);


    printf ("-- ");
    cl_int* Mask = (cl_int *) malloc (N * sizeof ( cl_int ) );
    /*
    cl_int* CostCost = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* UpdateCost = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* Clabel = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* Ulabel = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* UpdatePred = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* CostPred = (cl_int *) malloc (n * sizeof ( cl_int ) );
    */
    static volatile cl_int semaforo = 0;
    cl_int extra = 0;
    cl_int8* cache = (cl_int8 *) malloc ( sizeof (cl_int8) * N );

    memset (Mask, 0, N*4);


    // Trivial path initialization

    for (p=0; p < n; p++) {
        u.x = p % I->ncols;
        u.y = p / I->ncols;
        F->V->val[p]=INT_MAX; F->R->val[p]=p; F->P->val[p]=NIL;
        if (I->val[p]!=0){ // p belongs to an objects border
            F->V->val[p]=0;
    //        InsertGQueue(&Q,p);
            Mask [ p ] = true;
        }
    }

    cacheBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
            sizeof(cl_int8) * N,
            cache,
            &errNum);
    checkErr(errNum, "clCreateBuffer(cache)");

    printf ( "\n\n\ncl_int8*: %d cl_int8: %d\n\n", sizeof(cl_int8*), sizeof(cl_int8) );

    IrBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof ( int ),
            &(I->nrows),
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    IcBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof ( int ),
            &(I->ncols),
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    imgtbrowBuffer= clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            I->nrows * ( sizeof ( cl_int ) ),
            I->tbrow,
            &errNum);
    checkErr(errNum, "clCreateBuffer(img)");

    IvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            I->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    FVvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            F->V->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(FVvalBuffer)");

    FRvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            F->R->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(FRvalBuffer)");

    FPvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            F->P->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(FPvalBuffer)");

    Mbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            N * (sizeof(cl_int)),
            Mask,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_buffer)");

    Abuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof( cl_int * ),
            &(A->n),
            &errNum);
    checkErr(errNum, "clCreateBuffer(A)");

    Adxbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            A->n * sizeof( cl_int ),
            A->dx,
            &errNum);
    checkErr(errNum, "clCreateBuffer(A)");

    Adybuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            A->n * sizeof( cl_int ),
            A->dy,
            &errNum);
    checkErr(errNum, "clCreateBuffer(A)");

    SEMbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_int),
            (void *)&semaforo,
            &errNum);
    checkErr(errNum, "clCreateBuffer(semaforo)");

    extraBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_int),
            &extra,
            &errNum);
    checkErr(errNum, "clCreateBuffer(extra)");

    fila = clCreateCommandQueue (
            contexto, 
            listaDispositivoID[0],
            CL_QUEUE_PROFILING_ENABLE,
            &errNum);
    checkErr(errNum, "clCreateCommandQueue");

    // Setando os argumentos da função do Kernel

    errNum = clSetKernelArg(kernel0, 0, sizeof(cl_mem), &IrBuffer);
    errNum = clSetKernelArg(kernel0, 1, sizeof(cl_mem), &IcBuffer);
    errNum |= clSetKernelArg(kernel0, 2, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel0, 3, sizeof(cl_mem), &imgtbrowBuffer);
    errNum |= clSetKernelArg(kernel0, 4, sizeof(cl_mem), &Abuffer);
    errNum |= clSetKernelArg(kernel0, 5, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel0, 6, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel0, 7, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel0, 8, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel0, 9, sizeof(cl_mem), &cacheBuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 0");

    // Kernel 1: Dijkstra kernel
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &IcBuffer);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &FRvalBuffer);
    errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &FPvalBuffer);
    errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &FVvalBuffer);
    //errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &cacheBuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 1");

    /*
    // Setando os argumentos da função do Kernel2
    errNum = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel2, 1, sizeof(cl_mem), &Cbuffer);
    errNum |= clSetKernelArg(kernel2, 2, sizeof(cl_mem), &Ubuffer);
    errNum |= clSetKernelArg(kernel2, 3, sizeof(cl_mem), &Clabelbuffer);
    errNum |= clSetKernelArg(kernel2, 4, sizeof(cl_mem), &Ulabelbuffer);
    errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &UPredbuffer);
    errNum |= clSetKernelArg(kernel2, 6, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel2, 7, sizeof(cl_mem), &SEMbuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 2");

    // Setando os argumentos da função do Kernel3
    errNum = clSetKernelArg(kernel3, 0, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel3, 1, sizeof(cl_mem), &Cbuffer);
    errNum |= clSetKernelArg(kernel3, 2, sizeof(cl_mem), &Ubuffer);
    errNum |= clSetKernelArg(kernel3, 3, sizeof(cl_mem), &Clabelbuffer);
    errNum |= clSetKernelArg(kernel3, 4, sizeof(cl_mem), &Ulabelbuffer);
    errNum |= clSetKernelArg(kernel3, 5, sizeof(cl_mem), &UPredbuffer);
    errNum |= clSetKernelArg(kernel3, 6, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel3, 7, sizeof(cl_mem), &SEMbuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 3");
    */

    // Definindo o número de work-items globais e locais
    const size_t globalWorkSize[1] = { N };
    const size_t localWorkSize[1] = { 64 };
    const size_t globalWorkSize2[2] = { N , 8 };
    const size_t localWorkSize2[2] = { 256, 1 };

    cl_int vez = 0;
    double run_time_gpu = 0;
    double run_time_k0 = 0;
    double run_time_k1 = 0;
    double run_time_k2 = 0;
    cl_ulong ev_start_time=(cl_ulong)0;     
    cl_ulong ev_end_time=(cl_ulong)0;
    gettimeofday(tS1, NULL);
    // releituraFeita e um evento que sincroniza a atualizacao feita
    // por cada chamada aos kernels

    cl_event releituraFeita;
    errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
            sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
    checkErr(errNum, CL_SUCCESS);
    clWaitForEvents(1, &releituraFeita);

    printf ( "Caching neighboorhood..." );
    errNum = clEnqueueNDRangeKernel (
            fila,
            kernel0,
            1,
            NULL,
            globalWorkSize,
            localWorkSize,
            0,
            NULL,
            &evento0);
    checkErr(errNum, "clEnqueueNDRangeKernel 0");

    clFinish(fila);

    errNum |= clGetEventProfilingInfo(evento0, 
            CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
            &ev_start_time, NULL);

    errNum |= clGetEventProfilingInfo(evento0, 
            CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
            &ev_end_time, NULL);

    checkErr(errNum, "Error Profiling cache");

    run_time_k0 += (double)(ev_end_time - ev_start_time)/1e6; // in msec

    errNum = clEnqueueReadBuffer(fila, cacheBuffer, CL_TRUE, 0, 
            sizeof(cl_int8) * N, cache, 0, NULL, &releituraFeita);
    checkErr(errNum, CL_SUCCESS);
    clWaitForEvents(1, &releituraFeita);

    printf ( " Cached!" );

    fp = fopen ("cache.txt", "w");
    for ( i = 0; i < N*8; i++ ) {
        //fprintf ( fp, "%d ", cache[i] );
        if ( i % 24 == 0 ) 
            fprintf ( fp, "\n" );
    }
    fclose (fp);

    printf ( "\nEntering in loop... " );

    while(!vazio(Mask, n)) {

        for ( i = 0; i < NLOOP; i++) {
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
                    &evento1);
            checkErr(errNum, "clEnqueueNDRangeKernel 1");

            clFinish(fila);

            /*
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
                    &evento2);
            checkErr(errNum, "clEnqueueNDRangeKernel2");
            clFinish(fila);
            */
            errNum |= clGetEventProfilingInfo(evento1, 
                    CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
                    &ev_start_time, NULL);

            errNum |= clGetEventProfilingInfo(evento1, 
                    CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
                    &ev_end_time, NULL);

            checkErr(errNum, "Error Profiling Kernel 1");
            run_time_k1 += (double)(ev_end_time - ev_start_time)/1e6; // in msec
            /*

            errNum |= clGetEventProfilingInfo(evento2, 
                    CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
                    &ev_start_time, NULL);

            errNum |= clGetEventProfilingInfo(evento2, 
                    CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
                    &ev_end_time, NULL);

            checkErr(errNum, "Error Profiling");
            run_time_k2 += (double)(ev_end_time - ev_start_time)/1e6; // in msec
            */
        }

        errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
                sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
        checkErr(errNum, CL_SUCCESS);
        clWaitForEvents(1, &releituraFeita);

    }
    printf ( "Exiting from loop...\n" );
    printf ( "Time K0: %lf ms\n", run_time_k0 );
    printf ( "Time K1: %lf ms\n", run_time_k1 );
    printf ( "Total Parallel Time: %lf ms\n", run_time_k1+run_time_k0 );

    errNum = clEnqueueReadBuffer(   fila, 
        FVvalBuffer, 
        CL_TRUE, 
        0, 
        sizeof(cl_int) * n, 
        F->V->val,
        0, 
        NULL, 
        NULL    );

    checkErr ( errNum, "Reading FVval" );

    errNum = clEnqueueReadBuffer(   fila, 
        FPvalBuffer, 
        CL_TRUE, 
        0, 
        sizeof(cl_int) * n, 
        F->P->val,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading FPval" );

    errNum = clEnqueueReadBuffer(   fila, 
        FRvalBuffer, 
        CL_TRUE, 
        0, 
        sizeof(cl_int) * n, 
        F->R->val,
        0, 
        NULL, 
        NULL    );

    checkErr ( errNum, "Reading FRval" );

    /*
    // Path propagation

    while(!EmptyGQueue(Q)){
        p = RemoveGQueue(Q);
        u.x = p % I->ncols;
        u.y = p / I->ncols;
        w.x = F->R->val[p] % I->ncols;
        w.y = F->R->val[p] / I->ncols;
        for (i=1; i < A->n; i++) {
            v.x = u.x + A->dx[i];
            v.y = u.y + A->dy[i];
            if (ValidPixel(I,v.x,v.y)){
                q   = v.x + I->tbrow[v.y];
                if (F->V->val[q]>F->V->val[p]){
                    tmp = (v.x-w.x)*(v.x-w.x)+(v.y-w.y)*(v.y-w.y);
                    if (tmp < F->V->val[q]){
                        if (F->V->val[q]!=INT_MAX) RemoveGQueueElem(Q, q);
                        F->V->val[q]=tmp; F->R->val[q]=F->R->val[p]; F->P->val[q]=p;
                        InsertGQueue(&Q,q);
                    }
                }
            }
        }
    }

    DestroyGQueue(&Q);
*/
    DestroyAdjRel(&A);
    DestroyAdjRel(&A4);
    free ( Mask );
    free ( cache );

    return(F);
}

int main(int argc, char **argv)
{
    int p;
    char outfile[100];
    char *file_noext;
    timer    *t1=NULL,*t2=NULL;
    Image    *img,*aux, *sqrt_edt;
    Forest   *edt;

    /* The following block must the remarked when using non-linux machines */

    void *trash = malloc(1);
    struct mallinfo info;
    int MemDinInicial, MemDinFinal;
    free(trash);
    info = mallinfo();
    MemDinInicial = info.uordblks;

    /*----------------------------------------------------------------------*/

    if (argc != 2) {
        printf("Usage: %s <image.pgm>\n", argv[0]);
        printf("image.pgm: a binary image for which an Euclidian Distance Transform will be computed\n");
        exit(0);
    }

    aux = ReadImage(argv[1]);

    file_noext = strtok(argv[1],".");

    if (MaximumValue(aux)!=1){
        fprintf(stderr,"Input image must be binary with values 0/1 \n");
        fprintf(stderr,"Assuming lower threshold 100 for this conversion\n");
        img = Threshold(aux,100,INT_MAX);
        WriteImage(img,"shape.pgm");
    }else{
        img = CopyImage(aux);
    }
    DestroyImage(&aux);

    t1 = Tic();

    edt = DistTrans(img);

    t2 = Toc();

    fprintf(stdout,"Euclidian Distance Transform in %f ms\n",CTime(t1,t2));

    sqrt_edt = CreateImage(img->ncols, img->nrows);
    for(p = 0; p < img->ncols*img->nrows;p++)
        sqrt_edt->val[p] = (int)sqrtf(edt->V->val[p]);

    sprintf(outfile,"%s_edt.pgm",strtok(argv[1],"."));

    WriteImage(sqrt_edt,outfile);

    DestroyForest(&edt);
    DestroyImage(&img);
    DestroyImage(&sqrt_edt);

    /* The following block must the remarked when using non-linux machines */

    info = mallinfo();
    MemDinFinal = info.uordblks;
    if (MemDinInicial!=MemDinFinal)
        printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
                MemDinInicial,MemDinFinal);


    return(0);
}
