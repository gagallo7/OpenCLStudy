/*
   Copyright (CostCost) <2010> <Alexandre Xavier FalcÃ£o and Thiago Vallin Spina>

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

/* Including OpenCL framework for parallelizing */
#include "oclFunctions.h"

/* Papers related to this program:

   @incollection{Lotufo00,
   author    = "R.A. Lotufo and A.X. Falc{\~{a}}o",
   booktitle = "Mathematical Morphology and its Applications to Image and Signal Processing (ISMM)",
   title     = "The ordered queue and the optimality of the watershed approaches",
   publisher = "Kluwer",
   volume    = "18",
   month     = "Jun",
   pages     = "341--350",
   year      = "2000"
   }

   @article{Miranda09b,
   author = "P.A.V. Miranda and A.X. Falc{\~{a}}o",
   title = "Links Between Image Segmentation Based on Optimum-Path Forest and Minimum Cut in Graph",
   journal = "Journal of Mathematical Imaging and Vision",
   pages = {128--142},
   volume = 35,
   number = "2",
   publisher = {Springer},
   month = "Oct",
   year = {2009},
   note = "doi:10.1007/s10851-009-0159-9"
   }

   @inproceedings{Audigier07,
   author = "R. Audigier and R.A. Lotufo",
   title = "Watershed by Image Foresting Transform, tie-zone, and theoretical relationship with other watershed definitions",
   booktitle = "Mathematical Morphology and its Applications to Signal and Image Processing (ISMM)",
   address = "Rio de Janeiro, RJ",
   publisher = "MCT/INPE",
   pages = "277--288",
   month = "Oct",
   year = 2007
   }

*/

cl_int SetLabel (cl_int* pred, cl_int* label, cl_int n, cl_int pos) {
    if ( pred [pos] == -1 ) {
        return label[pos];
    }
    //label[pos] = SetLabel (pred, label, n, pred[pos]);
    return SetLabel (pred, label, n, pred[pos]);
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
    //	std::cout << "Ocorreu um erro durante o uso do context: " << errInfo << std::endl;
    printf ( "An error happenned during context use: %s\n", errInfo );
    // Ignorando limpeza de memória e saindo diretamente
    exit(1);
}

