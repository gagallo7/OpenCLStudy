#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int32_extended_atomics : enable
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

__kernel void pqueue (
        __global ImageIFT *img,
        __global int *MaskOfTheBlock,
        __global int *dx,
        __global int *dy,
        __global int *MaskGlobal,
        __global int *costGlobal,
        __global int *predGlobal,
        __global int *imgVal,
        __global int *sem,
        __global int *extra,
        __global int *labelGlobal,
        __global int16 *cache,
        __global int *Cmin,
        __global int *rootGlobal,
        __global volatile int *rwLocks
        )
{
    int globalId = get_global_id(0);
    int blockId = get_group_id(0);
    int blockSize = get_local_size (0);
    int localId  = get_local_id(0);
    int i, q, cq;
    int qLocal;
    int tmp;
    int16 q8;
    int minPixel, maxPixel;
    int16 neighboorhood;
    int qarray [8], imgValArray [8];
    int ncols = img->ncols;

    __local int     MaskLocal  [1024];
    __local int     costLocal  [1024];
    __local int     predLocal  [1024];
    __local int     rootLocal  [1024];
    __local int     labelLocal  [1024];
    /*
    */
    __local int CminLocal [1];
    __local int CminNext [1];

    // Blocking area 
    MaskLocal [ localId ] = MaskGlobal [ globalId ];
    costLocal [ localId ] = costGlobal [ globalId ];
    predLocal [ localId ] = predGlobal [ globalId ];
    rootLocal [ localId ] = rootGlobal [ globalId ];
    labelLocal [ localId ] = labelGlobal [ globalId ];
    /*
    */
    if ( localId == 0 )
    {
        CminLocal [0] = Cmin[0];
        CminNext [0] = INT_MAX;
    }

    barrier ( CLK_LOCAL_MEM_FENCE );

    // Private area 
    minPixel = blockId * blockSize;
    maxPixel = minPixel + blockSize;
    /*
    minPixel = ( localId / ncols ) * ncols;
    maxPixel = minPixel + ncols;
    */
    neighboorhood = cache[globalId];

    //if ( MaskGlobal [globalId] == 1 && costGlobal[globalId] == Cmin[0] )
    /*
        barrier ( CLK_LOCAL_MEM_FENCE );
        if ( localId == 0 )
        {
            MaskOfTheBlock [ blockId ] = 0;
        }
        */
    while ( MaskOfTheBlock [ blockId ] == 1 )
    {
        MaskOfTheBlock [ blockId ] = 0;
        if ( MaskLocal [localId] == 1 && costLocal[localId] == CminLocal[0] )
        {
            //MaskGlobal [globalId] = 0;
            MaskLocal [localId] = 0;
            q8 = cache[globalId];
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
                //q = localId + dx [ i+1 ] + ncols * dy [ i+1 ] ;
                q = qarray[i];
                if ( q >= 0 && 
                        ( q < maxPixel && q >= minPixel ) )
                {
                    qLocal = q - minPixel;
                    if ( costLocal [ localId ] < costLocal [ qLocal ] )
                    {
                        tmp = max ( costLocal [ localId ], imgValArray [ i ]);

                        //if ( !mq || mq > tmp )
                        if ( costLocal [ qLocal ] > tmp )
                        {
                            //         costGlobal [ q ] = tmp;
                            //GetSemaphor ( & rwLocks [ q ] );
                            //                labelGlobal [ q ] = labelGlobal [ globalId ];
                            labelLocal [ qLocal ] = labelLocal [ localId ];
                            costLocal [ qLocal ] = tmp;
                            // costGlobal [q] = tmp;
                            //MaskGlobal [ q ] = 1;
                            //MaskLocal [ q % blockSize ] = 1;
                            atom_xchg ( & MaskLocal [ qLocal ], 1 );
    //                        MaskOfTheBlock [ blockId ] = 1;
                            //MaskLocal [ q - minPixel ] = 1;
                            predLocal [qLocal] = globalId;
                            //rootGlobal [ q ] = rootGlobal [ globalId ];
                            rootLocal [ qLocal ] = rootLocal [ localId ];
                            //ReleaseSemaphor ( & rwLocks [ q ] );
                        }
                    }
                }
            }
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
        if ( MaskLocal [ localId ] )
        {
            MaskOfTheBlock [ blockId ] = 1;
            atom_min ( CminNext , costLocal [ localId ] );
        }
        if ( !localId )
        {
            CminLocal [0] = CminNext[0];
            CminNext [0] = INT_MAX;
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
    }
        /*
        else
        {
            //barrier ( CLK_LOCAL_MEM_FENCE );
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
        if ( MaskLocal [ localId ]  )
        {
            atom_min ( CminNext, costLocal [ localId ] );
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
        if ( localId == 0 )
        {
            CminLocal [0] = CminNext [0];
            CminNext [0] = INT_MAX;
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
    }
    */

    /*
    //if ( MaskLocal [localId] == 1 && costLocal[localId] == Cmin[0] )
    if ( MaskGlobal [ globalId ] == 1 && costGlobal [ globalId ] == Cmin[0] )
    {
        MaskLocal [localId] = 0;
        q8 = cache[localId];
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
            if ( q >= 0 && 
                    ( q <= maxPixel && q >= minPixel ) )
            {
                // Mapping q to a local area
                q %= blockSize;
                if ( costLocal [ localId ] < costLocal [ q ] )
                {
                    cq = costLocal [ q ];
                    tmp = max ( costLocal [ localId ], imgValArray [ i ]);

                    //if ( !mq || mq > tmp )
                    if ( cq > tmp )
                    {
               //         costLocal [ q ] = tmp;
                        //GetSemaphor ( & rwLocks [ q ] );
                        labelLocal [ q ] = labelLocal [ localId ];
                        MaskLocal [ q ] = 1;
                        costLocal [q] = tmp;
                        predLocal [q] = globalId;
                        rootLocal [ q ] = rootLocal [ localId ];
                        atom_xchg ( &predLocal[q], localId );
                        //ReleaseSemaphor ( & rwLocks [ q ] );
                    }
                }
            }
        }
        barrier ( CLK_LOCAL_MEM_FENCE );
    }
*/

    barrier ( CLK_GLOBAL_MEM_FENCE );
    // Blocking area 
    MaskGlobal [ globalId ]  = MaskLocal [ localId ];
    costGlobal [ globalId ]  = costLocal [ localId ];
    predGlobal [ globalId ]  = predLocal [ localId ];
    rootGlobal [ globalId ]  = rootLocal [ localId ];
    labelGlobal [ globalId ]  = labelLocal [ localId ];
    Cmin [0] = CminLocal [0];
    /*
    */
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
    int tid = get_global_id(0);

//    if ( M[tid] >= 0 && M[tid] < 495 )
    if ( M[tid] )
    {
        if ( cost[tid] == *Cmin )
        {
            atomic_xchg ( extra, *Cmin );
            //atomic_inc ( matches );
            return;
        }

            atomic_min ( extra, cost[tid] );
    }
}
