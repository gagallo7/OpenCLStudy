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

/*
   tid ← getThreadID
   if Ca [tid] > Ua [tid] then
   Ca [tid] ← Ua [tid]
   Ma [tid] ← true
   end if
   Ua [tid] ← Ca [tid]
 */

__kernel void dijkstra2 (
		__global const int *V,
		__global const int *A,
		__global const int *W,
		__global int *M,
		__global int *C,
		__global int *U,
		__global int *sem,
		//__global int *numV
		) {
	int tid = get_global_id(0);
    int numV = 5;
	// Garantindo que o work-item nao ultrapasse
	// o tamanho do vetor
	if (tid < numV) {
		if (C[tid] > U[tid]) {
			C[tid] = U[tid];
			M[tid] = true;
		}
		U[tid] = C[tid];
	}
}

