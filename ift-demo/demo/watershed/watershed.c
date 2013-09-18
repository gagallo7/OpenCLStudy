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

bool vazio (unsigned int M[], int n) {
    int i;
	for (i = 0; i < n; i++) {
		if (M[i] == true)
			return false;
	}
	return true;
}

// Watershed from binary marker

Image *Watershed(Image *img, Set *Obj, Set *Bkg)
{
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

    size_t Asize;
    A     = Circular(1.5, &Asize);

    /* Trivial path initialization */

    /* TODO */
    /* Dijkstra mode */
    /* Ignore GQueue, create a normal queue 
     * remembering that mask and update are
     * required for the OpenCL version of
     * Dijkstra algorithm */

    /* For Kernel's Buffer */

    int* M, C, U = AllocIntArray(Cmax+1);
    int semaforo = 0;

	// Alocando Buffers
	Vbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int) * g.vert.size(),
            sizeof ( int ) * img->
			g.vert.data(),
			&errNum);
	checkErr(errNum, "clCreateBuffer(V)");

	Abuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			2*(A->n) + sizeof(A),
			&A,
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
			Cmax+1,
			&U,
			&errNum);
	checkErr(errNum, "clCreateBuffer(U)");

	Mbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			Cmax+1,
			&M,
			&errNum);
	checkErr(errNum, "clCreateBuffer(M)");

	Cbuffer = clCreateBuffer (
			contexto,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			Cmax+1,
			&C,
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
    for (p=0; p < n; p++){
        cost->val[p] =INT_MAX;
    }
    S = Obj;
    while(S != NULL){
        p=S->elem;
        label->val[p]=1;
        cost->val[p]=0;
        InsertGQueue(&Q,p);
        S = S->next;
    }
    S = Bkg;
    while(S != NULL){
        p=S->elem;
        label->val[p]=0;
        cost->val[p]=0;
        InsertGQueue(&Q,p);
        S = S->next;
    }

    /* Path propagation */

    while (!EmptyGQueue(Q)){
        p   = RemoveGQueue(Q);
        u.x = p%img->ncols;
        u.y = p/img->ncols;
        for (i=1; i < A->n; i++) {
            /* Finding neighbors of u */
            v.x = u.x + A->dx[i];
            v.y = u.y + A->dy[i];
            /* If u is adjacent to v (into the image limits) */
            if (ValidPixel(img,v.x,v.y)){
                /* Now q has the spel form of the pixel v */
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


    DestroyGQueue(&Q);
    DestroyImage(&cost);
    DestroyAdjRel(&A);

    return(label);
}

int main(int argc, char **argv)
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
    int MemDinInicial, MemDinFinal;
    free(trash);
    info = mallinfo();
    MemDinInicial = info.uordblks;

    /*--------------------------------------------------------*/
    /* OpenCL variables --------------------------------------*/
	cl_int errNum;
	cl_uint nPlataformas;
	cl_uint nDispositivos;
	cl_platform_id *listaPlataformaID;
	cl_device_id *listaDispositivoID;
	cl_context contexto = NULL;
	cl_command_queue fila;
	cl_program programa, programa2;
	cl_kernel kernel, kernel2;
    


    /*--------------------------------------------------------*/

    if (argc!=4){
        fprintf(stderr,"Usage: watershed <image.pgm> <gradient.pgm> <seeds.txt>\n");
        fprintf(stderr,"image.pgm: image to overlay the watershed lines on it\n");
        fprintf(stderr,"gradient.pgm: gradient image to compute the watershed segmentation\n");
        fprintf(stderr,"seeds.txt: seed pixels\n");
        exit(-1);
    }

    /* Preparing device for OpenCL program */
    //prepareAllDataForDevice(errNum, nPlataformas, nDispositivos, listaPlataformaID, listaDispositivoID, contexto, fila, programa, programa2, kernel, kernel2);
    prepareAllDataForDevice(errNum, nPlataformas, nDispositivos, &contexto, fila, programa, programa2, &kernel, &kernel2);

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
