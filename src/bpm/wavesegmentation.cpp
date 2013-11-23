// wavesegmentation.cpp: implementation of the Segmentation class.
//
//////////////////////////////////////////////////////////////////////

#include "wavesegmentation.h"
#include "float.h"
#include "defs.h"

//////////////////////////////////////////////////////////////////////
// Main class for algorithm : Segmentation
//////////////////////////////////////////////////////////////////////

WaveSegmentation::WaveSegmentation()
{

}

int WaveSegmentation::Process(double * psdf,int count,float * segPoints,int maxPoints)
{

    Rhythmogram * rg=new Rhythmogram(psdf,count);
    selfsim * ss=new selfsim(rg);
    ShortestPath * sp=new ShortestPath(ss,10);

    int RetPoints = math_min(sp->length(),maxPoints);

    for(int i=0; i<RetPoints; i++)
        segPoints[i]=sp->getPoint(i);
    delete sp;
    delete ss;
    delete rg;
    return RetPoints;

}

WaveSegmentation::~WaveSegmentation()
{

}

//////////////////////////////////////////////////////////////////////
// Rhythmogram
//////////////////////////////////////////////////////////////////////

// Compute autocorrelation
void Rhythmogram::acf(double * wav, int nwav, double * acf, int nacf) {
    int i, j;
    double normMax,normMin=0.0;

    for(i = 0; i < nacf; i++) {
        double sum = 0.0;

        for(j = 0; j < nwav - i; j++)
            sum += wav[j] * wav[j + i];

        // Normalization ratio w time lag i=0
        if (!i)
            normMax=sum;

        acf[i] = sum/normMax;
    }
}

double * Rhythmogram::column(int c)
{
    return &_rhythmogram[c * r_height];
}

Rhythmogram::Rhythmogram(double * psdf, int count, double FeatureStepSize,
                         double HorzStepSize, double BlockSize, double HighRhythmIntv,
                         double LowRhythmIntv, double ZeroRhythmIntv)
{
    r_height=(long)(HighRhythmIntv/FeatureStepSize);
    r_width=(long)((count*FeatureStepSize-BlockSize)/HorzStepSize);
    _rhythmogram=new double[r_width*r_height];

    long proc_length=(long)(BlockSize/FeatureStepSize);

    for (int k=0; k<r_width; k++)
    {
        acf(&psdf[(long)(k*HorzStepSize/FeatureStepSize)],proc_length,column(k),r_height);
        for (int i=0; i<ZeroRhythmIntv/FeatureStepSize; i++)
            column(k)[i]=0;
    }
}

Rhythmogram::~Rhythmogram()
{
    delete [] _rhythmogram;
}



long Rhythmogram::height()
{
    return r_height;
}

long Rhythmogram::width()
{
    return r_width;
}


//////////////////////////////////////////////////////////////////////
// selfsimilarity computed as euclidean distance
//////////////////////////////////////////////////////////////////////
selfsim::selfsim(Rhythmogram * rg)
{
    s_size = rg->width();
    _selfsim = new double[s_size*s_size];
    for (int j=0; j<s_size; j++){
        for (int k=j; k<math_min(j+MAX_SEGMENT_SIZE+1,s_size); k++){
            double * colJ = rg->column(j);
            double * colK = rg->column(k);
            double rowsum=0;
            for (int row=0; row<rg->height(); row++){
                double d=colJ[row]-colK[row];
                rowsum += d*d;
            }
            column(j)[k]=sqrt(rowsum);
            column(k)[j]=column(j)[k];
        }
    }
    // Change to sums (weights to use for shortest path)
    int n;
    for (int k=0; k<math_min(s_size-2,MAX_SEGMENT_SIZE+2); k++)
        for(int m=k+2; m<s_size; m++){
            n=m-k-2;
            column(m)[n]+=column(m-1)[n] + column(m)[n+1] -column(m-1)[n+1];
            column(n)[m]=column(m)[n];
        }

}

selfsim::~selfsim()
{
    delete [] _selfsim;
}

double * selfsim::column(int c)
{
    return &_selfsim[c * s_size];
}

long selfsim::size()
{
    return s_size;
}

//////////////////////////////////////////////////////////////////////
// ShortestPath algo
//////////////////////////////////////////////////////////////////////


void relax(double * d, int i, int j, double a, int * py, double * dy)
{
    *py=-1;
    *dy=0.0;
    if (d[j]>d[i]+a){
        *dy=d[i]+a;
        *py=i;            // Set backtrack path
    }
}

ShortestPath::ShortestPath(selfsim * ss,double Threshold,double HorzStepSize )
{
    _HorzStepSize=HorzStepSize;
    int I  = ss->size();          //Length of sound file feature
    int * py = new int[I];
    double * d  = new double[I];
    segPoints=new int[I];

    for (int n=0; n<I; n++){   //Initialize vectors
        py[n]=-1;
        d[n]=DBL_MAX;
    }

    d[0]=0.0;

    for (int i=0; i<I-1; i++)
    {
        int limit=math_min(MAX_SEGMENT_SIZE+i,I-1);
        for (int j=i+1; j<=limit; j++)
        {
            double sum=ss->column(j)[i]*2;
            //printf("%lf\n",sum);
            int py2;
            double dy2;
            relax(d,i,j,Threshold+(1/(double)(j-i))*sum,&py2,&dy2);
            if (py2>-1){
                //printf("py[%d]=%d\n",j,py2);
                py[j]=py2;
                d[j]=dy2;
            }
        }
    }
    nPoints=0;
    segPoints[nPoints++]=I-1;
    while(py[ segPoints[nPoints-1] ] != -1){
        segPoints[nPoints]=(int)py[segPoints[nPoints-1]];        // * HorzStepSize;
        nPoints++;
    }
    segPoints[0] = segPoints[0] ;    //* HorzStepSize;
    delete [] py;
    delete [] d;
}

float ShortestPath::getPoint(int n)
{
    return (float)((double)segPoints[nPoints-n-1]*_HorzStepSize);
}

int ShortestPath::length()
{
    return nPoints;
}
ShortestPath::~ShortestPath()
{
    delete [] segPoints;
}
