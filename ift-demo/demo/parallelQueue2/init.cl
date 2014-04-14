#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int *tbrow;
    int ncols[0],nrows[0];
} ImageIFT;
/*

bool ValidPixel(__global ImageIFT *img, int x, int y)
{
    if ((x >= 0)&&(x < ncols[0])&&
            (y >= 0)&&(y < nrows[0]))
        return(true);
    else
        return(false);
}

void GetSemaphor(__global int * semaphor) {
    int occupied = atomic_xchg (semaphor, 1);
    while(occupied > 0)
    {
        occupied = atomic_xchg (semaphor, 1);
    }
}


void ReleaseSemaphor(__global int * semaphor)
{
    int prevVal = atomic_xchg (semaphor, 0);
}

*/

__kernel void initCache (
        __global int *ival,
        __constant int *itbrow,
        __constant int *An,
        __constant int *dx,
        __constant int *dy,
        __global int *sem,
        __global int *extra,
        __global int16 *cache,
        __global int *nrows,
        __global int *ncols
        )
{
    Pixel u, v;
    int tid = get_global_id(0);
    int i, j;
    int tmp[8];

    //__local int cache[512*8];
    u.x = tid % ncols[0];
    u.y = tid / ncols[0];

    for ( i = 0; i < *An-1; i++ ) {
        j = i + 1;
        v.x = u.x + dx[j];
        v.y = u.y + dy[j];

        if ((v.x >= 0)&&(v.x < ncols[0])&&
                (v.y >= 0)&&(v.y < nrows[0])) {
//            cache [tid + j]si = v.x + itbrow [v.y];
            tmp [i] = v.x + itbrow [v.y];
        }
        else {
            tmp [i] = -1;
        //    cache [tid + j] = -1;
        }
    }

    cache[tid] = (int16)( 
            tmp [ 0 ],
            tmp [ 1 ],
            tmp [ 2 ],
            tmp [ 3 ],
            tmp [ 4 ],
            tmp [ 5 ],
            tmp [ 6 ],
            tmp [ 7 ],
            ival [ tmp [0] ],
            ival [ tmp [1] ],
            ival [ tmp [2] ],
            ival [ tmp [3] ],
            ival [ tmp [4] ],
            ival [ tmp [5] ],
            ival [ tmp [6] ],
            ival [ tmp [7] ]
            );

} 
