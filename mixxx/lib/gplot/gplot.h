#ifndef GNUPLOT_
#define GNUPLOT_

typedef struct {
    FILE *fp;
    char  fname[L_tmpnam + 1];
    int fntsz;  /* font size */
    float (*func)(void *d, int idx, float *min, float *max);
    char *linetype;    
    
    /* the following are used for multiplots */
    int nrow;  /* number of rows */
    int ncol;  /* number of cols */
    int cp;    /* current plot in use */
}plot_t;

/* plot methods */
#define getPlotncol(x)  (x)->ncol
#define getPlotnrow(x)  (x)->nrow
#define getCurrentPlot(x) (x)->cp
#define getPlotFile(x)   (x)->fp
#define getLineType(x)   (x)->linetype
#define getFontSize(x)   (x)->fntsz

#define setPlotFunc(x,v)  (x)->func = v 
#define setCurrentPlot(x,v)  (x)->cp = v
#define setLineType(x,v)  (x)->linetype = v
#define setFontSize(x,v) (x)->fntsz = v
/*******************************/
#define send2plot  Send_2_plot_

#define replot(x)        Send_2_plot_(x,"replot")
#define titleplot(x,s)   Send_2_plot_(x,"set title '%s'  font '%d'",s, getFontSize(x))
#define xlabelplot(x,s)  Send_2_plot_(x,"set xlabel '%s' font '%d'",s, getFontSize(x))
#define ylabelplot(x,s)  Send_2_plot_(x,"set ylabel '%s' font '%d'",s, getFontSize(x))
#define zlabelplot(x,s)  Send_2_plot_(x,"set zlabel '%s' font '%d'",s, getFontSize(x))


#define showmPlot(x)  send2plot(x,"set nomultiplot; set multiplot") 
#define clearPlot(p)    send2plot(p,"clear")
#define nolabels(p)     send2plot(p,"set nolabel;replot")
#define plothist(x,y,z) plotHist(x,y,z)


/* function definitions */

void Send_2_plot_(plot_t *p, char * fmt, ...);
void setPlotJitter(float v);
float getPlotJitter(void);
void plotxy(float *x, float * y,int n,plot_t *p);
float plotChars(void *xd, int idx, float *min, float *max);
float plotInts(void *xd, int idx, float *min, float *max);
float plotFloats(void *xd, int idx,float *min, float *max);
void plotData(void *hist,
	      int n,
	      plot_t *p,
	      float pltfunc(void *xd,int idx,float *min,float *max));

void replotData(void *hist,
	      int n,
	      plot_t *p,
	      float pltfunc(void *xd,int idx,float *min,float *max));

void replotxy(float *x, float * y,int n,plot_t *p);
void scatPlot(plot_t *p, int n, ...);

void boxPlot(plot_t *p, /* plot */
	     int n,     /* number of arrays */
	     ...);       /* alternating triplets of arrays, elements and titles */
void * closePlot(plot_t *p);
plot_t * openPlot(char * plotname);
plot_t * openmPlot(char * plotname,
		   int nrow,
		   int ncol);

void savePlot(plot_t *p1, char *fname, char * mode);
void printPlot(plot_t *p1);
void selectPlot(plot_t *p1,    /* plot */
		int N);          /* plot number */
void clearmPlot(plot_t *p1,    /* plot */
		int N);          /* plot number */
void dataMPlot(plot_t *p,
	       char * (*f)(),  /* return the name of the file where */
			       /* data is stored */
	       int nv,         /* number of vectors in vecs */
	       void **vecs,    /* array of vectors */
	       int *lens);      /* array of lengths to vecs */	

#endif





