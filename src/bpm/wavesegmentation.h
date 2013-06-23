// wavesegmentation.h: interface for the Segmentation class.
//
//////////////////////////////////////////////////////////////////////

#ifndef WAVESEGMENTATION_H
#define WAVESEGMENTATION_H

#include "math.h"

#ifndef min
#define min(A,B) ((A)>(B)?(B):(A))
#endif

#define MAX_SEGMENT_SIZE 250


class WaveSegmentation  
{
public:
	WaveSegmentation();
	virtual ~WaveSegmentation();
	
	int Process(double *psdf,int count,float *segPoints,int maxPoints);

};


class Rhythmogram {
private:
	double * _rhythmogram;
	long r_width;
	long r_height;
public:
	long width();
	long height();
	Rhythmogram(double *psdf, 
		        int     count,
				double  FeatureStepSize  = 0.01,
		        double  HorzStepSize     = 0.5, 
				double  BlockSize        = 8.0,
				double  HighRhythmIntv   = 2.0,
				double  LowRhythmIntv    = 0.0, 
				double  ZeroRhythmIntv   = 0.1
				 );
	double * column(int c);

	~Rhythmogram();
	static void acf(double *wav, int nwav, double *acf, int nacf);
};


class selfsim {
private:
	double * _selfsim;
	long s_size;
public:
	selfsim(Rhythmogram *rg);
	~selfsim();
	long size();
	double * column(int c);

};


class ShortestPath  
{
	int *segPoints;
	int nPoints;
	double _HorzStepSize;
public:
	ShortestPath(selfsim *ss,double Threshold,
		         double  HorzStepSize     = 0.5);
	~ShortestPath();
	int length();
	float getPoint(int n);
};


#endif
