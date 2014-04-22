/*
    Copyright (C) <2010> <Alexandre Xavier Falcão and Thiago Vallin Spina>

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

    written by A.X. Falc�o <afalcao@ic.unicamp.br> and by T.V. Spina
    <tvspina@liv.ic.unicamp.br>, 2010

*/

#include "ift.h"
#define DEVICE CL_DEVICE_TYPE_GPU
#define NLOOP 3
#define NIL -1

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

int detectRootCycle ( cl_int* root, int n, int k )
{
    //int* mask = (int* ) calloc ( n, sizeof ( int  ));
    int search = root[k];
    int i = 0;

    while ( search != root[search] && i <= n )
    {
        search = root[search];
        i++;
    }

    //free ( mask );

    if ( i == n )
    {
        printf ( "Ciclo no mapa de raízes em %i\n", search );
        return 0;
    }
    
    return 1;
}

int detectPredCycle ( cl_int* pred, int n, int k )
{
    int* mask = (int* ) malloc ( n * sizeof ( int  ));
    int search = pred[k];
    int i;

    for ( i = 0; i < n; i++ )
    {
        mask [i] = 0;
    }

    while ( search >= 0 && !mask[search] )
    {
        mask [search] = 1;
        search = pred[search];
    }

    free ( mask );

    if ( search >= 0 )
    {
        return 0;
    }
    
    return 1;
}

void makeLabelMaps ( cl_int* pred, cl_int* root, cl_int* label, int n, Image* img )
{
    int i, p;

    Image* lP = ( Image* ) malloc ( sizeof ( Image ) );
    Image* lR = ( Image* ) malloc ( sizeof ( Image ) );

    FILE* fLabelP = fopen ( "predLabel.txt", "w" );
    FILE* fLabelR = fopen ( "rootLabel.txt", "w" );

    int* labelPred = ( int * ) malloc ( sizeof ( int) * n );
    int* labelRoot = ( int * ) malloc ( sizeof ( int) * n );

    lP->val = labelPred;
    lR->val = labelRoot;

    CImage   *cimg=NULL;

    for ( i = 0; i < n; i++ )
    {
        p = i;
        while ( pred [ p ] != NIL )
        {
            p = pred [ p ];
        }

        fprintf ( fLabelP, "%d ", label [ p ] );
        labelPred [ i ] = label [ p ];
        labelRoot [ i ] = label [ root [ i ] ];
        fprintf ( fLabelR, "%d ", label [ root[i] ] );
    }

    cimg = DrawLabeledRegions(img,lP);
    WriteCImage(cimg,"temp/mapFromPred.ppm");

    cimg = DrawLabeledRegions(img,lR);
    WriteCImage(cimg,"temp/mapFromRoot.ppm");

    fclose ( fLabelP );
    fclose ( fLabelR );

}


    
cl_int vazio (cl_int Mask[], cl_int n) {
    cl_int i;
    for (i = 0; i < n; i++) {
        if ( Mask[i] > 0 )
        {
            return 0;
        }
    }
    return 1;
}

void CL_CALLBACK contextCallback (
        const char *errInfo,
        const void *private_info,
        size_t cb,
        void *user_data			) {
    printf ( "An error happenned during context use: %s\n", errInfo );
    exit(1);
}

void SetLabelIterative (cl_int* pred, cl_int* label, cl_int n) {
    int pd;
    int i = 0, j;

    for ( i = 0; i < n; i++ ) {
        pd = i;
        j = 0;

        while ( pred [pd] >= 0 && j < n ) {
            pd = pred [ pd ];
            j++;
        }
        if ( pred [pd] == -1 )
            label [i] = label [ pd ];
    }
}

// Watershed from binary marker

