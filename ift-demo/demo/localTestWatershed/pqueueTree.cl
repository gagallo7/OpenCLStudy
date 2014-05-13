#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

#define counter M[200000]

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
    int occupied = atomic_xchg (semaphor, 1);
    while(occupied > 0)
    {
        occupied = atomic_xchg (semaphor, 1);
    }
}


void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atomic_xchg (semaphor, 0);
}

void getMin ( __global int* Mask, int i, __global int *Cmin, int someVal )
{
    if ( !Mask[i] || i > (1 << 18) )
    {
        return;
    }

    if ( someVal > Mask[i] )
    {
        atomic_xchg ( Cmin, Mask[i] );
        //atomic_xchg ( Cmin, Mask[i] );
    }
}

__kernel void pqueueOld (
        __global ImageIFT *img,
        __global int *ival,
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *cost,
        __global int *pred,
        __global int *imgVal,
        __global int *sem,
        __global int *extra,
        __global int *Clval,
        __global int16 *cache,
        __global int *Cmin
        )
{
    int tid = get_global_id(0);
    int i, q;
    int tmp;
    int16 q8;
    int qarray [8], imgValArray [8];
    Pixel u, v;

    if ( M[tid] == Cmin[0] )
    {
        M [tid] = 0;
        q8 = cache[tid];
        qarray [0] = q8.s0;
        qarray [1] = q8.s1;
        qarray [2] = q8.s2;
        qarray [3] = q8.s3;
        qarray [4] = q8.s4;
        qarray [5] = q8.s5;
        qarray [6] = q8.s6;
        qarray [7] = q8.s7;
        imgValArray [0] = q8.s8;
        imgValArray [1] = q8.s9;
        imgValArray [2] = q8.sA;
        imgValArray [3] = q8.sB;
        imgValArray [4] = q8.sC;
        imgValArray [5] = q8.sD;
        imgValArray [6] = q8.sE;
        imgValArray [7] = q8.sF;

        for ( i = 0; i < 8; i++ )
        {
            q = qarray[i];
            if ( q >= 0 ) 
            {
                if ( cost [ tid ] < cost [ q ] )
                {

                    tmp = max ( cost [ tid ], imgValArray [ i ]);

                    if ( tmp < cost [ q ] )
                    {
                        //atomic_min ( extra, tmp );
                        cost [ q ] = tmp;
                        //                        atomic_xchg ( &pred[q], tid );
                        //                            atomic_xchg ( &Clval[q], Clval[tid] );
                        Clval [ q ] = Clval [ tid ];
                        if ( !M [q] || M[q] > tmp )
                        {
                            M [ q ] = tmp;
                        }
                    }
                }
            }
        }
    }

}


__kernel void pqueue (
        __global ImageIFT *img,
        __global int *ival,
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *cost,
        __global int *pred,
        __global int *imgVal,
        __global int *sem,
        __global int *extra,
        __global int *label,
        __global int16 *cache,
        __global int *Cmin,
        __global int *root,
        __global volatile int *rwLocks
        )
{
    int tid = get_global_id(0);
    int i, q, cq;
    int tmp;
    int16 q8;
    int qarray [8], imgValArray [8];
    int R;

    if ( M [tid] == 1 && cost[tid] == Cmin[0] )
    {
        M [tid] = 0;
        R = root [ tid ];
        q8 = cache[tid];
        qarray [0] = q8.s0;
        qarray [1] = q8.s1;
        qarray [2] = q8.s2;
        qarray [3] = q8.s3;
        qarray [4] = q8.s4;
        qarray [5] = q8.s5;
        qarray [6] = q8.s6;
        qarray [7] = q8.s7;
        imgValArray [0] = q8.s8;
        imgValArray [1] = q8.s9;
        imgValArray [2] = q8.sA;
        imgValArray [3] = q8.sB;
        imgValArray [4] = q8.sC;
        imgValArray [5] = q8.sD;
        imgValArray [6] = q8.sE;
        imgValArray [7] = q8.sF;

        for ( i = 0; i < 8; i++ )
        {
            q = qarray[i];
            if ( q >= 0 ) 
            {
                if ( cost [ tid ] < cost [ q ] )
                {
                    cq = cost [ q ];
                    tmp = max ( cost [ tid ], imgValArray [ i ]);

                    //if ( !mq || mq > tmp )
                    if ( cq > tmp )
                    {
               //         cost [ q ] = tmp;
                        GetSemaphor ( & rwLocks [ q ] );
                        label [ q ] = label [ tid ];
                        M [ q ] = 1;
                        cost [q] = tmp;
                        pred [q] = tid;
                        root [ q ] = R;
                        /*
                        atom_xchg ( &pred[q], tid );
                        atom_xchg ( &root[q], R );
                        */
                        ReleaseSemaphor ( & rwLocks [ q ] );
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
    int qarray [8], imgValArray [8];
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
    imgValArray [0] = q8.s8;
    imgValArray [1] = q8.s9;
    imgValArray [2] = q8.sA;
    imgValArray [3] = q8.sB;
    imgValArray [4] = q8.sC;
    imgValArray [5] = q8.sD;
    imgValArray [6] = q8.sE;
    imgValArray [7] = q8.sF;

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
                //                        atomic_xchg ( &pred[q], tid );
                atomic_xchg ( &Clval[q], Clval[tid] );
                // Marking changes
                M [ q ] = true;
            }
        }
    }

}

__kernel void dijkstra3 (
        __global int4 *M,
        __global int *sem,
        __global int *extra,
        __global int *Cmin,
        __global int *matches,
        __global int4 *cost,
        __global int *gmin
        )
{
    int tid = get_global_id(0);
    int4 vals = cost [tid];
    int mininum = 1 << 30;
    int4 Mtid = M [tid];
    __local int lmin [1];

    int m = Mtid.x | Mtid.y | Mtid.w | Mtid.z;

    if ( Mtid.x )
    mininum = min ( mininum, vals.x );
    if ( Mtid.y )
    mininum = min ( mininum, vals.y );
    if ( Mtid.z )
    mininum = min ( mininum, vals.z );
    if ( Mtid.w )
    mininum = min ( mininum, vals.w );

    if ( !tid )
    {
        lmin[0] = 1 << 30;
    }
    barrier ( CLK_LOCAL_MEM_FENCE );
//    if ( M[tid] >= 0 && M[tid] < 495 )
    if ( m )
    {
        (void) atom_min ( lmin, mininum );

        /*
        if ( mininum == *Cmin )
        {
            atomic_xchg ( extra, *Cmin );
            //atomic_inc ( matches );
            return;
        }
        else if ( mininum > 0 )
        {
            atomic_min ( extra, mininum );
        }
        */
    }
    barrier ( CLK_LOCAL_MEM_FENCE );

    if ( !tid )
    {
        gmin [ get_group_id ( 0 ) ] = lmin [0];
    }

}

__kernel void min2global ( __global int * gmin )
{
    for (int n = 0; n < get_global_size(0); n++) {                                    
        barrier(CLK_GLOBAL_MEM_FENCE);                                                  
        if (get_global_id(0) == n) gmin[0] = min(gmin[0],gmin[n]);                      
    }                                                                                 

    barrier(CLK_GLOBAL_MEM_FENCE);        
}
