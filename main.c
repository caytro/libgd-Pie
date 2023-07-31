#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "gd.h"
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"

#define IMAGE_SIZE 712
#define H_TITRE 128
#define TYPE_PIE 1
#define TYPE_HISTO 2

typedef struct PieDataType{
    double valeur;
    char label[255];
    struct PieDataType *next;
} PieData;

typedef struct {
    char titre[255];
    int type;
    PieData *first;
} PieChart;



PieData *createPieData(char *label, double val){
    PieData *new = malloc(sizeof(PieData));
    strcpy(new->label,label);
    new->valeur = val;
    new->next = NULL;
    return new;
}

PieChart *newPieChart(char *titre)
{
    PieChart *new = malloc(sizeof(PieChart));
    strcpy(new->titre,titre);
    new->type = TYPE_PIE; // valeur par défaut
    new->first=NULL;
    return new;
}


PieChart *appendPieData(PieChart *pieChart,char *label, double val)
{
    PieData *newPieData = createPieData(label,val);
    if (pieChart->first == NULL) pieChart->first = newPieData;
    else
    {
        PieData *curPieData = pieChart->first;
        while ((curPieData->next) !=NULL) curPieData = curPieData->next;
        curPieData->next = newPieData;
     }
    return pieChart;
}

int getPieChartDataCount(PieChart *pieChart)
{
    PieData *curPieData = pieChart->first;
    if (curPieData == NULL) return 0;
    int cpt=1;
    while(curPieData->next != NULL)
    {
        cpt++;
        curPieData=curPieData->next;
    }
    return cpt;
}
int checkPieChartIntegrity(PieChart *pieChart)
{
    PieData *curPieData = pieChart->first;
    if (curPieData==NULL) return 2;
    while (curPieData !=NULL)
    {
        if (curPieData->valeur <=0 ) return 1; // non conforme
        curPieData = curPieData->next;
    }
    return 0;
}
double calcRatioPourcent(PieChart *pieChart) //calcule le ratio à appliquer pour avoir un total de 100(pour cents)
{
    double total = 0.0;
    PieData *curPieData = pieChart->first;
    while (curPieData !=NULL)
    {
        total += curPieData->valeur;
        curPieData = curPieData->next;
    }
    return 100.0 / total;
}

double getMaxPieChartValue(PieChart *pieChart)
{
    PieData *curPieData = pieChart->first;
    double max=0;
    while (curPieData !=NULL)
    {
        if(curPieData->valeur > max) max = curPieData->valeur;
        curPieData = curPieData->next;
    }
    return max;
}

int openTagXML(FILE *fp, char *nomChamp, int level,char *chaineRetour)
{
    char chaineLue[255]="";
    char chaineCmp[255]="";
    char tabs[10]="";
    for (int i=0;i<level;i++){
        strcat(tabs,"\t");
    }

    fgets(chaineLue,255,fp);
    sprintf(chaineCmp,"%s<%s>\n",tabs,nomChamp);
    strcpy(chaineRetour,chaineLue);
    if(strcmp(chaineLue,chaineCmp)!=0) return 1;
    return 0;
}

int closeTagXML(FILE *fp, char *nomChamp, int level, char *chaineLue)
{
    char chaineCmp[255]="";
    char tabs[10]="";
    for (int i=0;i<level;i++){
        strcat(tabs,"\t");
    }

    fgets(chaineLue,255,fp);
    sprintf(chaineCmp,"%s</%s>\n",tabs,nomChamp);
    if(strcmp(chaineLue,chaineCmp)!=0) return 1;
    return 0;
}

int parseXML(FILE *fp,char *nomChamp, int level, char *result, char *chaineLue){
    char chaineCmp[255]="";
    char tabs[10]="";
    for (int i=0;i<level;i++){
        strcat(tabs,"\t");
    }

    fgets(chaineLue,255,fp);
    sprintf(chaineCmp,"%s<%s>\n",tabs,nomChamp);
    if(strcmp(chaineLue,chaineCmp)!=0) return 1;
    fgets(chaineLue,255,fp);
    strncpy(result,chaineLue+level+1, strlen(chaineLue)-level-1 );
    result[strlen(chaineLue)-level-2]='\0';
    fgets(chaineLue,255,fp);
    sprintf(chaineCmp,"%s</%s>\n",tabs,nomChamp);
    if(strcmp(chaineLue,chaineCmp)!=0) return 1;


    return 0;
}

