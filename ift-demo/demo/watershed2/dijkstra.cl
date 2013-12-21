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


__kernel void dijkstra (
        __global ImageIFT *img,
        __global int *ival,
        __global int *itbrow,
        __global int *An,
        __global int *dx,
        __global int *dy,
        __global int *M,
        __global int *CostCost,
        __global int *UpdateCost,
        /*
           __global int *UpdateLabel,
           __global int *Clval,
         */
        __global int *UpdatePred,
        __global int *CostPred,
        __global int *sem,
        __global int *extra)
{
    Pixel u, v;
    int tid = get_global_id(0);
    int gid = get_local_id(0);
    int i, q, tmp;

    __local int cache[512*8];
    u.x = tid % img->ncols;
    u.y = tid / img->ncols;

    for ( i = 1; i < *An; i++ ) {
        v.x = u.x + dx[i];
        v.y = u.y + dy[i];

        if ( ValidPixel ( img, v.x, v.y ) ) {
            cache [gid*8 + i -1] = v.x + itbrow [v.y];
        }
        else
            cache [gid*8 + i -1] = -1;
    }

    barrier (CLK_LOCAL_MEM_FENCE);

    if ( M[tid] ) {
        M [tid] = false;
        /*
           u.x = tid % img->ncols;
           u.y = tid / img->ncols;

           for ( i = 1; i < *An; i++ ) {
           v.x = u.x + dx[i];
           v.y = u.y + dy[i];

           if ( ValidPixel ( img, v.x, v.y ) ) {
        //                q = v.x + itbrow [v.y];
         */
        for ( i = 0; i < 8; i++ ) {
            q = cache [ gid*8 + i ];
            if ( q >= 0 ) {
                tmp = max ( CostCost [tid], ival [q] );
                if ( UpdateCost [q] > tmp ) {
                    //            atom_inc (extra);
                    UpdateCost [q] = tmp;
                    UpdatePred [q] = tid;
                    /*
                       atom_xchg ( &CostCost [q], tmp );
                       atom_xchg ( &CostPred [q], tid );
                       atom_xchg ( &M [q], true );
                     */
                }
            }
        }
        /*
           }
           }                   
           }
         */
}
} 
