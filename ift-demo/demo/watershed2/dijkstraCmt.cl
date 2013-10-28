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
        __global int *An,
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *Ccostval,
        __global int *Ucostval,
        __global int *Ulval,
        /*13*/  __global int *sem,
        __global int *extra
        //		__global int *numV
        ) {
    int tid = get_global_id(0);
    Pixel u, v;
    int tmp;

    int i, q=0;

    if ( M [tid] ) {
        GetSemaphor(sem);
        M[tid] = false;

        u.x = tid % img->ncols;
        u.y = tid / img->ncols;

        for (i=1; i < *An; i++) {
            // Finding neighbors of u 
            /*
             */
            v.x = u.x + dx[i];
            v.y = u.y + dy[i];
            // If u is adjacent to v (into the image limits) 
            if (ValidPixel(img,v.x,v.y)) {
                // Now q has the spel form of the pixel v 
                q   = v.x + itbrow[v.y];


                barrier ( CLK_GLOBAL_MEM_FENCE );
                //if (cval[tid] < cval[q]){
                if (Ucostval[tid] < Ucostval[q]) {
                    //tmp = MAX(cval[tid] , ival[q]);
                    tmp = atom_max(&Ucostval[tid] , ival[q]);
                    if (tmp < Ucostval[q]) {
                        //if (tmp < cval[q]){
                        //if (cval[q]!=INT_MAX)
                        if (Ucostval[q]!=INT_MAX)
                            //       RemoveGQueueElem(Q,q);
                            //M[q] = false;
                            atom_xchg(&M[q], false);
                        //Ccostval[q] = tmp;
                        atom_xchg(&Ucostval[q], tmp);
                        //cval[q] = tmp;

                        // TODO: Verify if label array will
                        // require a Update and Cost arrays
                        //M[q] = true;
                        atom_xchg(&M[q], true);
                        atom_xchg(&Ulval[q], Ulval[tid]);
                        //    InsertGQueue(&Q,q);
                    }
                    /*
                     */
                    }
                }
            }
            ReleaseSemaphor(sem);

            }
            //    M[tid] = ( v.x < img->ncols ) | ( v.y < img->nrows ) ;
            /*
               M[tid] = *An;
             */
            /* Path propagation */
            /*

               while (!EmptyGQueue(Q)){
               tid   = RemoveGQueue(Q);
               u.x = tid%img->ncols;
               u.y = tid/img->ncols;
               for (i=1; i < An; i++) {
               v.x = u.x + dx[i];
               v.y = u.y + dy[i];
               if (ValidPixel(img,v.x,v.y)){
               q   = v.x + img->tbrow[v.y];
               if (cost->val[tid] < cost->val[q]){

               tmp = MAX(cost->val[tid] , img->val[q]);
               if (tmp < cost->val[q]){
               if (cost->val[q]!=INT_MAX)
               RemoveGQueueElem(Q,q);
               cost->val[q] =tmp;
               label->val[q]=label->val[tid];
               InsertGQueue(&Q,q);
               }
               }

               }
               }
               }

               barrier(CLK_GLOBAL_MEM_FENCE);
               if (v.x + itbrow[v.y] > *extra) {
             *extra = v.x + itbrow[v.y];
             }
             */
        }

