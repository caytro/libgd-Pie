#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"

#define IMAGE_SIZE 1024
#define NB_DATAS 5

typedef struct PieDataType{
    //int indice;
    double valeur;
    unsigned char label[255];
} PieData;

double calcRatioPourcent(PieData *datas)
{
    double total = 0;
    for (int i = 0 ; i < NB_DATAS ; i++){
        total += datas[i].valeur;
    }
    return 100.0 / total;
}
int main (void)
{
    gdImagePtr im = gdImageCreate(IMAGE_SIZE,IMAGE_SIZE);

    // Pie Datas

    PieData pieDatas[NB_DATAS] = {
        {12,"France"},
        {35, "Royaume Uni"},
        {58, "Allemagne"},
        {125, "Etats Unis"},
        {50,"Russie"}
    };
    // Colors

    int colors[16];
    int colorBlack=gdImageColorAllocate(im,0,0,0);
    int colorRed=gdImageColorAllocate(im,255,0,0);
    int colorGreen=gdImageColorAllocate(im,0,255,0);
    int colorBlue=gdImageColorAllocate(im,80,80,255);
    int colorWhite=gdImageColorAllocate(im,255,255,255);
    int colorYellow=gdImageColorAllocate(im,255,255,0);
    int colorOrange=gdImageColorAllocate(im,255,200,0);
    int colorViolet=gdImageColorAllocate(im,255,0,255);
    int colorGray=gdImageColorAllocate(im,128,128,128);
    int indiceColor = 0;
    colors[indiceColor++]=colorBlack;
    colors[indiceColor++]=colorRed;
    colors[indiceColor++]=colorGreen;
    colors[indiceColor++]=colorBlue;
    colors[indiceColor++]=colorWhite;
    colors[indiceColor++]=colorYellow;
    colors[indiceColor++]=colorOrange;
    colors[indiceColor++]=colorViolet;
    colors[indiceColor++]=colorGray;

    // Fonts

    gdFontPtr fonts[5];
    fonts[0] = gdFontGetTiny ();
    fonts[1] = gdFontGetSmall ();
    fonts[2] = gdFontGetMediumBold ();
    fonts[3] = gdFontGetLarge ();
    fonts[4] = gdFontGetGiant ();

    // Pie

    int xc = IMAGE_SIZE /2;
    int yc = IMAGE_SIZE /2;
    double ratio = 0.6;
    int w = IMAGE_SIZE * ratio;
    int h = IMAGE_SIZE * ratio;
    int textRadius = w /2 * 1.2;
    int percentRadius = w/4;
    double ratioAngle = calcRatioPourcent(pieDatas);
    double curAngle = 0;
    unsigned char label[256];
    for(int i=0;i<NB_DATAS;i++)
    {
        int s = (int) round(curAngle);
        int e = (int) round(curAngle  + pieDatas[i].valeur * ratioAngle * 360.0 /100.0);
        printf("Angles : %d %d\n",s, e);
        gdImageFilledArc(im,xc,yc,w,h,(int)s,(int)e,colors[i+1],0);
        int xText = (int) (xc + textRadius * cos ( (e + s)/2 * M_PI/180 ));
        int yText = (int) (yc + textRadius * sin ( (e + s)/2 * M_PI/180 ));
        gdImageString(im, fonts[2],(int)xText,(int)yText,pieDatas[i].label,colors[i+1]);
        xText = (int) (xc + percentRadius * cos ( (e + s)/2 * M_PI/180 ));
        yText = (int) (yc + percentRadius * sin ( (e + s)/2 * M_PI/180 ));
        sprintf(label,"%d%%",(int) round(pieDatas[i].valeur * ratioAngle));
        gdImageString(im, fonts[2],(int)xText,(int)yText,label ,colorBlack);
        curAngle = (double) e;


    }
    FILE *out = fopen("test.png","wb");

    gdImagePng(im,out);
    fclose(out);
    int status = system("display test.png");
    return status;
}
