__kernel void dijkstra (
		__global const int *V,
		__global const int *A,
		__global const int *W,
		__global int *M,
		__global int *C,
		__global int *U
		) {
	int tid = get_global_id(0);
	
	if (M[tid]) {
		M[tid] = false;
		int first, last;
		first = V[tid];
		// TODO
		last = V[tid+1]; // Non-coalesced access - Review

		for (int nid = first; nid < last; nid++) {
			if (U[nid] > C[tid]+W[nid]) {
				U[nid] = C[tid]+W[nid];
			}
		}
	}
}

