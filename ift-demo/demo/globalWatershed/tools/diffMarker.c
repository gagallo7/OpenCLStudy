#include "ift.h"

int main ( int argc, char* argv[] )
{
    Image *img = NULL;
    Image *comp = NULL;
    CImage *cimg = NULL;
    Image *label;
    FILE* fp;

    int i = 0, num;
    int j;

    if ( argc < 4 )
    {
        printf ( "The program needs three args\n" );
        printf ( "Usage: ./diffTool originalImage label1 label2\n" );
        printf ( "Usage2: ./diffTool originalImage label1 label2 labelCmp\n" );
        return 1;
    }

    img = ReadImage ( argv[1] );
    label = CreateImage ( img->ncols, img->nrows );

    fp = fopen ( argv[2], "r" );

    while ( fscanf ( fp, " %d", &num ) != EOF )
    {
        label->val [i] = num;
        i++;
    }

    fclose ( fp );

    fp = fopen ( argv[3], "r" );

    i = 0;
    while ( fscanf ( fp, " %d", &num ) != EOF )
    {
        label->val [i] -= num;
        i++;
    }

    fclose ( fp );

    fp = fopen ( "teste", "w" );

    for ( j = 0; j < i; j++ )
    {
        fprintf ( fp, "%d ", label->val[j] );
    }

    fclose ( fp );

    if ( argc > 4 )
    {
        comp = CreateImage ( img->ncols, img->nrows );
        fp = fopen ( argv[4], "r" );
        i = 0;
        while ( fscanf ( fp, " %d", &num ) != EOF )
        {
            comp->val [i] -= num;
            i++;
        }

        fclose ( fp );
        cimg = DrawLabeledRegionsComp(img,label, comp);
    }
    else
    {
        cimg = DrawLabeledRegions(img,label);
    }
    WriteCImage(cimg,"imagediff.ppm");

    return 0;
}