PieChart *parseError(PieChart *pieChart, char *chaineLue,int ligne){
    printf("\n** Parse Error ligne %d\n",ligne);
    printf("Chaine Lue '%s'\n",chaineLue);
    return NULL;
}

PieChart *readDataFile(PieChart *pieChart,char *ficIn)
{
    printf("\nOuverture de %s...\n",ficIn);
    FILE *fp = fopen(ficIn,"r");
    if (fp == NULL)
    {
        printf("\n*** Erreur lors de l'ouverture de %s\n",ficIn);
        return NULL;
    }
    char chaineLue[255];
    char label[255];
    char valeurStr[20];
    double valeur;
    int cpt=1;
    fgets(chaineLue,255,fp);
    if(strcmp(chaineLue,"<datas>\n") !=0 ) return parseError(pieChart, chaineLue,cpt);
    do{
        cpt++;
        if (openTagXML(fp,"data",1, chaineLue) != 0){
            if( strcmp(chaineLue,"</datas>\n" ) == 0) break; // Fin de parsing
            else return parseError(pieChart,chaineLue,cpt);
        }
        cpt++;
        if (parseXML(fp,"label",2,label,chaineLue) !=0) return parseError(pieChart,chaineLue,cpt);
        cpt+=3;
        if (parseXML(fp,"valeur",2,valeurStr,chaineLue) !=0) return parseError(pieChart,chaineLue,cpt);
        if (strtod(valeurStr,NULL) == 0) return parseError(pieChart,chaineLue,cpt);
        cpt+=3;
        if (closeTagXML(fp,"data",1, chaineLue)!=0) return parseError(pieChart,chaineLue,cpt);
        pieChart = appendPieData(pieChart,label,strtod(valeurStr,NULL));

    } while (strcmp(chaineLue,"</Datas>\n" ) !=0 );
    fclose(fp);
    printf("\n...Ok\n");
    return pieChart;
}

void displayHelp(char *execName)
{
    printf("\n** Usage : %s [-o outputFileName] [-f inputFileName] [-t title] [-d] [-i] [-h] label1 valeur1 label2 valeur2 ...\n",execName);
    printf("\tSi l'option -f est présente, les arguments label1 valeur1 ... seront ignorés\n\n");
    printf("\t -o\t\tEnregistre dans le fichier. Défault : pieChart.png\n");
    printf("\t -f\t\tLit les données dans le fichier au format XML\n");
    printf("\t\t\t <datas>\n");
    printf("\t\t\t\t<data>\n");
    printf("\t\t\t\t\t<label>\n");
    printf("\t\t\t\t\t\tlabel1\n");
    printf("\t\t\t\t\t</label>\n");
    printf("\t\t\t\t\t<valeur>\n");
    printf("\t\t\t\t\t\tvaleur1\n");
    printf("\t\t\t\t\t</valeur>\n");
    printf("\t\t\t\t</data>\n");
    printf("\t\t\t\t...\n");
    printf("\t\t\t</datas>\n");
    printf("\t -t\t\tTitre du graphique\n");
    printf("\t -d\t\tDisplay : affiche le graphique\n");
    printf("\t -i\t\tHistogramme - Par défaut graphique de type pie (camembert)\n");
    printf("\n");
}

