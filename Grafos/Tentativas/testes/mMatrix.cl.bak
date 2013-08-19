#define BLOCK_SIZE 8
// Considerando uma matriz a de dimensões l x m e
// uma matriz b de dimensões m x n
// c = a x b
__kernel __attribute__((reqd_work_group_size(BLOCK_SIZE, BLOCK_SIZE, 1))) void multiplyMatrix (	const __global uint *A,
		const __global uint *B,
		__global uint *C,
		const int l,
		const int m,
		const int n	)
{
	//Identification of this workgroup
   int i = get_group_id(0);
   int j = get_group_id(1);

   //Identification of work-item
   int idX = get_local_id(0); 
   int idY = get_local_id(1);

   //matrixes dimensions
   int p = get_global_size(0);
   int r = get_global_size(1);
   int qq = m;

   //Number of submatrixes to be processed by each worker (Q dimension)
   int numSubMat = qq/BLOCK_SIZE;

   float4 resp = (float4)(0,0,0,0);
   __local float sA[BLOCK_SIZE][BLOCK_SIZE];
   __local float sB[BLOCK_SIZE][BLOCK_SIZE];

   for (int k=0; k<numSubMat; k++)
   {
       //Copy submatrixes to local memory. Each worker copies one element
       //Notice that A[i,k] accesses elements starting from M[BLOCK_SIZE*i, BLOCK_SIZE*j]
       sA[idX][idY] = B[BLOCK_SIZE*i + idX + p*(BLOCK_SIZE*k+idY)];
       sB[idX][idY] = A[BLOCK_SIZE*k + idX + qq*(BLOCK_SIZE*j+idY)];
       barrier(CLK_LOCAL_MEM_FENCE);

       for (int k2 = 0; k2 < BLOCK_SIZE; k2+=4) 
       {
            float4 temp1=(float4)(sA[idX][k2],sA[idX][k2+1],sA[idX][k2+2],sA[idX][k2+3]);
            float4 temp2=(float4)(sB[k2][idY],sB[k2+1][idY],sB[k2+2][idY],sB[k2+3][idY]);
            resp += temp1 * temp2;
       }
       barrier(CLK_LOCAL_MEM_FENCE);
   }

   C[BLOCK_SIZE*i + idX + p*(BLOCK_SIZE*j+idY)] = resp.x+resp.y+resp.z+resp.w;


}


