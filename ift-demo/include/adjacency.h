#ifndef _ADJACENCY_H_
#define _ADJACENCY_H_

#include "image.h"

typedef struct _adjrel {
  cl_int *dx;
  cl_int *dy;
  cl_int n;
} AdjRel;

typedef struct _adjpxl {
  cl_int *dp;
  cl_int n;
} AdjPxl;


/* All adjacency relations must place the central pixel at first. The
   central pixel must be skipped when visiting the neighbors during
   the IFT. It must be considered in the other operations. */

AdjRel *CreateAdjRel(int n);
void    DestroyAdjRel(AdjRel **A);
AdjRel *CloneAdjRel(AdjRel *A);

Image  *AdjRel2Image(AdjRel *A);

AdjRel *RightSide(AdjRel *A);
AdjRel *LeftSide(AdjRel *A);
AdjRel *RightSide2(AdjRel *A, float r);
AdjRel *LeftSide2(AdjRel *A, float r);
AdjRel *Circular(float r);
AdjRel *FastCircular(float r);

AdjPxl *AdjPixels(Image *img,AdjRel *A);
void    DestroyAdjPxl(AdjPxl **N);
int     FrameSize(AdjRel *A);
AdjRel *ComplAdj(AdjRel *A1, AdjRel *A2);
AdjRel *Horizontal(cl_int r);
AdjRel *Vertical(cl_int r);
AdjRel *Box(cl_int ncols, cl_int nrows);
AdjRel *ShearedBox(cl_int xsize, cl_int ysize, float Si, float Sj);
AdjRel *Ring(float inner_radius, float outer_radius);
AdjRel *KAdjacency(); // for image compression

AdjRel *Cross(cl_int ncols, cl_int nrows); 

#endif