Image *Watershed(Image *img, Set *Obj, Set *Bkg, Image *imgOrig)
{
  timer    *t1=NULL,*t2=NULL;
  AdjRel *A=NULL;
  GQueue *Q=NULL;
  Image  *cost=NULL,*label=NULL;
  Pixel   u,v;
  int     i,p,q,n,tmp,Cmax=MaximumValue(img);
  Set    *S;

  cost  = CreateImage(img->ncols,img->nrows);
  label = CreateImage(img->ncols,img->nrows);
  n     = img->ncols*img->nrows;
  Q     = CreateGQueue(Cmax+1,n,cost->val);
  A     = Circular(1.5);
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
            IBuffer, IvalBuffer, imgValBuffer,
            labelBuffer, predBuffer,
            cacheBuffer, cache2Buffer, cache3Buffer,
            CminBuffer, matchesBuffer, rootBuffer,
            rwLocksBuffer, MblockBuffer;


    /*--------------------------------------------------------*/

    /* Preparing device for OpenCL program */
    errNum = clGetPlatformIDs(0, NULL, &nPlataformas);
    checkErr( (errNum != CL_SUCCESS) ? errNum : 
            (nPlataformas <= 0 ? -1 : CL_SUCCESS),
            "clGetPlataformsIDs");
    // Se n�o houve erro, alocar mem�ria para cl_platform_id
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
        // Atribuindo o n�mero de dispositivos de GPU a nDispositivos
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
        // Conferindo se h� dispositivos de CPU
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

    fp = fopen("pqueue.cl", "r");
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

    printf ( "Compiling the kernel 0 ... \r" );
    errNum = clBuildProgram (
            programa0,
            nDispositivos,
            listaDispositivoID,
            //"-cl-fast-relaxed-math -cl-mad-enable",
            "-cl-opt-disable -Werror",
            NULL,
            NULL);

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
    printf ( "init.cl compiled!          \n" );

    printf ( "Compiling the kernel 1 ... \r" );
    errNum = clBuildProgram (
            programa,
            nDispositivos,
            listaDispositivoID,
            //"-cl-fast-relaxed-math -cl-mad-enable",
            "-cl-opt-disable -Werror",
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

        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }

    printf ( "pqueue.cl compiled!         \n" );

    errNum = clBuildProgram (
            programa3,
            nDispositivos,
            listaDispositivoID,
            "-cl-opt-disable -Werror",
            NULL,
            NULL);


    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa3,
                listaDispositivoID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);
        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }
    printf ( "kernel3 compiled! \n" );

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
            "pqueue",
            &errNum);
    checkErr(errNum, "clCreateKernel1");

    printf ( "KERNEL 2\n" );
    // Criando o objeto do Kernel
    kernel2 = clCreateKernel (
            programa,
            "dijkstra2",
            &errNum);
    checkErr(errNum, "clCreateKernel2");

    printf ( "KERNEL 3\n" );
    // Criando o objeto do Kernel3
    kernel3 = clCreateKernel (
            programa,
            "dijkstra3",
            &errNum);
    checkErr(errNum, "clCreateKernel3");

    int N = n;
    int t = 0;
    while ( N > 0 ) {
        N >>= 1;
        t++;
    }

    N = 1 << t;

    printf ("n=%d ... N=%d (2^%d)## ncols=%d\n", n, N, t, img->ncols);


    printf ("-- ");
    cl_int* Mask = (cl_int *) malloc (N * sizeof ( cl_int ) );
    cl_int* pred = (cl_int *) malloc (N * sizeof ( cl_int ) );
    cl_int* root = (cl_int *) malloc (N * sizeof ( cl_int ) );
    cl_int* Mblocks = (cl_int *) calloc ( N, sizeof ( cl_int ) );
    /*
    cl_int* CostCost = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* UpdateCost = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* Clabel = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* Ulabel = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* UpdatePred = (cl_int *) malloc (n * sizeof ( cl_int ) );
    cl_int* CostPred = (cl_int *) malloc (n * sizeof ( cl_int ) );
    */
    static volatile cl_int semaforo = 0;
    volatile cl_int* rwLocks = ( volatile cl_int *) calloc ( n, sizeof ( volatile cl_int ) );
    cl_int extra = Cmax;
    cl_int16* cache = (cl_int16 *) malloc ( sizeof (cl_int16) * N );

    //memset (Mask, 0, N*4);

    printf ( "Empty test = %d\n", vazio ( Mask, n ) );

    memset (pred, 0, n*4);

  /* Trivial path initialization */

    for (p=0; p < N; p++)
    {
        Mask [ p ] = 0;
    }
    for (p=0; p < n; p++){
        cost->val[p] =INT_MAX;
        pred[p] = -2;
    }
    S = Obj;
    while(S != NULL){
        p=S->elem;
        label->val[p]=1;
        cost->val[p]=0;
        pred[p] = NIL;
        //InsertGQueue(&Q,p);
        Mask [ p ] = 1;
        root[p] = p;
        S = S->next;
    }
    S = Bkg;
    while(S != NULL){
        p=S->elem;
        label->val[p]=0;
        cost->val[p]=0;
        pred[p] = NIL;
        root[p] = p;
        //InsertGQueue(&Q,p);
        Mask [ p ] = 1;
        S = S->next;
    }

    int Cmin = 0;
    int matches = 0;

