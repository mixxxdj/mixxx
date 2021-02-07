// Wrapper for FidLib Filter Library

#include "engine/filters/enginefilter.h"

#include <QtDebug>

#include "moc_enginefilter.cpp"

EngineFilter::EngineFilter(char* conf, int predefinedType)
        : iir(0),
          fir(0),
          tmp(0),
          ff(nullptr),
          funcp(nullptr),
          run(nullptr) {
    switch(predefinedType)
    {
    case PREDEF_BP:
        processSample = &processSampleBp;
        fbuf1 = buf1;
        fbuf2 = buf2;
        break;
    case PREDEF_HP:
        processSample = &processSampleHp;
        fbuf1 = buf1;
        fbuf2 = buf2;
        break;
    case PREDEF_LP:
        processSample = &processSampleLp;
        fbuf1 = buf1;
        fbuf2 = buf2;
        break;
    default:
        ff = fid_design(conf, 44100., -1., -1., 1, nullptr);
        qDebug() << "Filter " << conf << " Setup: 0x" << ff;
        run = fid_run_new(ff, &funcp);
        fbuf1 = fid_run_newbuf(run);
        fbuf2 = fid_run_newbuf(run);
        processSample = funcp;
    }
    int i;
    for (i = 0; i < FILTER_BUF_SIZE; i++) {
        buf1[i] = buf2[i] = 0;
    }
}

EngineFilter::~EngineFilter()
{
    if(processSample == funcp) //if we used fidlib
    {
        fid_run_freebuf(fbuf2);
        fid_run_freebuf(fbuf1);
        fid_run_free(run);
        free(ff);
    }
}



void EngineFilter::process(CSAMPLE* pInOut, const int iBufferSize)
{
    int i;
    for(i = 0; i < iBufferSize; i += 2)
    {
        pInOut[i] = (CSAMPLE) processSample(fbuf1, (double) pInOut[i]);
        pInOut[i + 1] = (CSAMPLE) processSample(fbuf2, (double) pInOut[i + 1]);
    }
}


// 250Hz-3Khz Butterworth
double processSampleBp(void *bufIn, const double sample)
{
    double *buf = (double*) bufIn;
    double val = sample;
   double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 15*sizeof(double));
   // use 8.73843261546594e-007 below for unity gain at 100% level
   iir= val * 8.738432615466217e-007;
   iir -= 0.8716357571117795*tmp; fir= tmp;
   iir -= -1.704721971813985*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9881828644977958*tmp; fir= tmp;
   iir -= -1.986910175866046*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.6739219969192579*tmp; fir= tmp;
   iir -= -1.534687501075499*buf[4]; fir += -buf[4]-buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9644444065027918*tmp; fir= tmp;
   iir -= -1.963091971649609*buf[6]; fir += -buf[6]-buf[6];
   fir += iir;
   tmp= buf[7]; buf[7]= iir; val= fir;
   iir= val;
   iir -= 0.5508744524070673*tmp; fir= tmp;
   iir -= -1.437951258090782*buf[8]; fir += buf[8]+buf[8];
   fir += iir;
   tmp= buf[9]; buf[9]= iir; val= fir;
   iir= val;
   iir -= 0.940359918647641*tmp; fir= tmp;
   iir -= -1.938825711089784*buf[10]; fir += buf[10]+buf[10];
   fir += iir;
   tmp= buf[11]; buf[11]= iir; val= fir;
   iir= val;
   iir -= 0.494560064204263*tmp; fir= tmp;
   iir -= -1.400685123336887*buf[12]; fir += buf[12]+buf[12];
   fir += iir;
   tmp= buf[13]; buf[13]= iir; val= fir;
   iir= val;
   iir -= 0.9201106143536053*tmp; fir= tmp;
   iir -= -1.918341654664469*buf[14]; fir += buf[14]+buf[14];
   fir += iir;
   buf[15]= iir; val= fir;
   return val;
}

//3Khz butterworth
double processSampleHp(void *bufIn, const double sample)
{
    double *buf = (double*) bufIn;
    double val = sample;
    double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   // use 0.3307380993576275 below for unity gain at 100% level
   iir= val * 0.3307380993576274;
   iir -= 0.8503595356078639*tmp; fir= tmp;
   iir -= -1.683892145680763*buf[0]; fir += -buf[0]-buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.6256182051727028*tmp; fir= tmp;
   iir -= -1.479369644054996*buf[2]; fir += -buf[2]-buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.4873536896159359*tmp; fir= tmp;
   iir -= -1.35354408027022*buf[4]; fir += -buf[4]-buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.4219026276867782*tmp; fir= tmp;
   iir -= -1.2939813158517*buf[6]; fir += -buf[6]-buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}
double processSampleLp(void *bufIn, const double sample)
{
    double *buf = (double*) bufIn;
    double val = sample;
       double tmp, fir, iir;
   tmp= buf[0]; memmove(buf, buf+1, 7*sizeof(double));
   iir= val * 9.245468558718278e-015;
   iir -= 0.9862009760667707*tmp; fir= tmp;
   iir -= -1.984941152135637*buf[0]; fir += buf[0]+buf[0];
   fir += iir;
   tmp= buf[1]; buf[1]= iir; val= fir;
   iir= val;
   iir -= 0.9611983723246083*tmp; fir= tmp;
   iir -= -1.95995440725112*buf[2]; fir += buf[2]+buf[2];
   fir += iir;
   tmp= buf[3]; buf[3]= iir; val= fir;
   iir= val;
   iir -= 0.9424834072512262*tmp; fir= tmp;
   iir -= -1.941251312860088*buf[4]; fir += buf[4]+buf[4];
   fir += iir;
   tmp= buf[5]; buf[5]= iir; val= fir;
   iir= val;
   iir -= 0.9325031355986387*tmp; fir= tmp;
   iir -= -1.93127737157649*buf[6]; fir += buf[6]+buf[6];
   fir += iir;
   buf[7]= iir; val= fir;
   return val;
}
