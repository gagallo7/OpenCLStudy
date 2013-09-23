
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
// Considerando uma matriz a de dimensões l x m e
// uma matriz b de dimensões m x n
// c = a x b

void getSem (__global int *sem) {
	int busy = atom_xchg(sem, 1);
	while (busy > 0) {
		busy = atom_xchg(sem, 1);
	}
}

void unlockSem (__global int* sem) {
	atom_xchg(sem, 0);
}
__kernel  void sum2 (	
		const __global uint *A,
		__global uint *B,
		const int l,
		__global int* sem)
{
	int z = get_global_id(0);

	//matrixes dimensions
	int p = get_global_size(0);

	int t1,t2;

	
	if(z%l==0 && z+l/2<p) {
		t1 = B[z+l/2];
		B[z] += t1;
	}
	//B[0] = desc;
}

__kernel  void somatorio (	
		const __global uint *A,
		__global uint *B,
		const int l,
		__global int* sem)
{
	//Identification of this workgroup
	int i = get_group_id(0);
	int aux=0;

//	int z = get_global_id(0);

	//Identification of work-item
	int idX = get_local_id(0); 

	//matrixes dimensions
	int p = get_global_size(0);
	int g = get_local_size(0);

	int desc = 2;
	int t1,t2;

	/*
	
	if(z%2==0) {
		B[z] += B[z+1];
	}
	*/

		getSem(sem);
		int z = get_global_id(0);
	while(desc<3) {
//	mem_fence(CLK_GLOBAL_MEM_FENCE);
//		barrier(CLK_GLOBAL_MEM_FENCE);
//		write_mem_fence(CLK_GLOBAL_MEM_FENCE);
		if(z%desc==0) {
			B[z] += B[z+desc/2];
		}
		desc *= 2;
	}
		unlockSem(sem);
//	sum2(A,B,8,sem);
	//B[0] = desc;
}


