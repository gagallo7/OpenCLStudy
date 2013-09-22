// Considerando uma matriz a de dimensões l x m e
// uma matriz b de dimensões m x n
// c = a x b
__kernel void multiplyMatrix (	const __global uint *A,
		const __global uint *B,
		__global uint *C,
		const int l,
		const int m,
		const int n	)
{

	int i = get_global_id(0);
	int j = get_global_id(1);

	int p = get_global_size(0);
	int r = get_global_size(1);

//	C[i + r * j] = 0;
	C[j+r*i] = 0;
	int QQ = m;
	for (int k = 0; k < QQ; k++)
	{
//		C[i + p * j] += A[i + QQ * k] * B[k + QQ * j];
//		C[i + p * j] += B[i + QQ * k] * A[k + QQ * j];
		//C[i + p * j] = 100* A[i + QQ * k] +  B[k + QQ * j];
//		C[j + r * i] = 100* B[i + QQ * k] +  A[k + QQ * j];
		//C[j+r*i] += A[k + m*i] * B[j + k*n];
		C[j+p*i] += A[k + p*i] * B[j + k*p];
	//	C[i+p*j] = i + p*j;
//		C[i+p*j] = j ;

	//	C[i + p * j] = mad(A[i + p * k], B[k + QQ * j], C[i + p * j]);
	}
}


