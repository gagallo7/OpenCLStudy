#ifndef MAX
#define MAX(x,y) (((x) > (y))?(x):(y))
#endif

typedef struct _pixel {
    int x,y;
} Pixel;

typedef struct _figure {
    int *val;
    int *tbrow;
    int ncols,nrows;
} ImageIFT;

bool ValidPixel(__global ImageIFT *img, int x, int y)
{
    if ((x >= 0)&&(x < img->ncols)&&
            (y >= 0)&&(y < img->nrows))
        return(true);
    else
        return(false);
}

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


__kernel void initCache (
        __global ImageIFT *img,
        __global int *ival,
        __global int *itbrow,
        __global int *An,
        __global int *dx,
        __global int *dy,
        __global int *sem,
        __global int *extra,
        __global int *cache)
{
    Pixel u, v;
    int tid = get_global_id(0);
    int i, q, tmp;

    //__local int cache[512*8];
    u.x = tid % img->ncols;
    u.y = tid / img->ncols;

    for ( i = 1; i < *An; i++ ) {
        v.x = u.x + dx[i];
        v.y = u.y + dy[i];

        if ( ValidPixel ( img, v.x, v.y ) ) {
            cache [tid*8 + i -1] = v.x + itbrow [v.y];
        }
        else
            cache [tid*8 + i -1] = -1;
    }


} 
