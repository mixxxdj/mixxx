#include "bpmdetect.h"

#ifdef __WIN__
#define _USE_MATH_DEFINES
#endif

#include <math.h>

 #ifdef __LINUX__
	#include <sys/time.h>
	#include <unistd.h>
#endif

#ifdef __WIN__
#include <winsock2.h>
int gettimeofday
      (struct timeval* tp, void* tzp) {
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = t % 1000;
    /* 0 indicates success. */
    return 0;
}
#endif


using namespace std;

static void incRange(map<double,int>& bpms, double ix)
{
	double low=ix-0.25;
	double high=ix+0.25;
	map<double,int>::iterator it;

	for(it=bpms.begin();it!=bpms.end();++it)
	{
		if((*it).first >= low && (*it).first <= high)
			(*it).second++;
	}
}

static double findMax(map<double,int>& bpms, int limit=-1)
{
	map<double,int>::iterator it;
	int max=0;
	double maxbpm=0.0;

	for(it=bpms.begin();it!=bpms.end();++it)
	{
		if((*it).second > max && (limit == -1 || (*it).second < limit))
		{
			max=(*it).second;
			maxbpm=(*it).first;
		}
	}
	return maxbpm;
}

BpmDetect::BpmDetect(int inrate, unsigned long (*cb)(float *, unsigned long, void *), void *dt, unsigned long slen)
{
	buffer_fill=0;
	input_rate=inrate;
	measure_rate=11025;
	sample_length=slen;
	callback=cb;
	data=dt;
	block=0;

	if(!slen)
		measured=131072;
	else
	{
		unsigned long atmost=(long long)sample_length*measure_rate/inrate;
		measured=1;
		while(measured <= atmost)
			measured*=2;
		measured/=2;
		if(measured > 131072)
			measured=131072;
	}
	audio=new double[measured];
	en=new double[measured];
	ts=new double[measured];
	co=new double[measured];

	char wisdom[1024];
	sprintf(wisdom, "%s/.BpmWisdom", getenv("HOME"));
	FILE *wisdom_file=fopen(wisdom, "r");
	if(wisdom_file)
	{
		fftw_import_wisdom_from_file(wisdom_file);
		fclose(wisdom_file);
	}

	forward = fftw_plan_r2r_1d(measured, &(audio[0]), en, FFTW_R2HC, FFTW_MEASURE);
	backward = fftw_plan_r2r_1d(measured, ts, co, FFTW_HC2R, FFTW_MEASURE);

	wisdom_file=fopen(wisdom, "w");
	if(wisdom_file)
	{
		fftw_export_wisdom_to_file(wisdom_file);
		fclose(wisdom_file);
	}

	setLimits(85.0,160.0);
}

void BpmDetect::setLimits(double lower_boundary, double higher_boundary)
{
	double freq=higher_boundary/60.0;
	b1=4*(int)(measure_rate/freq);
	freq=lower_boundary/60.0;
	b2=4*(int)(measure_rate/freq);
	lower=lower_boundary;
	upper=higher_boundary;
}

BpmDetect::~BpmDetect()
{
	delete[] audio;
	delete[] en;
	delete[] ts;
	delete[] co;
}

double BpmDetect::run(float *samples, unsigned long sample_count, int *current, int *bound)
{
	unsigned long samps=(long long)measured*input_rate/measure_rate;

	if(!samples)
	{
		bpms.clear();

		if(!callback)
			return INDETERMINATE;

		block=new float[samps*2];

		while(1)
		{
			while(buffer_fill < samps)
			{
				unsigned long returned=(*callback)(block+buffer_fill*2, samps-buffer_fill, data);
				if(returned == 0)
				{
					delete[] block;
					block=0;
					buffer_fill=0;
					bpms.clear();
					return INDETERMINATE;
				}
				if(returned > (samps-buffer_fill))
					returned=samps-buffer_fill;
				buffer_fill+=returned;
			}

			double bpm=measure();

			if(bpm == 0.0)
			{
				buffer_fill=0;
				continue;
			}

			bpms[bpm];
			incRange(bpms, bpm);
			double top=findMax(bpms);
			int next_to_last=bpms[findMax(bpms, bpms[top])];
			if(next_to_last < 2)
				next_to_last=2;
			printf("%g\t%d\t%d (%g)\n",top,next_to_last,bpms[top], bpm);
			if(bpms[top] >= next_to_last*2 && bpms[top]>5)
			{
				delete[] block;
				block=0;
				buffer_fill=0;
				bpms.clear();
				return top;
			}
			if(bpms[top] >= 8)
			{
				delete[] block;
				block=0;
				buffer_fill=0;
				bpms.clear();
				return top;
			}

			buffer_fill=0;
		}
	}

	if(callback)
		return INDETERMINATE;
	
	if(sample_count == 0)
	{
		bpms.clear();
		return INDETERMINATE;
	}

	if(block)
		block=(float *)realloc(block, sizeof(float)*2*(buffer_fill+sample_count));
	else
		block=(float *)malloc(sizeof(float)*2*sample_count);
	
	for(int i=0;i<sample_count*2;++i)
		block[buffer_fill*2+i]=samples[i];
	buffer_fill+=sample_count;

	if(buffer_fill < samps)
		return NEED_MORE_DATA;

	double bpm=measure();

	bpms[bpm];
	incRange(bpms, bpm);

	double top=findMax(bpms);
	int next_to_last=bpms[findMax(bpms, bpms[top])];
	if(next_to_last < 2)
		next_to_last=2;
	printf("%g\t%d\t%d (%g)\n",top,next_to_last,bpms[top], bpm);
	int pass=next_to_last*2;
	if(pass < 5)
		pass=6;
	if(pass > 8)
		pass=8;
	if(bound)
		*bound=pass;
	if(current)
		*current=bpms[top];

	if(bpms[top] >= next_to_last*2 && bpms[top]>5)
	{
		free(block);
		block=0;
		buffer_fill=0;
		bpms.clear();
		return top;
	}
	if(bpms[top] >= 8)
	{
		free(block);
		block=0;
		buffer_fill=0;
		bpms.clear();
		return top;
	}

	if(buffer_fill-samps)
	{
		memmove(&block[0], &block[samps*2], (buffer_fill-samps)*2*sizeof(float));
		buffer_fill-=samps;
		block=(float *)realloc(block, sizeof(float)*2*buffer_fill);
	}
	else
	{
		free(block);
		block=0;
		buffer_fill=0;
	}
	return NEED_MORE_DATA;
}

