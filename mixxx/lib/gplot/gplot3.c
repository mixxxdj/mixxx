/*
  Copyright (c) Ed. Breen
  All rights reserved.
  
 
   EiC's interface to gnuplot. Ed. Breen

   Example usages: just cut and paste the following
      lines, one at a time, into your EiC session, but
      leave out the EiC #> bit of course:

      EiC 1> #include MathStats/randg.c
      EiC 2> #include gnuplot/gplot.h
      EiC 3> #include gnuplot/gplot3.c
      EiC 4> plot_t *p1 = openPlot("p1");
      EiC 5> send2plot(p1,"plot sin(x) + tan(x)");
      EiC 6> int i; float f[100];
      EiC 7> for(i=0;i<100;i++) f[i] = randg(50,10);
      EiC 8> plotData(f,100,p1,plotFloats);
      EiC 9> send2plot(p1,"f(x) = 10 * cos(x/2)+ 50;  replot f(x)");
      EiC 10> for(i=0;i<100;i++) f[i] = i;
      EiC 11> replotData(f,100,p1,plotFloats);
      EiC 12> closePlot(p1);

*/   

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>

#ifdef _EiC
#include "gnuplot/gplot.h"
#else
#include "gplot.h"
#endif

static float Plot_jitter = 0.01;


static int compareFloats(void const *f1, void const  *f2)
{
    if(*(float*)f1 > *(float*)f2)
	return 1;
    if(*(float*)f1 < *(float*)f2)
	return -1;
    return 0;
}
    
void Send_2_plot_(plot_t *p, char * fmt, ...)
{
    char buff[1024];
    va_list args;
    va_start(args,fmt);
    vsprintf(buff,fmt,args);

    fputs(buff,getPlotFile(p));
    fputc('\n',getPlotFile(p));
    
    va_end(args);
}

void setPlotJitter(float v)
{
    Plot_jitter = v;
}

float getPlotJitter(void)
{
    return Plot_jitter;
}

void plotxy(float *x, float * y,int n,plot_t *p)
{
    int i;
    float miny,maxy,minx,maxx, xr, yr;
    FILE *fp;

    fp = fopen(p->fname,"w");
    miny = maxy = *y;
    minx = maxx = *x;
    for(i=0;i<n;++i,++y,++x) {
        if(*y < miny)
            miny = *y;
        if(*y > maxy)
            maxy = *y;

        if(*x < minx)
            minx = *x;
        if(*x > maxx)
            maxx = *x;

        fprintf(fp,"%g %g\n",*x,*y);
    }
    fclose(fp);

    xr = (maxx - minx)/n;
    yr = (maxy - miny)/n;

    minx -= xr;
    maxx += xr;
    miny -= yr;
    maxy += yr;
    
    Send_2_plot_(p,"plot [%g:%g] [%g:%g] '%s' with %s",
		 minx,maxx,miny,maxy,p->fname,
		 p->linetype);
}


float plotChars(void *xd, int idx, float *min, float *max)
{
    int *id = xd;
    if(id[idx] < *min)
	*min = id[idx];
    if(id[idx] > *max)
	*max = id[idx];
    return id[idx];
}

float plotInts(void *xd, int idx, float *min, float *max)
{
    int *id = xd;
    if(id[idx] < *min)
	*min = id[idx];
    if(id[idx] > *max)
	*max = id[idx];
    return id[idx];
}


float plotFloats(void *xd, int idx,float *min, float *max)
{
    float *id = xd;
    if(id[idx] < *min)
	*min = id[idx];
    if(id[idx] > *max)
	*max = id[idx];
    return id[idx];
}

void plotData(void *hist,
	      int n,
	      plot_t *p,
	      float pltfunc(void *xd,int idx,float *min,float *max))
{
    int i;
    float min = FLT_MAX,max = FLT_MIN;
    FILE *fp;
    fp = fopen(p->fname,"w");
    for(i=0;i<n;++i)
        fprintf(fp,"%d %g\n",i,pltfunc(hist,i,&min,&max));
    fclose(fp);
    Send_2_plot_(p,"plot [0:%d] [%g:%g] '%s' with %s",
		 n,min,max,p->fname,
		 p->linetype);
}


void replotData(void *hist,
	      int n,
	      plot_t *p,
	      float pltfunc(void *xd,int idx,float *min,float *max))
{
    int i;
    float min = FLT_MAX,max = FLT_MIN;
    FILE *fp;
    char *name = tmpnam(NULL);
    fp = fopen(name,"w");
    for(i=0;i<n;++i)
        fprintf(fp,"%d %g\n",i,pltfunc(hist,i,&min,&max));
    fclose(fp);
    Send_2_plot_(p,"replot '%s' with %s",
		 name,
		 p->linetype);
}

