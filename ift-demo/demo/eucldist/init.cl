#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int *tbrow;
    int nCols,nRows;
} ImageIFT;


__kernel void initCache (
        __constant int *nrows,
        __constant int *ncols,
        __global int *ival,
        __constant int *itbrow,
        __constant int *An,
        __constant int *dx,
        __constant int *dy,
        __global int *sem,
        __global int *extra,
        __global int8 *cache)
{
    Pixel u, v;
    int tid = get_global_id(0);
    int i, j;
    int tmp[8];
    int nRows = nrows[0];
    int nCols = ncols[0];

    //__local int cache[512*8];
    u.x = tid % nCols;
    u.y = tid / nCols;

    for ( i = 0; i < *An-1; i++ ) {
        j = i + 1;
        v.x = u.x + dx[j];
        v.y = u.y + dy[j];

        if ((v.x >= 0)&&(v.x < nCols)&&
                (v.y >= 0)&&(v.y < (nRows))) {
//            cache [tid + j]si = v.x + itbrow [v.y];
            tmp [i] = v.x + itbrow [v.y];
        }
        else {
            tmp [i] = -1;
        //    cache [tid + j] = -1;
        }
    }

    cache[tid] = (int8)( 
            tmp [ 0 ],
            tmp [ 1 ],
            tmp [ 2 ],
            tmp [ 3 ],
            tmp [ 4 ],
            tmp [ 5 ],
            tmp [ 6 ],
            tmp [ 7 ]
            );

} 
