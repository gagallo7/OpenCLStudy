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

//TODO
/*
Only update label here
calculate q only here, and update it
*/

__kernel void dijkstra2 (
		__global int *M,
		__global int *C,
		__global int *U,
		__global int *ClabelVal,
		__global int *UlabelVal,
		__global int *sem
		) {
	int tid = get_global_id(0);
	// Garantindo que o work-item nao ultrapasse
	// o tamanho do vetor
        int t;
    t = ClabelVal[tid];
    if (C[tid] > U[tid]) {
        C[tid] = U[tid];
        atom_xchg ( &UlabelVal[tid], UlabelVal[t] );
        M[tid] = true;
    }
    //U[tid] = C[tid];
}