// This describes the format of the image data.
    cl_image_format format;
    format.image_channel_order = CL_INTENSITY;
    format.image_channel_data_type = CL_UNSIGNED_INT32;
 
    int width = img->ncols;
    int height = img->nrows;
    /*
    cl_mem input_image = clCreateImage2D(
                                           contexto,
                                           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           &format,
                                           img->ncols,
                                           img->nrows,
                                           0,
                                           img->val,
                                           &errNum
                                           );
    checkErr ( errNum, "Creating Input Image" );

    cl_mem output_image = clCreateImage2D( contexto,
                                           CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                           &format,
                                           img->ncols,
                                           img->nrows,
                                           0,
                                           img->val,
                                           &errNum
                                           );
    checkErr ( errNum, "Creating Output Image" );

    cl_sampler sampler = clCreateSampler (  contexto,
                                            CL_FALSE,
                                            CL_ADDRESS_CLAMP,
                                            CL_FILTER_NEAREST,
                                            &errNum );
    checkErr ( errNum, "Creating Sampler" );
      */                                      
    cacheBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
            sizeof(cl_int16) * N,
            cache,
            &errNum);
    checkErr(errNum, "clCreateBuffer(cache)");

    IBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof ( Image ),
            img,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    imgtbrowBuffer= clCreateBuffer (
            contexto,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            img->nrows * ( sizeof ( cl_int ) ),
            img->tbrow,
            &errNum);
    checkErr(errNum, "clCreateBuffer(img)");

    rootBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * sizeof(cl_int),
            root,
            &errNum);
    checkErr(errNum, "clCreateBuffer(root)");

    MblockBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            N * sizeof(cl_int),
            Mblocks,
            &errNum);
    checkErr(errNum, "clCreateBuffer(root)");

    CminBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            sizeof(cl_int),
            &Cmin,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    matchesBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            sizeof(cl_int),
            &matches,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    IvalBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            img->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(M_valBuffer)");

    imgValBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            img->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(imgValBuffer)");

    costBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            cost->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(costBuffer)");

    labelBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            label->val,
            &errNum);
    checkErr(errNum, "clCreateBuffer(labelBuffer)");

    predBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            n * ( sizeof(cl_int) ),
            pred,
            &errNum);
    checkErr(errNum, "clCreateBuffer(predBuffer)");

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

    rwLocksBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            n * sizeof ( volatile cl_int ),
            rwLocks,
            &errNum);
    checkErr(errNum, "clCreateBuffer(rwLocks)");

    fila = clCreateCommandQueue (
            contexto, 
            listaDispositivoID[0],
            CL_QUEUE_PROFILING_ENABLE,
            &errNum);
    checkErr(errNum, "clCreateCommandQueue");

    // Definindo o n�mero de work-items globais e locais
    const size_t globalWorkSize[1] = { N };
    const size_t localWorkSize[1] = { 1024 };
    const size_t globalWorkSize2[2] = { N , 8 };
    const size_t localWorkSize2[2] = { 1024, 1 };

    // Setando os argumentos da fun��o do Kernel

    errNum = clSetKernelArg(kernel0, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel0, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel0, 2, sizeof(cl_mem), &imgtbrowBuffer);
    errNum |= clSetKernelArg(kernel0, 3, sizeof(cl_mem), &Abuffer);
    errNum |= clSetKernelArg(kernel0, 4, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel0, 5, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel0, 6, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel0, 7, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel0, 8, sizeof(cl_mem), &cacheBuffer);
    /*
    errNum |= clSetKernelArg(kernel0, 9, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel0, 10, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 0");

    // Kernel 1: Dijkstra' kernel
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &costBuffer);
    errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &predBuffer);
    //errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &labelBuffer);
    errNum |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &cacheBuffer);
    errNum |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &CminBuffer);
    errNum |= clSetKernelArg(kernel, 12, sizeof(cl_mem), &rootBuffer);
    errNum |= clSetKernelArg(kernel, 13, sizeof(cl_mem), &rwLocksBuffer);
    errNum |= clSetKernelArg(kernel, 14, sizeof(cl_mem), &MblockBuffer );
    /*
    errNum |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel, 12, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 1");

    errNum = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel2, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel2, 2, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel2, 3, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel2, 4, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &costBuffer);
    errNum |= clSetKernelArg(kernel2, 6, sizeof(cl_mem), &predBuffer);
    errNum |= clSetKernelArg(kernel2, 7, sizeof(cl_mem), &imgValBuffer);
    //errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel2, 8, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel2, 9, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel2, 10, sizeof(cl_mem), &labelBuffer);
    errNum |= clSetKernelArg(kernel2, 11, sizeof(cl_mem), &cacheBuffer);
    /*
    errNum |= clSetKernelArg(kernel2, 11, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel2, 12, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 2");


    errNum = clSetKernelArg(kernel3, 0, sizeof(cl_mem), &Mbuffer);
    errNum |= clSetKernelArg(kernel3, 1, sizeof(cl_mem), &SEMbuffer);
    errNum |= clSetKernelArg(kernel3, 2, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel3, 3, sizeof(cl_mem), &CminBuffer);
    errNum |= clSetKernelArg(kernel3, 4, sizeof(cl_mem), &matchesBuffer);
    errNum |= clSetKernelArg(kernel3, 5, sizeof(cl_mem), &costBuffer);
    checkErr(errNum, "clSetKernelArg at Kernel 3");

    cl_int vez = 0, nqueue = 0;
    double run_time_k0 = 0;
    double run_time_k1 = 0;
    double run_time_k2 = 0;
    double run_time_k3 = 0;
    cl_ulong ev_start_time=(cl_ulong)0;     
    cl_ulong ev_end_time=(cl_ulong)0;
    gettimeofday(tS1, NULL);
    // releituraFeita e' um evento que sincroniza a atualizacao feita
    // por cada chamada aos kernels

    cl_event releituraFeita;
    errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
            sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
    checkErr(errNum, CL_SUCCESS);
    clWaitForEvents(1, &releituraFeita);

    // Calling the cache kernel.
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

    for ( i = 0; i < n; i++ ) {
        if ( pred[i] >= 0 )
            printf ( "%d ", pred[i] );
    }


    printf ( "\nEntering in loop...\n" );

    //while( Cmin < Cmax ) 
        while ( !vazio ( Mask, n ) )
        {

        errNum = clEnqueueWriteBuffer(   
                fila, 
                CminBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &Cmin,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading Cmin" );

        errNum = clEnqueueReadBuffer(   
                fila, 
                CminBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &Cmin,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading Cmin" );

        errNum = clEnqueueReadBuffer(   
                fila, 
                extraBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &extra,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading extra" );

        errNum = clEnqueueReadBuffer(   
                fila, 
                matchesBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &matches,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading matches" );

        errNum = clEnqueueReadBuffer(   
                fila, 
                Mbuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int) * n, 
                Mask,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading extra" );

        errNum = clEnqueueReadBuffer(   
                fila, 
                costBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int) * n, 
                cost->val,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading extra" );

        nqueue = 0;
        for ( i = 0; i < n; i++ ) 
        {
            if ( cost->val [ i ] == extra && Mask[i] )
            {
                vez++;
            }
            if ( Mask [i] )
            {
                nqueue++;
            }
        }

        printf ( "Cmin,%d,Matches,%d,extra,%d,matchesP,%d,nqueue,%d\n", Cmin, vez, extra, matches, nqueue );


        /*
        */

        extra = 0;
        errNum = clEnqueueWriteBuffer(   
                fila, 
                extraBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &extra,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Writing Cmin" );

        memset ( Mblocks, 0, sizeof Mblocks );

        errNum = clEnqueueWriteBuffer(   
                fila, 
                MblockBuffer, 
                CL_TRUE, 
                0, 
                (N>>7) * sizeof(cl_int), 
                Mblocks,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Writing Mblocks" );


        for ( i = 0; i < NLOOP; i++ )
        {

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

            errNum |= clGetEventProfilingInfo(evento1, 
                    CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
                    &ev_start_time, NULL);

            errNum |= clGetEventProfilingInfo(evento1, 
                    CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
                    &ev_end_time, NULL);

            checkErr(errNum, "Error Profiling Kernel 1");
            run_time_k1 += (double)(ev_end_time - ev_start_time)/1e6; // in msec
        }
        vez = 0;

        errNum = clEnqueueWriteBuffer(   
                fila, 
                matchesBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &vez,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Writing Matches" );

        extra = Cmax+1;
        errNum = clEnqueueWriteBuffer(   
                fila, 
                extraBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &extra,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Writing extra" );

        errNum = clEnqueueNDRangeKernel (
                fila,
                kernel3,
                1,
                NULL,
                globalWorkSize,
                localWorkSize,
                0,
                NULL,
                &evento1);
        checkErr(errNum, "clEnqueueNDRangeKernel 3");

        clFinish(fila);

        errNum |= clGetEventProfilingInfo(evento1, 
                CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
                &ev_start_time, NULL);

        errNum |= clGetEventProfilingInfo(evento1, 
                CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
                &ev_end_time, NULL);

        checkErr(errNum, "Error Profiling Kernel 3");
        run_time_k3 += (double)(ev_end_time - ev_start_time)/1e6; // in msec
        /*
        */

        errNum = clEnqueueReadBuffer(fila, extraBuffer, CL_FALSE, 0, 
                sizeof(cl_int), &Cmin, 0, NULL, &releituraFeita);
        checkErr(errNum, "Reading Mask");
        clWaitForEvents(1, &releituraFeita);

        errNum = clEnqueueReadBuffer(   
                fila, 
                extraBuffer, 
                CL_TRUE, 
                0, 
                sizeof(cl_int), 
                &extra,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading extra" );

        //printf ( "\n--------Cmin NEW = %d  Extra COnf %d-----------\n", Cmin, extra );

        /*
        errNum = clEnqueueReadBuffer(fila, Mbuffer, CL_FALSE, 0, 
                sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
        checkErr(errNum, "Reading Mask");
        clWaitForEvents(1, &releituraFeita);
        Cmin = vazio(Mask, n);
        */
    }



/*
    errNum = clReleaseMemObject ( Mbuffer );
    errNum |= clReleaseMemObject ( cacheBuffer );
    errNum |= clReleaseMemObject ( IBuffer );
   
    checkErr(errNum, "clRelease");
*/


    fp = fopen ("diffPred.txt", "w");
    for ( i = 0; i < N; i++ ) {
        if ( pred[i] >= 0 )
            fprintf ( fp, "%d ", pred[i] - i );
        else
            fprintf ( fp, "-1 " );
        if ( i % 24 == 0 ) 
            fprintf ( fp, "\n" );
    }
    fclose (fp);

    /*
            errNum = clEnqueueNDRangeKernel (
                    fila,
                    kernel3,
                    1,
                    NULL,
                    globalWorkSize2,
                    localWorkSize2,
                    0,
                    NULL,
                    &evento2);
            checkErr(errNum, "clEnqueueNDRangeKernel 3");

            clFinish ( fila );
            errNum |= clGetEventProfilingInfo(evento2, 
                    CL_PROFILING_COMMAND_START, sizeof(cl_ulong),
                    &ev_start_time, NULL);

            errNum |= clGetEventProfilingInfo(evento2, 
                    CL_PROFILING_COMMAND_END, sizeof(cl_ulong), 
                    &ev_end_time, NULL);

            checkErr(errNum, "Error Profiling Kernel 3");
            run_time_k3 += (double)(ev_end_time - ev_start_time)/1e6; // in msec
*/
    printf ( "Reading buffer...\n" );
    errNum = clEnqueueReadBuffer(   fila, 
        labelBuffer, 
        CL_FALSE, 
        0, 
        sizeof(cl_int) * n, 
        label->val,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading label" );
    /*
*/
    fp = fopen ("label.txt", "w");
    for ( i = 0; i < n; i++ ) {
        fprintf ( fp, "%d ", label->val[i] );
    }
    fclose (fp);

    errNum = clEnqueueReadBuffer(   fila, 
        costBuffer, 
        CL_FALSE, 
        0, 
        sizeof(cl_int) * n, 
        cost->val,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading cost" );

    errNum = clEnqueueReadBuffer(   fila, 
        predBuffer, 
        CL_FALSE, 
        0, 
        sizeof(cl_int) * n, 
        pred,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading pred" );

    errNum = clEnqueueReadBuffer(   fila, 
        rootBuffer, 
        CL_FALSE, 
        0, 
        sizeof(cl_int) * n, 
        root,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading pred" );

    printf ( "\r\rRead!\n" );

    fp = fopen ("pred.txt", "w");
    for ( i = 0; i < n; i++ ) {
        if ( pred[i] >= 0 )
        {
            /*
            printf ( "Detecting cycle in %d...\r", i );
            if ( detectPredCycle ( pred, n, i ) == 0
                    || detectRootCycle ( root, n, i ) == 0 )
            {
                printf ( "Ciclo detectado em %d!\n", i );
                fprintf ( fp, "Ciclo detectado em %d!\n", i );
                exit ( 1 );
            }
            */
            fprintf ( fp, "%d:%d\n", i, pred[i]  );
        }
    }
    fclose (fp);

    //printf ( "\nNenhum ciclo detectado!\n" );

    //makeLabelMaps ( pred, root, label->val, n, imgOrig );

    fp = fopen ("root.txt", "w");
    for ( i = 0; i < n; i++ ) {
        fprintf ( fp, "%d:%d\n", i, root[i]  );
    }
    fclose (fp);

    fp = fopen ("cost.txt", "w");
    for ( i = 0; i < n; i++ ) {
        if ( cost->val[i] >= 0 )
            fprintf ( fp, "%d:%d ", i, cost->val[i]  );
    }
    fclose (fp);
    /*
    errNum = clEnqueueReadBuffer(   fila, 
        predBuffer, 
        CL_FALSE, 
        0, 
        sizeof(cl_int) * n, 
        pred,
        0, 
        NULL, 
        NULL    );
    checkErr ( errNum, "Reading pred" );

    clFinish ( fila );


 //   SetLabelIterative ( pred, label->val, n );
    printf ( "Labeled!\n" );

    */

    printf ( "Exiting from loop...\n" );
    printf ( "Time K0: %lf ms\n", run_time_k0 );
    if ( run_time_k1 )
        printf ( "Time K1: %lf ms\n", run_time_k1 );
    if ( run_time_k2 )
        printf ( "Time K2: %lf ms\n", run_time_k2 );
    if ( run_time_k3 )
        printf ( "Time K3: %lf ms\n", run_time_k3 );
    printf ( "Total Parallel Time: %lf ms\n", run_time_k3+run_time_k1+run_time_k0 );

    /*
    errNum = clEnqueueReadBuffer(   fila, 
        imgValBuffer, 
        CL_TRUE, 
        0, 
        sizeof(cl_int) * n, 
        F->V->val,
        0, 
        NULL, 
        NULL    );

    checkErr ( errNum, "Reading imgVal" );
    errNum = clEnqueueReadBuffer(   fila, 
        costBuffer, 
        CL_TRUE, 
        0, 
        sizeof(cl_int) * n, 
        F->R->val,
        0, 
        NULL, 
        NULL    );

    checkErr ( errNum, "Reading cost" );
*/
    /*Para todo p
     *   q = p
     *      Enquanto P(q) != NIL
     *             q = P(q)
     *                L(p) = L(q).*/
/*
                int l ;

for ( p = 0; p < n; p++ )
{
    q = p;
    i = 0;
    //while ( pred[q] != -1 && i < n )
    while ( pred[q] != -1 )
    {
        q = pred[q];
        i++;
    }
    label->val[p] = label->val[q];

}
    while (!EmptyGQueue(Q)){
        p   = RemoveGQueue(Q);
        u.x = p%img->ncols;
        u.y = p/img->ncols;
        for (i=1; i < A->n; i++) {
            v.x = u.x + A->dx[i];
            v.y = u.y + A->dy[i];
            if (ValidPixel(img,v.x,v.y)){
                q   = v.x + img->tbrow[v.y];
                l = label->val[p];
                if ( pred[q] == p && label->val [q] != l )
                {
                    label->val[q]=l;
                InsertGQueue(&Q,q);
                }
            }
        }

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

  fprintf(stdout,"Processing time in %f ms\n",CTime(t1,t2));

  DestroyGQueue(&Q);
  DestroyImage(&cost);
  DestroyAdjRel(&A);

  return(label);
}

int main(int argc, char **argv)
{
  Image    *img=NULL,*grad=NULL;
  Image    *label=NULL;
  CImage   *cimg=NULL;
  Set      *Obj=NULL,*Bkg=NULL;
  char     outfile[100];
  char     *file_noext;
  /*--------------------------------------------------------*/

  void *trash = malloc(1);
  struct mallinfo info;
  int MemDinInicial, MemDinFinal;
  free(trash);
  info = mallinfo();
  MemDinInicial = info.uordblks;

  /*--------------------------------------------------------*/

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


  label = Watershed(grad,Obj,Bkg, img);


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
