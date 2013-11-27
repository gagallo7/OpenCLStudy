#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
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
		__global int *CostLabel,
		__global int *Sets,
        /*
		__global int *UpdateLabel,
        __global int *UpdatePred,
        */
        __global int *CostPred,
		__global volatile int *sem
		//__global int *numV
		) {
	int tid = get_global_id(0);
<<<<<<< HEAD
=======


    CostLabel[tid] = -1;
    CostCost[tid] =  INT_MAX;
    Mask[tid] = false;

    /*
    if ( Sets [tid] == 1) {
        CostLabel[tid] = 0;
        CostCost[tid] =  0;
        CostPred[tid] = -1;
        Mask[tid] = true;
    }

    
    else if (Sets[tid] == 2) {
        CostLabel[tid] = 1;
        CostCost[tid] =  0;
        CostPred[tid] = -1;
        Mask[tid] = true;
    }
    */

    /*
    //if (CostCost[tid] > UpdateCost[tid]) {
>>>>>>> f52f5c5666372d78a2e73ea9d5317ccd8255e9b7
    if ( CostCost[tid] > UpdateCost[tid] //||
    ) {
        CostCost[tid] = UpdateCost[tid];
        CostPred[tid] = UpdatePred[tid];
        Mask[tid] = true;
    }
    UpdatePred[tid] = CostPred[tid];
    UpdateCost[tid] = CostCost[tid];
<<<<<<< HEAD
=======
  //  UpdateLabel[tid] = CostLabel[tid];
  */
>>>>>>> f52f5c5666372d78a2e73ea9d5317ccd8255e9b7
}