// Watershed from binary marker

        Image *Watershed(Image *img, Set *Obj, Set *Bkg)
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
    cl_program programa, programa2;
    cl_kernel kernel, kernel2;
    cl_event evento;

    cl_mem  costBuffer, costvalBuffer, costtbrowBuffer,
            numVbuffer, SEMbuffer, extraBuffer,
            Abuffer, Adxbuffer, Adybuffer,
            labelBuffer, labelvalBuffer, labeltbrowBuffer,
            Mbuffer, MvalBuffer, MtbrowBuffer,
            Ubuffer, Ulabelbuffer,
            Cbuffer, Clabelbuffer,
            UPredbuffer, CPredbuffer,
            imgBuffer, imgvalBuffer, imgtbrowBuffer;


    /*--------------------------------------------------------*/
    AdjRel *A=NULL;
    GQueue *Q=NULL;
    Image  *cost=NULL,*label=NULL;
    Pixel   u,v;
    cl_int     i,p,q,n,tmp,Cmax=MaximumValue(img);
    Set    *S;


    cost  = CreateImage(img->ncols,img->nrows);
    label = CreateImage(img->ncols,img->nrows);
    n     = img->ncols*img->nrows;
    Q     = CreateGQueue(Cmax+1,n,cost->val);

    size_t Asize;
    A     = Circular(1.5);


    /* Preparing device for OpenCL program */
    //prepareAllDataForDevice(errNum, nPlataformas, nDispositivos, &listaDispositivoID, &contexto, &contextCallback, fila, programa, programa2, &kernel, &kernel2);

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
    cl_uint j;
    for (j=0; j < nPlataformas; j++) {
        // Atribuindo o número de dispositivos de GPU a nDispositivos
        errNum = clGetDeviceIDs (	listaPlataformaID[j],
                //								CL_DEVICE_TYPE_ALL,
                CL_DEVICE_TYPE_CPU,
                //			CL_DEVICE_TYPE_GPU,
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

    printf ( "Source code of kernel 1:\n%s\n\n", source_str );

    programa = clCreateProgramWithSource(
            contexto, 
            1, 
            (const char **)&source_str,
            (const size_t *)&source_size, 
            &errNum);
    checkErr(errNum, "clCreateProgramWithSource");

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
    /*
    */

    // Compilando programa
    printf ( "Compiling the kernel 1 ... \n" );
    errNum = clBuildProgram (
            programa,
            nDispositivos,
            listaDispositivoID,
            NULL,
            NULL,
            NULL);

    printf ( "kernel1 compiled! \n" );

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
    printf ( "Compiling the kernel2 ... \n" );
    errNum = clBuildProgram (
            programa2,
            nDispositivos,
            listaDispositivoID,
            NULL,
            NULL,
            NULL);

    /*
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
    */

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

    /* TODO */
    /* Dijkstra mode */
    /* Ignore GQueue, create a normal queue 
     * remembering that mask and update arrays are
     * required for the OpenCL version of
     * Dijkstra algorithm */

    /* For Kernel's Buffer */

    /*
       Image* Mask = CreateImage(img->ncols, img->nrows);
       Image* UpdateCost = CreateImage(img->ncols, img->nrows);
       Image* CostCost = CreateImage(img->ncols, img->nrows);
       */
    cl_int* Mask = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* CostCost = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* UpdateCost = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* Clabel = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* Ulabel = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* UpdatePred = (cl_int *) alloca (n * sizeof ( cl_int ) );
    cl_int* CostPred = (cl_int *) alloca (n * sizeof ( cl_int ) );
    static volatile cl_int semaforo = 0;
    cl_int extra = 0;
    /*
    */
    /* Trivial path initialization */

    for (p=0; p < n; p++){
        cost->val[p] =INT_MAX;
        CostCost[p] = INT_MAX;
        UpdateCost[p] = INT_MAX;
        Mask[p] = false;
    }
    S = Obj;
    while(S != NULL){
        p=S->elem;

        label->val[p]=1;
        Ulabel[p]=1;
        Clabel[p]=1;

        cost->val[p]=0;
        CostCost[p] = 0;
        UpdateCost[p] = 0;

        UpdatePred[p] = -1;
        CostPred[p] = -1;

        InsertGQueue(&Q,p);
        Mask[p] = true;
        S = S->next;
    }
    S = Bkg;
    while(S != NULL){
        p=S->elem;

        label->val[p]=0;
        Ulabel[p]=0;
        Clabel[p]=0;

        cost->val[p]=0;
        CostCost[p] = 0;
        UpdateCost[p] = 0;

        UpdatePred[p] = -1;
        CostPred[p] = -1;

        InsertGQueue(&Q,p);
        Mask[p] = true;
        S = S->next;
    }



    // Alocando Buffers
    /*
    */
    printf ( "\n-------------------\nTamanho Image: %d\n", sizeof(Image) );
    printf ( "\n-------------------\nn Image: %d == %dB\n", n, n * sizeof(cl_int) );
    printf ( "\n-------------------\nM: %d   A->n:%d\n", 
            malloc_usable_size (Mask), A->n);
    printf ( "\n-------------------\nMalloc Image: %d\n", malloc_usable_size (img) );
    imgBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(Image),
            img,
            &errNum);
    checkErr(errNum, "clCreateBuffer(img)");

    imgvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            img->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    imgtbrowBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            img->nrows * ( sizeof(cl_int) ),
            img->tbrow,
            &errNum);
    checkErr(errNum, "clCreateBuffer(img_tbrowBuffer)");

    Mbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * (sizeof(cl_int)),
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

    Ulabelbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            Ulabel,
            &errNum);
    checkErr(errNum, "clCreateBuffer(Ulabel_buffer)");

    Clabelbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            Clabel,
            &errNum);
    checkErr(errNum, "clCreateBuffer(Clabel)");

    Ubuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            UpdateCost,
            &errNum);
    checkErr(errNum, "clCreateBuffer(U_buffer)");

    Cbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            CostCost,
            &errNum);
    checkErr(errNum, "clCreateBuffer(CostCost)");

    UPredbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            UpdatePred,
            &errNum);
    checkErr(errNum, "clCreateBuffer(CostCost)");

    CPredbuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            CostPred,
            &errNum);
    checkErr(errNum, "clCreateBuffer(CostCost)");

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


    /*
       __global Image *img,
       __global Image *cost,
       __global Image *label,
       __global AdjRel *A,
       __global cl_int *Mask,
       __global cl_int *CostCost,
       __global cl_int *UpdateCost,
       __global cl_int *sem,
       */

    // Escolhendo o primeiro dispositivo e criando a fila de comando
    fila = clCreateCommandQueue (
            contexto, 
            listaDispositivoID[0],
            CL_QUEUE_PROFILING_ENABLE,
            &errNum);
    checkErr(errNum, "clCreateCommandQueue");

    // Setando os argumentos da função do Kernel
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imgBuffer);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &imgvalBuffer);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &imgtbrowBuffer);
    //	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &costBuffer);
    //errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &costvalBuffer);
    //errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &labelBuffer);
    //errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &labelvalBuffer);
    errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &Abuffer);
    errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &Cbuffer);
    errNum |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &Ubuffer);
    errNum |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &Ulabelbuffer);
    errNum |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &Clabelbuffer);
    errNum |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &UPredbuffer);
    errNum |= clSetKernelArg(kernel, 12, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel, 13, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel, 14, sizeof(cl_mem), &extraBuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 1");

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
    /*
    */

    // Definindo o número de work-items globais e locais
    const size_t globalWorkSize[1] = { n };
    const size_t localWorkSize[1] = { 1 };

    // releituraFeita e um evento que sincroniza a atualizacao feita
    // por cada chamada aos kernels

    cl_event releituraFeita;
    errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
            sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
    checkErr(errNum, CL_SUCCESS);
    clWaitForEvents(1, &releituraFeita);

    printf ( "Entering in loop...\n" );
    cl_int vez = 0;
    while(!vazio(Mask, n)) {

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


        /*
        */
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

        errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
                sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
        checkErr(errNum, CL_SUCCESS);
        clWaitForEvents(1, &releituraFeita);


    }



    /*
       for (i = 0; i < n; i++) {
       if (Mask[i])
    //printf ( "%d: %d ", i, Mask[i] );
    printf ( "%d ", Mask[i] );
    }
    printf("\n");
    */
    errNum = clEnqueueReadBuffer(   fila, 
            Clabelbuffer, 
            CL_FALSE, 
            0, 
            sizeof(cl_int) * n, 
            label->val, 
            0, 
            NULL, 
            NULL    );

    errNum = clEnqueueReadBuffer(   fila, 
            CPredbuffer, 
            CL_FALSE, 
            0, 
            sizeof(cl_int) * n, 
            CostPred,
            0, 
            NULL, 
            NULL    );


    cl_ulong ev_start_time=(cl_ulong)0;     
    cl_ulong ev_end_time=(cl_ulong)0;
    clFinish(fila);

    errNum = clWaitForEvents(1, &evento);
    errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &ev_start_time, NULL);
    errNum |= clGetEventProfilingInfo(evento, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &ev_end_time, NULL);

    checkErr(errNum, "Error Profiling");
    double run_time_gpu = (double)(ev_end_time - ev_start_time)/1000; // in usec

    tS1 = (timer *)malloc(sizeof(timer));
    gettimeofday(tS1, NULL);
    printf("\nEnd of parallel code. Labelling correction started.\n");
    for (i=0; i<n; i++) {
        label->val[i] = SetLabel (CostPred, label->val, n, i);
    }
    tS2 = (timer *)malloc(sizeof(timer));
    gettimeofday(tS2, NULL);

    /*
    printf("\nLabel\n");
    for (i = 0; i < n; i++) {
        if (label->val[i])
            printf ( "%d ", label->val[i] );
    }
    printf("\nLabel\n");
    printf("\n");
*/
    printf ( "\nRecursion time (Serial): %fms\nParallel Execution time: %lfms\n",
                CTime(tS1, tS2), run_time_gpu);


    /*
       for (i = 0; i < n; i++) {
       if (i <100)
       label->val[i] = i%25;
       }
       */
    /* Path propagation */

    /*
       while (!EmptyGQueue(Q)){
       p   = RemoveGQueue(Q);
       u.x = p%img->ncols;
       u.y = p/img->ncols;
       for (i=1; i < A->n; i++) {
       v.x = u.x + A->dx[i];
       v.y = u.y + A->dy[i];
       if (ValidPixel(img,v.x,v.y)){
       q   = v.x + img->tbrow[v.y];
       if (cost->val[p] < cost->val[q]){

       tmp = MAX(cost->val[p] , img->val[q]);
       if (tmp < cost->val[q]){
       if (cost->val[q]!=INT_MAX)
       RemoveGQueueElem(Q,q);
       cost->val[q] =tmp;
       label->val[q]=label->val[p];
       InsertGQueue(&Q,q);
       }
       }

       }
       }
       }
       */
    printf ( "\n~~~~~~~%d %d %d~~~~~~~\n", *(&A->dx), A->dx[0], *A->dx );


    DestroyGQueue(&Q);
    DestroyImage(&cost);
    DestroyAdjRel(&A);


    return(label);
}