void replotxy(float *x, float * y,int n,plot_t *p)
{
    int i;
    FILE *fp;
    char *name = tmpnam(NULL);
    fp = fopen(name,"w");
    for(i=0;i<n;++i,++y,++x) {
        fprintf(fp,"%g %g\n",*x,*y);
    }
    fclose(fp);
    Send_2_plot_(p,"replot '%s' with %s",
		 name,
		 p->linetype);
}

void scatPlot(plot_t *p, int n, ...)
{
    /* do Line scatter plots.
     *  This function expects `n' arrays of
     *  floats.
     *
     *  example1:  scatPlot(p1,1, a1, n1);
     *  where
     *        a1 is a pointer of an array of float
     *	      with n1 elements.
     *
     *  example2:  scatPlot(p1,2,
     *                      a1,n1,
     *                      a2, n2);
     *
     */	     
       
    va_list  ap;
    FILE *fp;
    char buf[100];
    int  nX;
    float *aX, min, max;
    int i, j;

//    float randg(float,float);
    
    va_start (ap, n);
    fp = fopen(p->fname,"w");

    min = FLT_MAX;
    max = FLT_MIN;
    for(j=1; j <= n; j++) {
	aX = va_arg(ap, float *);
	nX = va_arg(ap, int );
	for(i=0;i<nX;++i,++aX) {
	    if(*aX < min)
		min = *aX;
	    if(*aX > max)
		max = *aX;
//	    fprintf(fp,"%g %g\n",j+randg(0,Plot_jitter),*aX);
	}
    }
    fclose(fp);
    sprintf(buf,"plot [0:%d] [%g:%g] '%s' with points",j,min,max,p->fname);
    send2plot(p,buf);

    va_end (ap);
}    

void boxPlot(plot_t *p, /* plot */
	     int n,     /* number of arrays */
	     ...)       /* alternating triplets of arrays, elements and titles */
{
    /* do box Plots          
     * This function expects `n' arrays of floats.  For
     * each array, a box plot is drawn. The boxed region represents
     * the middle 50% of the data, while the whiskers, at either end
     * of the boxed region, stretch out to cover 100% of the data. The
     * middle of the boxed region is the location of the data's
     * median.  The 5% and 95% positions are also given and are
     * displayed with a dash along the whiskers.
     *
     *  Example (drawn horizontally):
     *
     *      5%     +--------------------+   95%
     *  ----|------|                    |----|------
     *             |--------------------|
     *            25%                  75%
     *
     *  Usage1:  boxPlot(p1,1, a1, n1,"title");
     *  where
     *       a1 is a pointer to an array of floats
     *       with n1 elements.
     *
     *  Usage2: boxPlot(p1,2, a1,n1, "title 1",a2, n2,"t2");
     *
     * */

    va_list  ap;
    char buff[512] = {0}, *cp;

    FILE *fp;
    float *aX, *sp, min, max, gap = 0.25, g2 = gap/4.0;
    int i1, i2, i3, j,nX,ln;

    va_start (ap, n);

    fp = fopen(p->fname,"w");

    min = FLT_MAX;
    max = FLT_MIN;
    for(j=1; j <= n; j++) {
	aX = va_arg(ap, float *);
	nX = va_arg(ap, int );
	cp = va_arg(ap, char *);

	if(cp != NULL) {
	    ln = strlen(buff);
	    if(j > 1)
		buff[ln++] = ',';
	    buff[ln] = ' ';
	    sprintf(&buff[ln+1],"\"%s\" %d",cp,j);
	}
	    
	sp = malloc(sizeof(float) * nX);
	memcpy(sp,aX,nX*sizeof(float));

	/* sort data */
	qsort(sp,nX,sizeof(float),compareFloats);

	i1 = (nX/4.0+0.5);
	i2 = (nX/2.0+0.5);
	i3 = (3 * nX/4.0 + 0.5);

	if(sp[0] < min)
	    min =  sp[0]; /*sp[i1];*/
	if(sp[nX-1] > max)
	    max = sp[nX-1]; /*sp[i3];*/

	fprintf(fp,"%d %g %g %g %g %g \n",
		j, sp[i2],   /* x,y */
		j-gap,j+gap, /* xlow, xhigh */
		sp[i1], sp[i3]); /*ylow, yhigh */
	
	Send_2_plot_(p,"set arrow from %d,%g to %d,%g nohead\n",
		     j,sp[0], j, sp[i1]);
	Send_2_plot_(p,"set arrow from %g,%g to %g,%g nohead\n",
		     j-g2,sp[(int)(nX*0.05)], j+g2, sp[(int)(nX*0.05)]);


	Send_2_plot_(p,"set arrow from %d,%g to %d,%g nohead\n",
		     j,sp[nX-1], j, sp[i3]);
	Send_2_plot_(p,"set arrow from %g,%g to %g,%g nohead\n",
		     j-g2,sp[(int)(nX*0.95)], j+g2, sp[(int)(nX*0.95)]);


	free(sp);
    }
    fclose(fp);

    if(max > 0)
	gap = max/20.0;
    else
	gap = -max/20.0;
    
    if(buff[0] != '\0') {
	printf("buff = [%s]\n",buff);
	Send_2_plot_(p,"set xtics (%s)",buff);
    }
    Send_2_plot_(p,"plot [0:%d] [%g:%g] '%s' with "
	    "boxxy; set noarrow; set xtics",j,min-gap,max+gap,p->fname);
    
    va_end (ap);
}    



