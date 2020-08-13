// Wrapper for FidLib Filter Library
#pragma once

#include <cstdio>
#include <fidlib.h>

#include "engine/engineobject.h"
#include "util/types.h"

#define PREDEF_HP 1
#define PREDEF_BP 2
#define PREDEF_LP 3

class EngineFilter : public EngineObject {
    Q_OBJECT
  public:
    EngineFilter(char* conf, int predefinedType = 0);
    virtual ~EngineFilter();

    void process(CSAMPLE* pInOut, const int iBufferSize);

  protected:
    double iir;
    double fir;
    double tmp;
#define FILTER_BUF_SIZE 16
    double buf1[FILTER_BUF_SIZE];
    double buf2[FILTER_BUF_SIZE];

  private:
    double (*processSample)(void *buf, const double sample);

    FidFilter *ff;
    FidFunc *funcp;
    FidRun *run;
    void *fbuf1;
    void *fbuf2;
};

double processSampleDynamic(void *buf, const double sample);
double processSampleHp(void *buf, const double sample);
double processSampleBp(void *buf, const double sample);
double processSampleLp(void *buf, const double sample);
