#include "ift.h"

int main (int argc, char* argv[] )
{
    Image *img = NULL;
    Image *label = NULL;
    CImage *cimg = NULL;
    char     outfile[100];
    char     *file_noext;
    int p;

    Set      *Obj=NULL,*Bkg=NULL, *S;

    img = ReadImage ( argv [1] );
    label = CreateImage ( img->ncols, img->nrows );

    ReadSeeds ( argv[2], &Obj, &Bkg );

    S = Obj;
    while(S != NULL){
        p=S->elem;
        label->val[p]=1;
        S = S->next;
    }
    S = Bkg;
    while(S != NULL){
        p=S->elem;
        label->val[p]=-1;
        S = S->next;
    }

    cimg = DrawLabeledRegionsDiff(img,label);
    WriteCImage(cimg, "seeds.ppm");

    DestroyImage(&img);
    DestroySet(&Obj);
    DestroySet(&Bkg);

    return 0;
}
