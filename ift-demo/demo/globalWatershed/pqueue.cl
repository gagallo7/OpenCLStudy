/*
 * Watershed OpenCL implementation 
 * 2014
 * Guilherme Alcarde Gallo, Guido Araújo and Alexandre Xavier Falcão
 *
 * GPLv3 License
 *
 * This file is a part from a implementation of IFT.
 * This is the kernel file.
 * The host file is watershed.c
*/

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

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
    int globalId = get_global_id(0);
    int i, q, cq;
    int tmp;
    int16 neighboorhood;
    int neighboor [8], imgValArray [8];

    // If pixel globalId is in queue
    // and has a minimum cost
    if ( M [globalId] == 1 && cost[globalId] == Cmin[0] )
    {
        // globalId is dequeued
        M [globalId] = 0;

        // fetching neighboorhood from cache
        neighboorhood = cache[globalId];
        neighboor [0] = neighboorhood.s0;
        neighboor [1] = neighboorhood.s1;
        neighboor [2] = neighboorhood.s2;
        neighboor [3] = neighboorhood.s3;
        neighboor [4] = neighboorhood.s4;
        neighboor [5] = neighboorhood.s5;
        neighboor [6] = neighboorhood.s6;
        neighboor [7] = neighboorhood.s7;
        imgValArray [0] = neighboorhood.s8;
        imgValArray [1] = neighboorhood.s9;
        imgValArray [2] = neighboorhood.sA;
        imgValArray [3] = neighboorhood.sB;
        imgValArray [4] = neighboorhood.sC;
        imgValArray [5] = neighboorhood.sD;
        imgValArray [6] = neighboorhood.sE;
        imgValArray [7] = neighboorhood.sF;

        for ( i = 0; i < 8; i++ )
        {
            // neighboor i from globalId
            q = neighboor[i];

            // if q is a valid pixel
            if ( q >= 0 ) 
            {
                // fmax function
                if ( cost [ globalId ] < cost [ q ] )
                {
                    tmp = max ( cost [ globalId ], imgValArray [ i ]);

                    if ( cost [ q ] > tmp )
                    {
                        // Conquering the neighboor
                        // rwLocks is a reading/write lock
                        // used to protect this critical
                        // region
                        GetSemaphor ( & rwLocks [ q ] );
                        label [ q ] = label [ globalId ];
                        M [ q ] = 1;
                        cost [q] = tmp;
                        pred [q] = globalId;
                        root [ q ] = root [ globalId ];
                        ReleaseSemaphor ( & rwLocks [ q ] );
                    }
                }
            }
        }
    }
}

// Not used
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

// Calculates the current minimum cost of the image
__kernel void dijkstra3 (
        __global int *M,
        __global int *sem,
        __global int *extra,
        __global int *Cmin,
        __global int *matches,
        __global int *cost
        )
{
    // Thread id
    int tid = get_global_id(0);

    // if tid is in queue
    if ( M[tid] )
    {
        // if Cmin doesn't change
        if ( cost[tid] == *Cmin )
        {
            atomic_xchg ( extra, *Cmin );
            //atomic_inc ( matches ); // couting the min-elements in parallel
            return;
        }

        else if ( cost [tid] > 0 )
        {
            atomic_min ( extra, cost[tid] );
        }
    }
}
