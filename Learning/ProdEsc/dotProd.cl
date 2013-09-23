__kernel void dotProd ( __global const float *a,
			__global const float *b,
			__global float *result,
			uint n)
{
	float temp = 0;

	int gid = get_global_id(0);

	if(gid >= n/4) {
		return;
	}

	gid <<= 2;

	temp = a[gid] * b[gid] + a[gid+1] * b[gid+1] +
		      a[gid+2] * b[gid+2] + a[gid+3] * b[gid+3];

	result[gid] = temp;
}

