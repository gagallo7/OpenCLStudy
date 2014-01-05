#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int *tbrow;
    int ncols,nrows;
} ImageIFT;

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
        //        __global int *itbrow,
        /*
           __global int *An,
         */
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *FRval,
        __global int *FPval,
        __global int *FVval,
        /*
           __global int *CostCost,
           __global int *UpdateCost,
           __global int *UpdateLabel,
           __global int *Clval,
           __global int *UpdatePred,
           __global int *CostPred,
         */
        __global int *sem,
        __global int *extra,
        __global int *cache)
{
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp;
    Pixel u, w, v;
    int ncols = img->ncols;


    if ( M[tid] ) {
        u.x = tid % ncols;
        u.y = tid / ncols;
        w.x = FRval [ tid ] % ncols;
        w.y = FRval [ tid ] / ncols;

        M [tid] = false;

        for ( i = 0; i < 8; i++ ) {
            v.x = u.x + dx [i+1];
            v.y = u.y + dy [i+1];
            q = cache [ tid*8 + i ];
            if ( q >= 0 ) {
                // if (F->V->val[q]>F->V->val[p]) --- can be cached
                if ( FVval [ q ] > FVval [tid] ) {
                    tmp = (v.x-w.x)*(v.x-w.x)+(v.y-w.y)*(v.y-w.y);
                    if ( tmp < FVval [ q ] ) {
                        if ( FVval [ q ] != INT_MAX ) 
                            M [ q ] = false;
                        FVval [ q ] = tmp;
                        FRval [ q ] = FRval [ tid ];
                        FPval [ q ] = tid;
                            M [ q ] = true;
                    }
                }
            }
        }
    }
}
