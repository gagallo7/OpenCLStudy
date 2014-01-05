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
        __global int *Mask,
        __global int *CostCost,
        __global int *UpdateCost,
        __global int *CostLabel,
        __global int *UpdateLabel,
        __global int *UpdatePred,
        __global int *CostPred,
        __global volatile int *sem,
        __global int *n
        ) {
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int uPredRec;
    /*
    __local int map [ 128 ];
    map [ gid ] = 0;
    */
//    uint gap = get_global_size(0) - n[0];

    // Having a conditional if without else is bad because it debalances
    // the workflow of workitems and their workgroups
    // but it stills faster than the serial version
    if ( tid < n[0] ) {

        /*
        for ( uPredRec = tid; uPredRec != -1 && !map[gid]; uPredRec = UpdatePred [ uPredRec ] ) {
            if ( UpdatePred [ uPredRec ] < 0 ) {
                //CostLabel [tid] = CostLabel [ uPredRec ];
                atom_xchg (&CostLabel[tid], CostLabel[uPredRec]);
                // The pixel was just sweeped
                //CostPred [tid] = -1;
                atom_xchg (&CostPred[tid], -1);
                map [ gid ] = 1;
            }
        }
        */

        uPredRec = tid;

        while ( UpdatePred [ uPredRec ] != -1 ) {
            uPredRec = UpdatePred [ uPredRec ];
        }
                atom_xchg (&CostLabel[tid], CostLabel[uPredRec]);
                atom_xchg (&CostPred[tid], -1);
        UpdatePred[tid] = CostPred[tid];
        UpdateCost[tid] = CostCost[tid];
    }
}

