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

typedef struct _figura {
  int *val;
  int ncols;
  int nrows;
  int *tbrow;
} figura;

bool ValidPixel(__global figura *img, int x, int y)
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
/*
*/

__kernel void dijkstra (
		__global int *img,
		__global int *cost,
		__global figura *label,
		__global AdjRel *A,
		__global int *M,
		__global int *C,
		__global int *U,
		__global int *sem//,
//		__global int *numV
		) {
	int tid = get_global_id(0);
    Pixel u, v;
    int tmp;


    int i, q, p;
    if ( M [tid] ) {
		M[tid] = false;
        /*
        u.x = tid % img->ncols;
        u.y = tid / img->ncols;
        for (i=1; i < A->n; i++) {
            // Finding neighbors of u 
            v.x = u.x + A->dx[i];
            v.y = u.y + A->dy[i];
            // If u is adjacent to v (into the image limits) 
            if (ValidPixel(img,v.x,v.y)){
                // Now q has the spel form of the pixel v
                q   = v.x + img->tbrow[v.y];
                if (cost->val[p] < cost->val[q]){

                    tmp = MAX(cost->val[p] , img->val[q]);
                    if (tmp < cost->val[q]){
                        if (cost->val[q]!=INT_MAX)
                     //       RemoveGQueueElem(Q,q);
                            M[q] = false;
                        cost->val[q] =tmp;
                        label->val[q]=label->val[p];
                        M[q] = true;
                    //    InsertGQueue(&Q,q);
                    }
                }

            }
        }*/

    }
}

