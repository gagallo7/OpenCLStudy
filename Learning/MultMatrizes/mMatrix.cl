// Considerando uma matriz a de dimensões l x m e
// uma matriz b de dimensões m x n
// c = a x b
__kernel void multiplyMatrix (	const __global float *a,
		const __global float *b,
		__global float *result,
		const int l,
		const int m,
		const int n	)
{
/*
	const int x = get_global_id(0);
	const int y = get_global_id(1);

	float somaElem=0;
	uint tmp, ind1, ind2;
	//	if(x < l && y < n) {
	for(int i=0; i<m; i++) {
		ind1 = x*get_global_size(0) + i;
		ind2 = i*m + y;
		somaElem += a[ind1] * b[ind2];
	}

	tmp = x*get_global_size(0) + y;
	result[tmp] = somaElem;
	somaElem = 0;
	//	}

*/

	int i = get_global_id(0);
	int j = get_global_id(1);

	int p = get_global_size(0);
	int r = get_global_size(1);

	result[i + p * j] = 0;
	int QQ = m;
	for (int k = 0; k < QQ; k++)
	{
//		result[i + p * j] += a[i + p * k] * b[k + QQ * j];
		result[i + p * j] = mad(a[i + p * k], b[k + QQ * j], result[i + p * j]);
	}
}


