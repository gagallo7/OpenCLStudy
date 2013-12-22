void GetSemaphor(__global int * semaphor) {
    int occuPredRecied = atom_xchg (semaphor, 1);
    while(occuPredRecied > 0)
    {
        occuPredRecied = atom_xchg (semaphor, 1);
    }
}

void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atom_xchg (semaphor, 0);
}

__kernel void dijkstra3 (
        __global int *Mask,
        __global int *CostCost,
        __global int *UpdateCost,
        __global int *CostLabel,
        __global int *UpdateLabel,
        __global int *UpdatePred,
        __global int *CostPred,
        __global volatile int *sem
        ) {
    int tid = get_global_id(0);
    int uPredRec, i = 0;
    for ( uPredRec = tid; uPredRec != -1; uPredRec = UpdatePred [ uPredRec ] ) {
            if ( UpdatePred [ uPredRec ] < 0 ) {
                //CostLabel [tid] = CostLabel [ uPredRec ];
                atom_xchg (&CostLabel[tid], CostLabel[uPredRec]);
                // The pixel was just sweeped
                //CostPred [tid] = -1;
                atom_xchg (&CostPred[tid], -1);
            }
    }
    UpdatePred[tid] = CostPred[tid];
    UpdateCost[tid] = CostCost[tid];
}

