void GetSemaphor(__global int * semaphor) {
    int occuPredRecied = atomic_xchg (semaphor, 1);
    while(occuPredRecied > 0)
    {
        occuPredRecied = atomic_xchg (semaphor, 1);
    }
}

void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atomic_xchg (semaphor, 0);
}

__kernel void dijkstra3 (
//        __global int *Mask,
        //__global int *CostCost,
//        __global int *UpdateCost,
        __global int *CostLabel,
//        __global int *UpdateLabel,
        //__global int *CostPred,
        __global int *CostPred,
        __global volatile int *sem,
        __global int *n
        ) {
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int uPredRec, tmp, j = 0;
    int nn = n[0];
    /*
    __local int map [ 1024 ];
    map [gid] = CostPred [tid];
    barrier ( CLK_LOCAL_MEM_FENCE );
    */
    int a = get_local_size(0) * get_group_id (0);
    int b = a + get_local_size(0);
    /*

*/

    /*
    map [ gid ] = 0;
    */
//    uint gap = get_global_size(0) - n[0];

    // Having a conditional if without else is bad because it debalances
    // the workflow of workitems and their workgroups
    // but it stills faster than the serial version
 //   if ( tid < nn ) {

        /*
        for ( uPredRec = tid; uPredRec != -1 && !map[gid]; uPredRec = CostPred [ uPredRec ] ) {
            if ( CostPred [ uPredRec ] < 0 ) {
                //CostLabel [tid] = CostLabel [ uPredRec ];
                atom_xchg (&CostLabel[tid], CostLabel[uPredRec]);
                // The pixel was just sweeped
                //CostPred [tid] = -1;
                atom_xchg (&CostPred[tid], -1);
                map [ gid ] = 1;
            }
        }
        */
        /*
        uPredRec = tid;
        tmp = map [ uPredRec % 1024 ];

        while ( tmp >= 0 && uPredRec >= a && uPredRec < b ) {
            uPredRec = tmp;
            tmp = map [ uPredRec % 1024 ];
        }
        if ( tmp == -1 ) {
            CostLabel [tid] = CostLabel [uPredRec];
            CostPred [tid] = -1;
        }
        */

    if ( tid < nn ) {
        uPredRec = tid;
        tmp = CostPred [ uPredRec ];

        while ( tmp >= 0 && j < nn ) {
            j++;
            uPredRec = tmp;
            tmp = CostPred [ uPredRec ];
        }
        CostLabel [tid] = CostLabel [uPredRec];
        CostPred [tid] = -1;
    }
        /*
    */
}