cl_int main(cl_int argc, char **argv)
{
    timer    *t1=NULL,*t2=NULL;
    Image    *img=NULL,*grad=NULL;
    Image    *label=NULL;
    CImage   *cimg=NULL;
    Set      *Obj=NULL,*Bkg=NULL;
    char     outfile[100];
    char     *file_noext;
    /*--------------------------------------------------------*/

    void *trash = malloc(1);
    struct mallinfo info;
    cl_int MemDinInicial, MemDinFinal;
    free(trash);
    info = mallinfo();
    MemDinInicial = info.uordblks;


    if (argc!=4){
        fprintf(stderr,"Usage: watershed <image.pgm> <gradient.pgm> <seeds.txt>\n");
        fprintf(stderr,"image.pgm: image to overlay the watershed lines on it\n");
        fprintf(stderr,"gradient.pgm: gradient image to compute the watershed segmentation\n");
        fprintf(stderr,"seeds.txt: seed pixels\n");
        exit(-1);
    }


    img   = ReadImage(argv[1]);
    grad  = ReadImage(argv[2]);
    ReadSeeds(argv[3],&Obj,&Bkg);

    file_noext = strtok(argv[1],".");

    t1 = Tic();

    label = Watershed(grad,Obj,Bkg);

    t2 = Toc();

    fprintf(stdout,"Processing time in %f ms\n",CTime(t1,t2));

    cimg = DrawLabeledRegions(img,label);
    sprintf(outfile,"%s_result.ppm",file_noext);
    WriteCImage(cimg,outfile);
    DestroyImage(&grad);
    DestroyImage(&img);
    DestroyImage(&label);
    DestroyCImage(&cimg);
    DestroySet(&Obj);
    DestroySet(&Bkg);

    /* ---------------------------------------------------------- */

    info = mallinfo();
    MemDinFinal = info.uordblks;
    if (MemDinInicial!=MemDinFinal)
        printf("\n\nDinamic memory was not completely deallocated (%d, %d)\n",
                MemDinInicial,MemDinFinal);

    return(0);
}
