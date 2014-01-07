#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

int qq (int i, int8 I) {
    switch (i) {
        case 0:
            return I.s0;
        case 1:
            return I.s1;
        case 2:
            return I.s2;
        case 3:
            return I.s3;
        case 4:
            return I.s4;
        case 5:
            return I.s5;
        case 6:
            return I.s6;
        case 7:
            return I.s7;
    }
}

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
        __global int8 *cache)
{
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp;
    int8 q8;
    int qarray [8];
    Pixel u, w, v;
    int ncols = img->ncols;


    if ( M[tid] ) {
        u.x = tid % ncols;
        //u.y = tid / ncols;
        u.y = native_divide ( (float)tid, (float)ncols );
        w.x = FRval [ tid ] % ncols;
        //w.y = FRval [ tid ] / ncols;
        w.y = native_divide ( (float)FRval[tid], (float)ncols );

        M [tid] = false;
        q8 = cache[tid];
        qarray [0] = q8.s0;
        qarray [1] = q8.s1;
        qarray [2] = q8.s2;
        qarray [3] = q8.s3;
        qarray [4] = q8.s4;
        qarray [5] = q8.s5;
        qarray [6] = q8.s6;
        qarray [7] = q8.s7;

        for ( i = 0; i < 8; i++ ) {
            /*
            v.x = u.x + dx [i+1];
            v.y = u.y + dy [i+1];
            */
            v.x = u.x + dx [i+1] - w.x;
            v.y = u.y + dy [i+1] - w.y;
            //q = cache [ tid*8 + i ];
            q = qarray[i];
            if ( q >= 0 ) {
                // if (F->V->val[q]>F->V->val[p]) --- can be cached
                if ( FVval [ q ] > FVval [tid] ) {
                    tmp = v.x*v.x+ v.y*v.y;
                    /*
                    tmp = (v.x-w.x)*(v.x-w.x)+(v.y-w.y)*(v.y-w.y);
                    */
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
