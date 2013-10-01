#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

typedef struct _adjrel {
    int *dx;
    int *dy;
    int n;
} AdjRel;

typedef struct _adjpxl {
    int *dp;
    int n;
} AdjPxl;

typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int ncols,nrows;
    int *tbrow;
} ImageIFT;

bool ValidPixel(__global ImageIFT *img, int x, int y)
{
    if ((x >= 0)&&(x < img->ncols)&&
            (y >= 0)&&(y < img->nrows))
        return(true);
    else
        return(false);
}

void GetSemaphor(__global int * semaphor) {
    int occupied = atom_xchg (semaphor, 1);
    while(occupied > 0)
    {
        occupied = atom_xchg (semaphor, 1);
    }
}


void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atom_xchg (semaphor, 0);
}


__kernel void dijkstra (
        __global ImageIFT *img,
        __global int *ival,
        __global int *itbrow,
        __global ImageIFT *cost,
        __global int *cval,
        __global ImageIFT *label,
        __global int *lval,
        __global AdjRel *A,
        __global int *M,
        __global int *C,
        __global int *U,
/*10*/  __global int *sem//,
        //		__global int *numV
        ) {
    int tid = get_global_id(0);
    Pixel u, v;
    int tmp;

    int i, q, p;
    if ( M [tid] ) {
        M[tid] = false;
        //u.x = tid % img->ncols;
        //u.y = tid / img->ncols;
        u.x = tid % img->ncols;
        u.y = tid / img->ncols;
        for (i=1; i < A->n; i++) {
            // Finding neighbors of u 
            v.x = u.x + A->dx[i];
            v.y = u.y + A->dy[i];
            // If u is adjacent to v (into the image limits) 
            if (ValidPixel(img,v.x,v.y)){
                //* Now q has the spel form of the pixel v *
                q   = v.x + itbrow[v.y];
                if (cval[p] < cval[q]){

                    tmp = MAX(cval[p] , ival[q]);
                    if (tmp < cval[q]){
                        if (cval[q]!=INT_MAX)
                            //       RemoveGQueueElem(Q,q);
                            M[q] = false;
                        cval[q] =tmp;
                        lval[q]=lval[p];
                        M[q] = true;
                        //    InsertGQueue(&Q,q);
                    }
                }

            }
        }

    }
}