void * closePlot(plot_t *p)
{
    if(p != NULL) {
        pclose(p->fp);
        remove(p->fname);
        free(p);
    }
    return NULL;
}


plot_t * openPlot(char * plotname)
{
    plot_t * plot = (plot_t*)calloc(sizeof(plot_t),1);

    if(plot != NULL) {
        plot->fp = popen("gnuplot","w");
        setvbuf(plot->fp,NULL,_IOLBF,0);
	setFontSize(plot,12); /* 12 point */
        if(plotname != NULL) {
	    Send_2_plot_(plot,"set title '%s'; set nokey",plotname);
        }
	
        tmpnam(plot->fname);
	setLineType(plot,"lines");
    }
    return plot;

}

plot_t * openmPlot(char * plotname,
		   int nrow,
		   int ncol)
{
    plot_t * plot = openPlot(plotname);
    if(plot != NULL) {
	plot->ncol = ncol;
	plot->nrow = nrow;
	send2plot(plot,"set multiplot");
    }
    return plot;
}


void ready4Output(plot_t *p1, char *filename, char *mode)
{
  
  Send_2_plot_(p1,"set term post %s  %d",mode, getFontSize(p1));
  Send_2_plot_(p1,"set output \'%s\'",filename);
}



void savePlot(plot_t *p1, char *fname, char * mode)
{
    /* save plot as a postscript file of name `fname'
     * mode = landscape, portrait, eps or default
     */
    Send_2_plot_(p1,"set term post %s  %d",mode, getFontSize(p1));
    Send_2_plot_(p1,"set output \'%s\'",fname);
    send2plot(p1,"replot");
    send2plot(p1,"set term x11");
    send2plot(p1,"replot");
}

void printPlot(plot_t *p1)
{
    savePlot(p1,"printfile.ps","landscape");
    system("lpr printfile.ps");
    remove("printfile.ps");
}

void selectPlot(plot_t *p1,    /* plot */
		int N)          /* plot number */
{
    float xp, yp, xinc, yinc;
    char buf[512];
    xinc = 1.0/getPlotncol(p1);
    yinc = 1.0/getPlotnrow(p1);
    N--;
    yp = 1 - N/getPlotncol(p1) * yinc - yinc;
    xp = N%getPlotncol(p1);
    xp *= xinc;
    sprintf(buf,"set size %g, %g; set origin %g, %g",
	    xinc, yinc,xp,yp);
    send2plot(p1,buf);
}


void clearmPlot(plot_t *p1,    /* plot */
		int N)          /* plot number */
{
    selectPlot(p1,N);
    send2plot(p1,"clear");
}

void dataMPlot(plot_t *p,
	       char * (*f)(),  /* return the name of the file where */
			       /* data is stored */
	       int nv,         /* number of vectors in vecs */
	       void **vecs,    /* array of vectors */
	       int *lens)      /* array of lengths to vecs */	
{
    /*
       Plot multiply data arrays to one display.
       
       Assumes that plot title, line style etc  have been
       handled elsewhere.
    */
    char *buff;
    char buf[256];
    int blen = 256;
    char **names = malloc(nv * sizeof(char*));    
    char *tmp;
    int i,k, tl = 0;
    
    for(i=0;i<nv;i++) {
	tmp = f(vecs[i],lens[i]);
	k = strlen(tmp);
	names[i] = malloc(k+1);
	tl += k + 5;
	strcpy(names[i],tmp);
    }
    buff = malloc(blen+tl);
    
    sprintf(buff,"plot '%s'",names[0]);
    free(names[0]);
    for(i=1;i<nv; i++) {
        sprintf(buf,",'%s'",names[i]);
        strcat(buff,buf);
	free(names[i]);
    }
    send2plot(p,buff);
    free(names);
    free(buff);
}

#undef _POSIX_SOURCE
