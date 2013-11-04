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

__kernel void dijkstra2 (
		__global int *Mask,
		__global int *CostCost,
		__global int *UpdateCost,
        /*
		__global int *CostLabel,
		__global int *UpdateLabel,
        */
        __global int *UpdatePred,
        __global int *CostPred,
		__global volatile int *sem
		//__global int *numV
		) {
	int tid = get_global_id(0);
    //if (CostCost[tid] > UpdateCost[tid]) {
    if ( CostCost[tid] > UpdateCost[tid] //||
//        (CostCost[tid] == UpdateCost[tid] && UpdatePred[tid] != CostPred[tid]) 
    ) {
        CostCost[tid] = UpdateCost[tid];
//        CostLabel[tid] = UpdateLabel[tid];
        CostPred[tid] = UpdatePred[tid];
        Mask[tid] = true;
    }
//        if (CostLabel[tid] != UpdateLabel[tid]) {}
    UpdatePred[tid] = CostPred[tid];
    UpdateCost[tid] = CostCost[tid];
  //  UpdateLabel[tid] = CostLabel[tid];
}

