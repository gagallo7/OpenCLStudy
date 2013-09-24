#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "common.h"
#include "oclFunctions.h"

typedef struct _pixel {
  cl_int x,y;
} Pixel;

typedef struct _image {
  cl_int *val;
  cl_int ncols,nrows;
  cl_int *tbrow;
} Image;

Image  *CreateImage(cl_int ncols,cl_int nrows);
void    DestroyImage(Image **img);
Image  *ReadImage(char *filename);
void    WriteImage(Image *img, char *filename);
Image  *CopyImage(Image *img);

int     MinimumValue(Image *img);
int     MaximumValue(Image *img);
void    SetImage(Image *img, cl_int value);
bool    ValidPixel(Image *img, cl_int x, cl_int y);
Image  *AddFrame(Image *img, cl_int sz, cl_int value);
Image  *RemFrame(Image *fimg, cl_int sz);

Image *Threshold(Image *img, cl_int lower, cl_int higher);

#endif
