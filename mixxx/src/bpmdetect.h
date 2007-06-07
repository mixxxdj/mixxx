#ifndef _BPM_H_INCLUDED
#define _BPM_H_INCLUDED

#include <fftw3.h>
#include <map>

#define NEED_MORE_DATA 0.0
#define INDETERMINATE -1.0

class BpmDetect
{
public:
	BpmDetect(int samplerate, unsigned long (*callback)(float *, unsigned long, void *)=0, void *data=0, unsigned long sample_count=0);
	~BpmDetect();

	void setLimits(double lower, double upper);
	double run(float *samples=0, unsigned long sample_count=0, int *current=0, int *max=0);
	
protected:
	double measure();
	void differentiate(double * arr, int s);
	double find_abs_max(double *data, long l);
	double normalize_abs_max(double *data, long l);
	void energize(double *data, double audiosize, unsigned long fs);

	unsigned long measured;
	unsigned long sample_length;
	float *block;
	double *audio;
	unsigned long b1, b2;
	double *en;
	double *ts;
	double *co;
	fftw_plan forward;
	fftw_plan backward;
	int input_rate;
	int measure_rate;
	unsigned long (*callback)(float *, unsigned long, void *);
	double lower;
	double upper;
	unsigned long buffer_fill;
	std::map<double,int> bpms;
	void *data;
};

#endif
