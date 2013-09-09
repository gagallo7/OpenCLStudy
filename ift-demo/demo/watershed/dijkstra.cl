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


__kernel void dijkstra (
		__global const int *V,
		__global const int *A,
		__global const int *W,
		__global int *M,
		__global int *C,
		__global int *U,
		__global int *sem,
		__global int *numV
		) {
	int tid = get_global_id(0);

	if (M[tid]) {
		M[tid] = false;
		GetSemaphor(sem);
		int first, last;
		first = V[tid];
		// Garantindo que o work-item nao ultrapasse
		// o tamanho do vetor
		if ( tid + 0 < numV  ) 
					// TODO
			last = V[tid+1]; // Non-coalesced access - Review
		ReleaseSemaphor(sem);

		for (int nid = first; nid < last; nid++) {
			int edge = A[nid];
			if (U[edge] > C[tid]+W[nid]) {
				U[edge] = C[tid]+W[nid];
			}
		}
	}
}

