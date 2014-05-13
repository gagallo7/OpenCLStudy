#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

#define counter M[200000]


typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int *tbrow;
    int ncols,nrows;
} ImageIFT;

void GetSemaphor(__global volatile int * semaphor) {
    int occupied = atomic_xchg (semaphor, 1);
    while(occupied > 0)
    {
        occupied = atomic_xchg (semaphor, 1);
    }
}


void ReleaseSemaphor(__global volatile int * semaphor)
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
        __global int *CminBlock,
        __global int *MaskBlocks,
        __global int *dy,
        __global int *MaskGlobal,
        __global int *costGlobal,
        __global int *predGlobal,
        __global int *sem,
        __global int *extra,
        __global int *labelGlobal,
        __global int16 *cache,
        __global int *Cmin,
        __global int *rootGlobal,
        __global volatile int *rwLocks
//        __global int *MaskBlocks2
        )
{
    int globalId = get_global_id(0);
    int blockId = get_group_id(0);
    int blockSize = get_local_size (0);
    int localId  = get_local_id(0);
    int i, q, cq;
    int tmp;
    int minPixel, maxPixel;
    int16 neighboorhood;
    int neighboor [8], imgValArray [8];
    int R;
    __local int     MaskLocal  [1024];
    __local int     costLocal  [1024];
    __local int     predLocal  [1024];
    __local int     rootLocal  [1024];
    __local int     labelLocal  [1024];
    __local int CminLocal [2];
    __local int ImLive [1];

    // Blocking area 
    MaskLocal [ localId ] = MaskGlobal [ globalId ];
    costLocal [ localId ] = costGlobal [ globalId ];
    predLocal [ localId ] = predGlobal [ globalId ];
    rootLocal [ localId ] = rootGlobal [ globalId ];
    labelLocal [ localId ] = labelGlobal [ globalId ];
    /*
    CminLocal [0] = Cmin[0];
    CminLocal [1] = INT_MAX;
    *ImLive = 0;
    */
    barrier ( CLK_LOCAL_MEM_FENCE );

    // Private area 
    minPixel = blockId * blockSize;
    maxPixel = minPixel + blockSize;
    neighboorhood = cache[globalId];
    int done = 0;

    // There are 8 neighboors, because 
    // it is 8-adjacency 
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

    // Initializing Blocks' Mask 
    //if ( !MaskBlocks [ blockId ] && MaskLocal [ localId ] == 1 )
    /*
    if ( !localId && MaskBlocks [ blockId ] )
    {
       //MaskBlocks [ blockId ] = 1;
       MaskBlocks [ blockId ] = 0;
       *ImLive = 1;
    }
    if ( !MaskBlocks [ blockId ] && MaskLocal [ localId ] == 1 )
    {
        MaskBlocks [ blockId ] = 1;
    }
    */

    // In the toy figure, the image has to be
    // segmented when the program exits this loop
    //while ( MaskBlocks [ blockId ] == 1 )
    while ( !done && MaskBlocks [ blockId ] == 1 )
    {
        barrier ( CLK_LOCAL_MEM_FENCE );
        MaskBlocks [ blockId ] = 0;

        if ( MaskLocal [ localId ] == 1 && costLocal [ localId ] == CminLocal [0] )
        {
            MaskLocal [ localId ] = 0;
            done = 1;
            for ( i = 0; i < 8; i++ )
            {
                q = neighboor[i];

                // q >= 0 -- neighboor is a valid pixel
                // clamp ( q, minPixel, maxPixel ) -- testing if
                // q is into the block -- minPixel <= q <= maxPixel?
                //if ( q >= 0 && clamp ( q, minPixel, maxPixel ) == q ) 
                if ( q >= 0 && 
                        ( q <= maxPixel && q >= minPixel ) )

                {
                    // Mapping q to a local area
                    q %= blockSize;
                    //q &= blockSize - 1;

                    if ( costLocal [ localId ] < costLocal [ q ] )
                    {
                        tmp = max ( costLocal [ localId ], imgValArray [ i ]);

                        if ( costLocal [ q ] > tmp )
                        {
                            // Conquering neighboor
                            //GetSemaphor ( & rwLocks [ q ] );
                            MaskLocal [ q ] = 1;
                            MaskBlocks [ blockId ] = 1;
                            labelLocal [ q ] = labelLocal [ localId ];
                            rootLocal [ q ] = rootLocal [ localId ];
                            costLocal [ q ] = tmp;
                            predLocal [ q ] = globalId;
                            //ReleaseSemaphor ( & rwLocks [ q ] );
                        }
                    }
                }
            }
        }
        // Guaranteeing that all local operations has finished
        // before the loop iterates again
        /*
        //barrier ( CLK_LOCAL_MEM_FENCE );
        if ( MaskLocal [ localId ] == 1 )
        {
           atom_min ( &CminLocal [1], costLocal [ localId ] );
        }
        //barrier ( CLK_LOCAL_MEM_FENCE );
        if ( localId == 0 )
        {
            //atom_xchg ( CminLocal, CminLocal [ 1 ] );
            CminLocal [ 0 ] = CminLocal [ 1 ];
            CminLocal [ 1 ] = INT_MAX;
        }
        */
        barrier ( CLK_LOCAL_MEM_FENCE );
    }

    // Blocking area 
    MaskGlobal [ globalId ]  = MaskLocal [ localId ];
    costGlobal [ globalId ]  = costLocal [ localId ];
    predGlobal [ globalId ]  = predLocal [ localId ];
    rootGlobal [ globalId ]  = rootLocal [ localId ];
    labelGlobal [ globalId ]  = labelLocal [ localId ];
    //MaskBlocks [ blockId ] = blockId;
    //CminBlock [ blockId ] = CminLocal [ 0 ];
    //Cmin [ 0 ]  = CminLocal [ 0 ] ;
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

        else if ( cost [tid] > 0 )
        {
            atomic_min ( extra, cost[tid] );
        }
    }
}