double BpmDetect::measure()
{
	for(long long i=0;i < measured;i++)
	{
		long long posa = i*input_rate/measure_rate;
		long long posb = (i+1)*input_rate/measure_rate;
		double sum = 0;

		unsigned long samps=(long long)measured*input_rate/measure_rate;
		if(posb>=samps)
			posb=samps;

		for(unsigned long j=posa;j < posb;j++)
			sum += fabs(block[j<<1])+fabs(block[(j<<1)|1]);
		audio[i]=sum/(2*(posb-posa));
	}

	int blsi=1;
	int tmpr=measure_rate;

	while(tmpr > 50) 
	{
		blsi*=2;
		tmpr/=2;
	}

	energize(audio,measured,blsi);

	double *w=new double[measured];

	for(unsigned int x=0;x < measured;x++)
		w[x]=audio[x];

	differentiate(w,measured);

	normalize_abs_max(w,measured);

	normalize_abs_max(audio,measured);

	for(unsigned int x=0;x < measured;x++)
		audio[x]+=w[x];

	delete[] w;

	for(unsigned long x=0;x < measured;x++)
		audio[x]*= 0.5 - 0.5 * cos(M_PI*2.0*(double)x/(double)measured);

	timeval start, end;

	gettimeofday(&start, 0);
	gettimeofday(&end, 0);

	end.tv_sec-=start.tv_sec;
	end.tv_usec+=end.tv_sec*1000000;
	end.tv_usec-=start.tv_usec;
	if(end.tv_usec > 500000)
		return 0.0;

	for(unsigned long x=1;x < measured/2;x++)
		en[x]=sqrt(en[x]*en[x]+en[measured-x]*en[measured-x]);
	for(unsigned long x=1;x < measured/2;x++)
		en[x]/=measured/x;
	en[0]=0.0;

	for(unsigned long x=0;x < measured/2;x++)
		ts[x]=en[x];

	for(unsigned long x=measured/2;x < measured;x++)
		ts[x]=0;
	fftw_execute(backward);

	for(unsigned long x=1;x < measured;x++)
		co[x]/=measured-x;
	co[0]=0;

	double mf = en[0] = 0;
	int pf = 0;

	normalize_abs_max(en,measured);
	normalize_abs_max(co,measured);

	for(unsigned long x=1;x < measured/2;x++)
		co[x]*=en[measured/x];
	for(unsigned long x=b1;x < b2;x++)
	{
		int m = 1;
		double E = 0;

		while(m <= 2)
		{
			E+=co[x*m];
			E+=co[x/m];
			m*=2;
		}
		if(E > mf || mf == 0.0)
		{
			mf=E;
			pf=x;
		}
	}

	double t=4.0*measure_rate*60.0/(double)pf;
	return t;
}

void BpmDetect::differentiate(double * arr, int s)
{
	for(int i=0;i < s-1;i++)
		arr[i]=arr[i+1]-arr[i];
	arr[s-1]=-arr[s-1];
}

double BpmDetect::find_abs_max(double *data, long l)
{
	if(l==0)
		return 0.0;
	double m=data[0];
	for(long i=0;i < l;i++)
		if(fabs(data[i]) > m)
			m=fabs(data[i]);
	return m;
}

double BpmDetect::normalize_abs_max(double *data, long l)
{
	double m=find_abs_max(data,l);
	if(m > 0)
		for(long i=0;i < l;i++)
			data[i]/=m;
	return m;
}

void BpmDetect::energize(double *data, double audiosize, unsigned long fs)
{
	float *rr=new float[fs];

	for(unsigned long i=0;i < fs;i++)
		rr[i]=0;

	double M=0;

	for(unsigned long x=0;x < audiosize;x++)
	{
		M-=rr[x%fs];
		rr[x%fs]=data[x];
		M+=data[x];
		double R=M/fs;
		data[x >= fs ? x-fs : 0]-=R;
	}

	for(unsigned long x=0;x < audiosize;x++)
		data[x]*=data[x];

	for(unsigned long i=0;i < fs;i++)
		rr[i]=0;

	double S = 0;

	for(unsigned long x=0;x < audiosize;x++)
	{
		S-=rr[x%fs];
		rr[x%fs]=data[x];
		S+=data[x];
		double R=sqrt(S/fs);
		data[x >= fs ? x-fs : 0]=R;
	}
	delete[] rr;
}
