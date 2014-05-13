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
        __global int *cost,
        __global int *pred,
        __global int *imgVal,
        /*
           __global int *CostCost,
           __global int *UpdateCost,
           __global int *UpdateLabel,
           __global int *UpdatePred,
           __global int *CostPred,
         */
        __global int *sem,
        __global int *extra,
        __global int *Clval,
        __global int16 *cache)
{
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp, w;
    int16 q8;
    int qarray [8], carray [8];
    Pixel u, v;
    int ncols = img->ncols;
    int cc = 0;

    if ( M[tid] )
    {
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
        carray [0] = q8.s8;
        carray [1] = q8.s9;
        carray [2] = q8.sA;
        carray [3] = q8.sB;
        carray [4] = q8.sC;
        carray [5] = q8.sD;
        carray [6] = q8.sE;
        carray [7] = q8.sF;
        
        for ( i = 0; i < 8; i++ )
        {
            q = qarray[i];
            if ( q >= 0 ) 
            {
                if ( cost [ tid ] < cost [ q ] )
                {

                    tmp = max ( cost [ tid ], carray [ i ]);
 //                   tmp = max ( cost [ tid ], cost [ q ] );
//                    tmp = carray [i];

                    if ( tmp < cost [ q ] ||                             
                            ( 
                             tmp <= cost [ q ] &&
                             Clval [ q ] != Clval[tid] &&
                             pred [ q ] == tid 
                            ) 
                       )
                    {
                        cost [ q ] = tmp;
                        atom_xchg ( &pred[q], tid );
                        atom_xchg ( &Clval[q], Clval[tid] );
                        M [ q ] = true;
                    }
                }
            }
        }
    }

}

__kernel void dijkstra2 (
        __global ImageIFT *img,
        __global int *ival,
        //        __global int *itbrow,
        /*
           __global int *An,
         */
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *cost,
        __global int *pred,
        __global int *imgVal,
        /*
           __global int *CostCost,
           __global int *UpdateCost,
           __global int *UpdateLabel,
           __global int *UpdatePred,
           __global int *CostPred,
         */
        __global int *sem,
        __global int *extra,
        __global int *Clval,
        __global int16 *cache)
{
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp, w;
    int16 q8;
    int qarray [8], carray [8];
    Pixel u, v;
    int ncols = img->ncols;
    int cc = 0;

        q8 = cache[tid];
        qarray [0] = q8.s0;
        qarray [1] = q8.s1;
        qarray [2] = q8.s2;
        qarray [3] = q8.s3;
        qarray [4] = q8.s4;
        qarray [5] = q8.s5;
        qarray [6] = q8.s6;
        qarray [7] = q8.s7;
        carray [0] = q8.s8;
        carray [1] = q8.s9;
        carray [2] = q8.sA;
        carray [3] = q8.sB;
        carray [4] = q8.sC;
        carray [5] = q8.sD;
        carray [6] = q8.sE;
        carray [7] = q8.sF;
        
        for ( i = 0; i < 8; i++ )
        {
            q = qarray[i];
            if ( q >= 0 ) 
            {
                    if  ( 
                            Clval [ q ] != Clval[tid] &&
                            pred [ q ] == tid 
                        ) 
                    {
//                        atom_xchg ( &pred[q], tid );
                        atom_xchg ( &Clval[q], Clval[tid] );
                        // Marking changes
                        M [ q ] = true;
                    }
            }
        }

}

__kernel void dijkstra3 (
        __global ImageIFT *img,
        __global int *ival,
        //        __global int *itbrow,
        /*
           __global int *An,
         */
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *cost,
        __global int *pred,
        __global int *imgVal,
        /*
           __global int *CostCost,
           __global int *UpdateCost,
           __global int *UpdateLabel,
           __global int *UpdatePred,
           __global int *CostPred,
         */
        __global int *sem,
        __global int *extra,
        __global int *Clval,
        __global int16 *cache)
{
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp, w;
    int16 q8;
    int qarray [8], carray [8];
    Pixel u, v;
    int ncols = img->ncols;
    int cc = 0;

    if ( M [ tid ] ) {
        M [ tid ] = false;
        q8 = cache[tid];
        qarray [0] = q8.s0;
        qarray [1] = q8.s1;
        qarray [2] = q8.s2;
        qarray [3] = q8.s3;
        qarray [4] = q8.s4;
        qarray [5] = q8.s5;
        qarray [6] = q8.s6;
        qarray [7] = q8.s7;
        /*
        carray [0] = q8.s8;
        carray [1] = q8.s9;
        carray [2] = q8.sA;
        carray [3] = q8.sB;
        carray [4] = q8.sC;
        carray [5] = q8.sD;
        carray [6] = q8.sE;
        carray [7] = q8.sF;
        */
        
        for ( i = 0; i < 8; i++ )
        {
            q = qarray[i];
            if ( q >= 0 ) 
            {
                    if  ( 
                            Clval [ q ] != Clval[tid] &&
                            pred [ q ] == tid 
                        ) 
                    {
//                        atom_xchg ( &pred[q], tid );
                        atom_xchg ( &Clval[q], Clval[tid] );
                        // Marking changes
                        M [ q ] = true;
                    }
            }
        }
    }
}
