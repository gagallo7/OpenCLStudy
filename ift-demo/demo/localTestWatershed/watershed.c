/*
 * Watershed OpenCL implementation 
 * 2014
 * Guilherme Alcarde Gallo, Guido Araújo and Alexandre Xavier Falcão
 *
 * GPLv3 License
 *
 * This file is a part from a blockage implementation of IFT.
 * This is the host file.
 * The kernel file is pqueue.cl
*/

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

/*
 * This function detects if a cycle exists in the root
 * map generated from IFT for debugging and correctness
 * avaliation
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

    if ( i == n )
    {
        printf ( "There is a cycle in the pixel %i\n", search );
        return 0;
    }
    
    return 1;
}

/*
 * This function detects if a cycle exists in the predecessor
 * map generated from IFT for debugging and correctness
 * avaliation
*/
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

/*
 * This function has an output img which is the
 * image that represents the the label maps
 * made with predecessor and root maps
 */
void makeLabelMaps ( cl_int* pred, cl_int* root, cl_int* label, int n, Image* img )
{
    int i, p, k;

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
        k = 0;
        while ( pred [ p ] != NIL  )
        {
            if ( k == n )
            {
                printf ( "Cycle detected! Map isn't created!\n" );
                return;
            }
            k++;
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


/* Tests if a queue's mask is empty */
    
cl_int empty (cl_int Mask[], cl_int n) {
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
    cl_uint nPlatforms;
    cl_uint nDevices;
    cl_platform_id *platformListID;
    cl_device_id *deviceListID;
    cl_context contexto = NULL;
    cl_command_queue fila;
    cl_program programa0, programa, programa2, programa3;
    cl_kernel kernel0, kernel, kernel2, kernel3;
    cl_event evento, evento0, evento1, evento2, evento3;

    cl_mem  costBuffer, costvalBuffer, costtbrowBuffer,
            numVbuffer, SEMaskBuffer, extraBuffer,
            Abuffer, Adxbuffer, Adybuffer,
            imgtbrowBuffer, labelvalBuffer, labeltbrowBuffer,
            MaskBuffer, MvalBuffer, MtbrowBuffer,
            /*
            Ubuffer, Ulabelbuffer,
            Cbuffer, Clabelbuffer,
            UPredbuffer, CPredbuffer,
            */
            IBuffer, IvalBuffer, imgValBuffer,
            labelBuffer, predBuffer,
            cacheBuffer, cache2Buffer, cache3Buffer,
            CminBuffer, matchesBuffer, rootBuffer,
            rwLocksBuffer,
            CminBlockBuffer, MblockBuffer;


    /*--------------------------------------------------------*/

    /* Preparing device for OpenCL program */
    errNum = clGetPlatformIDs(0, NULL, &nPlatforms);
    checkErr( (errNum != CL_SUCCESS) ? errNum : 
            (nPlatforms <= 0 ? -1 : CL_SUCCESS),
            "clGetPlataformsIDs");
    // If there is no error, cl_platform_id is allocated in memory
    platformListID = (cl_platform_id *)alloca(sizeof(cl_platform_id)*nPlatforms);
    // Attributing the platform to the platformListID
    errNum = clGetPlatformIDs(nPlatforms, platformListID, NULL);
    printf ( "#Platforms: %d\n", nPlatforms );
    checkErr(
            (errNum != CL_SUCCESS) ? errNum :
            (nPlatforms <= 0 ? -1 : CL_SUCCESS),
            "clGetPlatformIDs");

    cl_uint j;
    for (j=0; j < nPlatforms; j++) {
        // Attributing the number of GPU devices to nDevices
        errNum = clGetDeviceIDs (	platformListID[j],
                //								CL_DEVICE_TYPE_ALL,
                DEVICE,
                0,
                NULL,
                &nDevices		);
        if (errNum != CL_SUCCESS && errNum != CL_DEVICE_NOT_FOUND) {
            infoPlataforma(platformListID, j);
            checkErr (errNum, "clGetDeviceIDs");
        }
        // Checking if there are CPU devices
        else if (nDevices > 0) {
            // Attributing the device to the deviceListID
            deviceListID = (cl_device_id *)alloca(sizeof(cl_device_id)*nDevices);

            errNum = clGetDeviceIDs (	
                    platformListID[j],
                    //									CL_DEVICE_TYPE_ALL,
                    DEVICE, // defined in a macro/define
                    nDevices,
                    &deviceListID[0],
                    NULL);
            checkErr(errNum, "clGetPlatformIDs");
            break;
        }
    }

    // Creating a context on the selected device and platform
    printf ( " Adding OpenCL devices -- number %d\n", j );
    infoPlataforma(platformListID, j);
    cl_context_properties propContexto[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platformListID[j],
        0
    };

    contexto = clCreateContext(
            propContexto,
            nDevices,
            deviceListID,
            &contextCallback,
            NULL,
            &errNum			);

    checkErr(errNum, "clCreateContext");

    // Opening the source code from kernel
    fp = fopen("init.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)alloca(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // The program is created from source
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
            nDevices,
            deviceListID,
            //"-cl-fast-relaxed-math -cl-mad-enable",
            // Optimisations disabled
            "-cl-opt-disable",
            NULL,
            NULL);

    printf ( "kernel0 compiled!          \n" );
    if (errNum != CL_SUCCESS) { 		// Verifying if there is an error
        // Catching the log of the error
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa0,
                deviceListID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);

        printf ( " Build error : %s\n", logCompilacao );

        checkErr(errNum, "clBuildProgram");
    }

    printf ( "Compiling the kernel 1 ... \r" );
    errNum = clBuildProgram (
            programa,
            nDevices,
            deviceListID,
            // Setting the warnings flags and some optimizations
            "-cl-fast-relaxed-math -w -Werror -cl-mad-enable",
            NULL,
            NULL);

    printf ( "kernel1 compiled!         \n" );

    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa,
                deviceListID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);

        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }

    errNum = clBuildProgram (
            programa3,
            nDevices,
            deviceListID,
            NULL,
            NULL,
            NULL);

    printf ( "kernel3 compiled! \n" );

    if (errNum != CL_SUCCESS) { 		// Verificando se houve erro
        char logCompilacao[16384];
        clGetProgramBuildInfo (
                programa3,
                deviceListID[0],
                CL_PROGRAM_BUILD_LOG,
                sizeof(logCompilacao),
                logCompilacao,
                NULL);
        printf ( " Build error : %s\n", logCompilacao );
        checkErr(errNum, "clBuildProgram");
    }

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

    // N is the next power of 2 number greater or equal than n
    int N = 0;
    float F = n;
    int t = 0;
    while ( F > 1 ) {
        F /= 2;
        t++;
    }
    N = 1 << t;

    printf ("n=%d ... N=%d (2^%d)## ncols=%d\n", n, N, t, img->ncols);

    // Defining the number of global and local work-items
    const size_t globalWorkSize[1] = { N };
    const size_t localWorkSize[1] = { 1024 };
    const size_t globalWorkSize2[2] = { N , 8 };
    const size_t localWorkSize2[2] = { 1024, 1 };

    // numBlocks is the number of blocks generated by the
    // global and local work size
    size_t numBlocks = globalWorkSize [ 0 ] / localWorkSize [ 0 ];

    cl_int* Mask = (cl_int *) calloc (N, sizeof ( cl_int ) );
    cl_int* pred = (cl_int *) calloc (N, sizeof ( cl_int ) );
    cl_int* root = (cl_int *) malloc (N * sizeof ( cl_int ) );
    cl_int * Mblocks = ( cl_int * ) calloc ( numBlocks, sizeof ( cl_int ) );
    static volatile cl_int semaforo = 0;
    volatile cl_int* rwLocks = ( volatile cl_int *) calloc ( n, sizeof ( volatile cl_int ) );
    cl_int extra = Cmax;
    cl_int16* cache = (cl_int16 *) malloc ( sizeof (cl_int16) * N );



  /* Trivial path initialization */

    for (p=0; p < n; p++){
        cost->val[p] =INT_MAX;
        // the predecessor has an invalid value
        pred[p] = -2;
    }

    // S represents the Seeds from file (Obj and Bkg)
    S = Obj;
    while(S != NULL){
        p=S->elem;
        label->val[p]=1;
        cost->val[p]=0;
        pred[p] = NIL;
        //InsertGQueue(&Q,p);
        // p is enqueued
        Mask [ p ] = 1;
        Mblocks [ p / localWorkSize [0] ] = 1;
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
        // p is enqueued
        Mask [ p ] = 1;
        Mblocks [ p / localWorkSize [0] ] = 1;
        //InsertGQueue(&Q,p);
        S = S->next;
    }

    printf ( "Block occupancy\n" );
    for ( i = 0; i < (globalWorkSize [0] / localWorkSize [0] ); i++ )
    {
        printf ( "%d ", Mblocks [ i ] );
    }
    printf ( "\n" );

    int Cmin = 0;
    int matches = 0;

    /* Creating Buffers */
    MblockBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            ( globalWorkSize [0] / localWorkSize [ 0 ] ) * sizeof(cl_int),
            Mblocks,
            &errNum);
    checkErr(errNum, "clCreateBuffer(Mblock)");

    /*
    CminBlockBuffer = clCreateBuffer (
            contexto,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            //Cmax+1,
            ( globalWorkSize [0] / localWorkSize [ 0 ] ) * sizeof(cl_int),
            CminBlock,
            &errNum);
    checkErr(errNum, "clCreateBuffer(CminBlock)");
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

    MaskBuffer = clCreateBuffer (
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

    SEMaskBuffer = clCreateBuffer (
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
            deviceListID[0],
            CL_QUEUE_PROFILING_ENABLE,
            &errNum);
    checkErr(errNum, "clCreateCommandQueue");

    // Setting kernel arguments
    errNum = clSetKernelArg(kernel0, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel0, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel0, 2, sizeof(cl_mem), &imgtbrowBuffer);
    errNum |= clSetKernelArg(kernel0, 3, sizeof(cl_mem), &Abuffer);
    errNum |= clSetKernelArg(kernel0, 4, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel0, 5, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel0, 6, sizeof(cl_mem), &SEMaskBuffer);
    errNum |= clSetKernelArg(kernel0, 7, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel0, 8, sizeof(cl_mem), &cacheBuffer);
    /*
    errNum |= clSetKernelArg(kernel0, 9, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel0, 10, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 0");

    // Kernel 1: Dijkstra' kernel
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &MblockBuffer);
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &MaskBuffer);
    errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &costBuffer);
    errNum |= clSetKernelArg(kernel, 6, sizeof(cl_mem), &predBuffer);
    errNum |= clSetKernelArg(kernel, 7, sizeof(cl_mem), &imgValBuffer);
    //errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel, 8, sizeof(cl_mem), &SEMaskBuffer);
    errNum |= clSetKernelArg(kernel, 9, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &labelBuffer);
    errNum |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &cacheBuffer);
    errNum |= clSetKernelArg(kernel, 12, sizeof(cl_mem), &CminBuffer);
    errNum |= clSetKernelArg(kernel, 13, sizeof(cl_mem), &rootBuffer);
    errNum |= clSetKernelArg(kernel, 14, sizeof(cl_mem), &rwLocksBuffer);
    /*
    errNum |= clSetKernelArg(kernel, 11, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel, 12, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 1");

    errNum = clSetKernelArg(kernel2, 0, sizeof(cl_mem), &IBuffer);
    errNum |= clSetKernelArg(kernel2, 1, sizeof(cl_mem), &IvalBuffer);
    errNum |= clSetKernelArg(kernel2, 2, sizeof(cl_mem), &Adxbuffer);
    errNum |= clSetKernelArg(kernel2, 3, sizeof(cl_mem), &Adybuffer);
    errNum |= clSetKernelArg(kernel2, 4, sizeof(cl_mem), &MaskBuffer);
    errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &costBuffer);
    errNum |= clSetKernelArg(kernel2, 6, sizeof(cl_mem), &predBuffer);
    errNum |= clSetKernelArg(kernel2, 7, sizeof(cl_mem), &imgValBuffer);
    //errNum |= clSetKernelArg(kernel2, 5, sizeof(cl_mem), &CPredbuffer);
    errNum |= clSetKernelArg(kernel2, 8, sizeof(cl_mem), &SEMaskBuffer);
    errNum |= clSetKernelArg(kernel2, 9, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel2, 10, sizeof(cl_mem), &labelBuffer);
    errNum |= clSetKernelArg(kernel2, 11, sizeof(cl_mem), &cacheBuffer);
    /*
    errNum |= clSetKernelArg(kernel2, 11, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel2, 12, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 2");


    errNum = clSetKernelArg(kernel3, 0, sizeof(cl_mem), &MaskBuffer);
    errNum |= clSetKernelArg(kernel3, 1, sizeof(cl_mem), &SEMaskBuffer);
    errNum |= clSetKernelArg(kernel3, 2, sizeof(cl_mem), &extraBuffer);
    errNum |= clSetKernelArg(kernel3, 3, sizeof(cl_mem), &CminBuffer);
    errNum |= clSetKernelArg(kernel3, 4, sizeof(cl_mem), &matchesBuffer);
    errNum |= clSetKernelArg(kernel3, 5, sizeof(cl_mem), &costBuffer);
    /*
    errNum |= clSetKernelArg(kernel2, 11, sizeof(cl_mem), &input_image);
    errNum |= clSetKernelArg(kernel2, 12, sizeof(cl_sampler), &sampler);
    */
    checkErr(errNum, "clSetKernelArg at Kernel 3");

    cl_int vez = 0, nqueue = 0;
    double run_time_k0 = 0;
    double run_time_k1 = 0;
    double run_time_k2 = 0;
    double run_time_k3 = 0;
    cl_ulong ev_start_time=(cl_ulong)0;     
    cl_ulong ev_end_time=(cl_ulong)0;
    gettimeofday(tS1, NULL);

    // releituraFeita is an event that synchronizes the queue
    // modifications made by kernel 1
    cl_event releituraFeita;
    errNum = clEnqueueReadBuffer(fila, MaskBuffer, CL_FALSE, 0, 
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

    // Counting the kernel execution time
    run_time_k0 += (double)(ev_end_time - ev_start_time)/1e6; // in msec

    /*
    for ( i = 0; i < n; i++ ) {
        if ( pred[i] >= 0 )
            printf ( "%d ", pred[i] );
    }
    */

    printf ( "\n\nEntering in loop...\n" );

    if ( !empty ( Mask, n ) )
    {
        /*
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
           extraBuffer, 
           CL_TRUE, 
           0, 
           sizeof(cl_int), 
           &extra,
           0, 
           NULL, 
           NULL    );
           checkErr ( errNum, "Reading extra" );
           */

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
                MaskBuffer, 
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

        // Counting pixels that can be extracted from queue
        // and how many pixels is in the queue
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
         * Updating extra
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
        checkErr ( errNum, "Reading Cmin" );


        // This loop intends to optimize the memory transfers
        // between multiple calls of kernel 1
        // A pixel that is just conquered/enqueued may has that same 
        // cost than Cmin
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

        errNum = clEnqueueReadBuffer(   
                fila, 
                MblockBuffer, 
                CL_TRUE, 
                0, 
                numBlocks * sizeof(cl_int), 
                Mblocks,
                0, 
                NULL, 
                NULL    );
        checkErr ( errNum, "Reading Mblocks" );

        /*
        for ( i = 0; i < numBlocks; i++ )
        {
            printf ( "MaskBlock[%d] = %d\n", i, Mblocks[i] );
        }
        */

        // Reseting matches
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

        // extra will be minimized until it reaches 
        // the new Cmin value
        extra = INT_MAX;
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

        // Kernel 3's call
        /*
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
           */

        //printf ( "\n--------Cmin NEW = %d  Extra COnf %d-----------\n", Cmin, extra );

        /*
           errNum = clEnqueueReadBuffer(fila, MaskBuffer, CL_FALSE, 0, 
           sizeof(cl_int) * n, Mask, 0, NULL, &releituraFeita);
           checkErr(errNum, "Reading Mask");
           clWaitForEvents(1, &releituraFeita);
           Cmin = empty(Mask, n);
           */
    }


    // Correcting memory leaks

/*
    errNum = clReleaseMemObject ( MaskBuffer );
    errNum |= clReleaseMemObject ( cacheBuffer );
    errNum |= clReleaseMemObject ( IBuffer );
   
    checkErr(errNum, "clRelease");
*/


    // Debug file
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

    printf ( "\nReading buffer...\n" );
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
     * Debug file
    */
    fp = fopen ("label.txt", "w");
    for ( i = 0; i < n; i++ ) {
        fprintf ( fp, "%d ", label->val[i] );
    }
    fclose (fp);

    // Reading the image properties from GPU
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

            /*
    fp = fopen ("pred.txt", "w");
    for ( i = 0; i < n; i++ ) {
        if ( pred[i] >= 0 )
        {
            printf ( "Detecting cycle in %d...\r", i );
            if ( detectPredCycle ( pred, n, i ) == 0
                    || detectRootCycle ( root, n, i ) == 0 )
            {
                printf ( "Ciclo detectado em %d!\n", i );
                fprintf ( fp, "Ciclo detectado em %d!\n", i );
                exit ( 1 );
            }
            fprintf ( fp, "%d:%d\n", i, pred[i]  );
        }
    }
    fclose (fp);
            */

    printf ( "\nDetecting cycle\n" );
    makeLabelMaps ( pred, root, label->val, n, imgOrig );
    printf ( "No cycle detected!\n\n" );

    // Debug files
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

    // Printing times
    printf ( "Exiting from loop...\n" );
    printf ( "Time K0: %lf ms\n", run_time_k0 );
    if ( run_time_k1 )
        printf ( "Time K1: %lf ms\n", run_time_k1 );
    if ( run_time_k2 )
        printf ( "Time K2: %lf ms\n", run_time_k2 );
    if ( run_time_k3 )
        printf ( "Time K3: %lf ms\n", run_time_k3 );
    printf ( "Total Parallel Time: %lf ms\n", run_time_k3+run_time_k1+run_time_k0 );

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

  fprintf(stdout,"Processing time in %f ms\n",CTime(t1,t2));
*/

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