int main (int argc, char **argv)
{

    char *titreOpt = NULL;
    char *ficOutOpt = NULL;
    char *ficInOpt = NULL;
    char titre[255];
    char ficOut[255];
    int display = 0;
    int histo = 0;
    int help = 0;
    int c, index;

    opterr = 0;
    // lecture des options -t titre -o outputfile -d (display) -f inputFile -h (help) -i (histogramme)

    while( (c = getopt (argc, argv,"dt:o:f:hi")) != -1)
    {
        switch ((char)c)
        {
            case 'd' :
                display = 1;
            break;
            case 't' :
                titreOpt = optarg;
            break;
            case 'o' :
                ficOutOpt = optarg;
            break;
            case 'f':
                ficInOpt = optarg;
            break;
            case 'i':
                histo = 1;
            break;
            case 'h':
                help = 1;
            break;
            case '?':
                if ((optopt == 't')||(optopt == 'o'))
                      fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                  fprintf (stderr,
                           "Unknown option character `\\x%x'.\n",
                           optopt);
                return 1;
          default:
            abort ();
        }

    }
    if (help)
    {
        displayHelp(argv[0]);
        return 0;
    }
    if (titreOpt) strncpy(titre,titreOpt,254); else strcpy(titre,"Mon graphique");
    if (ficOutOpt) strncpy(ficOut,ficOutOpt,254); else strcpy(ficOut,"pieChart.png");
    PieChart *pieChart = newPieChart(titre);
    if (histo) pieChart->type = TYPE_HISTO;

    if (ficInOpt) // lecture des datas dans fichier formaté xml
    {
        pieChart = readDataFile(pieChart,ficInOpt);
        if(pieChart == NULL) return 1;
    }
    else // lecture des datas dans la ligne de commande
    {
        if(argc - optind<3){
            printf("\nUsage : %s -t titre -o output filename label valeur [label valeur ...]\n", argv[0]);
            return 1;
        }

        for (int i=optind;i<argc ;i+=2)
        {
             if (strtod(argv[i+1],NULL) ==  0)
            {
                printf("\nParse error : param %d\n",i);
                printf("\nUsage : %s titre label valeur [...]\n",argv[0]);
                return 1;
            }
            pieChart= appendPieData(pieChart,argv[i],strtod(argv[i+1],NULL));

        }
    }

    if (checkPieChartIntegrity(pieChart) != 0){
        printf("\n** Erreur : Les datas contiennent des valeurs non conformes\n");
        return 1;
    }

    // Création de l'image
    gdImagePtr im = gdImageCreate(IMAGE_SIZE,IMAGE_SIZE+H_TITRE);


    // Colors

    int colors[16];
    int colorBlack=gdImageColorAllocate(im,0,0,0);
    int colorRed=gdImageColorAllocate(im,255,0,0);
    int colorGreen=gdImageColorAllocate(im,0,255,0);
    int colorBlue=gdImageColorAllocate(im,80,80,255);
    int colorWhite=gdImageColorAllocate(im,255,255,255);
    int colorYellow=gdImageColorAllocate(im,240,240,0);
    int colorOrange=gdImageColorAllocate(im,255,128,0);
    int colorViolet=gdImageColorAllocate(im,255,0,255);
    int colorGray=gdImageColorAllocate(im,200,200,200);
    int colorDarkGray=gdImageColorAllocate(im,30,30,30);
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
    colors[indiceColor++]=colorDarkGray;


    // Fonts
    gdFontPtr fonts[5];
    fonts[0] = gdFontGetTiny ();
    fonts[1] = gdFontGetSmall ();
    fonts[2] = gdFontGetMediumBold ();
    fonts[3] = gdFontGetLarge ();
    fonts[4] = gdFontGetGiant ();

    // Titre
    gdImageString(im, fonts[4],(IMAGE_SIZE - strlen(titre) * 10) /2,H_TITRE/2 ,(unsigned char *)titre ,colorWhite);

    // focus sur la zone de dessin
    gdImageFilledRectangle(im,1,H_TITRE,IMAGE_SIZE,IMAGE_SIZE+H_TITRE,colorDarkGray);


     // Pie

    if(pieChart->type == TYPE_PIE)
    {
        int xc = IMAGE_SIZE /2;
        int yc = (IMAGE_SIZE + H_TITRE) /2;
        double ratio = 0.5;
        int w = IMAGE_SIZE * ratio;
        int h = IMAGE_SIZE * ratio;
        int textRadius = w /2 * 1.2;
        int percentRadius = w * 0.4;
        double ratioAngle = calcRatioPourcent(pieChart);
        double curAngle = 0;
        char label[256];
        PieData *curPieData = pieChart->first;
        int colorIndex=1;
        while(curPieData != NULL)
        {
            int s = (int) round(curAngle);
            int e = (int) round(curAngle  + curPieData->valeur * ratioAngle * 360.0 /100.0);
            gdImageFilledArc(im,xc,yc,w,h,(int)s,(int)e,colors[colorIndex],0);
            // label
            int xText = (int) (xc + textRadius * cos ( (e + s)/2 * M_PI/180 ));
            int yText = (int) (yc + textRadius * sin ( (e + s)/2 * M_PI/180 ));
            if((((e+s)/2)>90) && (((e+s)/2)<270)){  // partie gauche du camembert
                xText -= strlen((char *)curPieData->label)*8; // Décalage du texte vers la gauche pour avoir un affichage plus homogène
            }
            gdImageString(im, fonts[2],(int)xText,(int)yText,(unsigned char *)curPieData->label,colors[colorIndex]);

            //Pourcentages
            xText = (int) (xc + percentRadius * cos ( (e + s)/2 * M_PI/180 ));
            if((((e+s)/2)>270) || (((e+s)/2)<90)){  // partie droite du camembert
                xText -= 30; // Décalage du texte vers la gauche pour avoir un affichage plus homogène
            }
            yText = (int) (yc + percentRadius * sin ( (e + s)/2 * M_PI/180 ));
            sprintf(label,"%.2f%%",curPieData->valeur * ratioAngle);
            gdImageString(im, fonts[2],(int)xText,(int)yText,(unsigned char *)label ,colorBlack);
            curAngle = (double) e;
            colorIndex++;
            curPieData = curPieData->next;

        }
    }
    // Histo
    else if (pieChart->type == TYPE_HISTO)
    {
        double maxVal= getMaxPieChartValue(pieChart);
        int nbDatas = getPieChartDataCount(pieChart);
        if(nbDatas ==0) return 1;
        int marginH = IMAGE_SIZE * 0.1; // margin left et margin right
        int marginTop = IMAGE_SIZE * 0.1;
        int textZoneHeight = 200; // hauteur de l'espace destiné à écrire les labels
        int graphicWidth = IMAGE_SIZE - 2 * marginH;
        int graphicHeight = IMAGE_SIZE - marginTop - textZoneHeight;
        int originY = H_TITRE + graphicHeight + marginTop;
        int originX = marginH;
        int largeurRectangle = graphicWidth / nbDatas;
        gdImageLine(im,originX, originY,originX+graphicWidth,originY,colorWhite);
        gdImageLine(im,originX,originY,originX,originY-graphicHeight,colorWhite);
        PieData *curPieData = pieChart->first;
        int colorIndex=1;
        int rectStartX = originX+1;
        double yFactor = graphicHeight / maxVal;
        char label[20];
        while(curPieData != NULL)
        {
            int hauteur = (int) (yFactor * curPieData->valeur);
            gdImageFilledRectangle(im,rectStartX, originY-1,rectStartX + largeurRectangle, originY - hauteur ,colors[colorIndex]);
            gdImageStringUp(im, fonts[2],rectStartX + largeurRectangle /2, originY + strlen(curPieData->label) * 8,(unsigned char *)curPieData->label, colors[colorIndex]);
            sprintf(label,"%.2f",curPieData->valeur);
            gdImageString(im,fonts[2], rectStartX + largeurRectangle /2, originY - hauteur - marginTop /2,(unsigned char *)label, colorWhite);
            curPieData =curPieData->next;
            rectStartX+=largeurRectangle;
            colorIndex++;
        }

    }


    // enregistrement
    FILE *out = fopen(ficOut,"wb");
    printf("\nEnregistrement dans %s\n",ficOut);
    gdImagePng(im,out);
    fclose(out);
    if (display)
    {
        char commande[300];
        sprintf(commande, "display %s &",ficOut);
        int status = system(commande);
        return status;
     }
    return 0;
}
