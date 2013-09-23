
__kernel void hello_kernel(__global float *a,
		__global const float *b,
		__global float *result,
		const unsigned int n)
{
	float temp;
	int gid = get_global_id(0);
	if(gid<n) {
		temp = a[gid] * b[gid];
	}
//		result[gid] += a[gid] * b[gid];
	result[gid] += temp;
}
