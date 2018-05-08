#ifndef KISS_FTR_H
#define KISS_FTR_H

#include "kiss_fft.h"

#ifndef KISSFFT_USE_CPP_LINKAGE
#ifdef __cplusplus
extern "C" {
#endif
#endif

    
/* 
 
 Real optimized version can save about 45% cpu time vs. complex fft of a real seq.

 
 
 */

typedef struct kiss_fftr_state *kiss_fftr_cfg;


kiss_fftr_cfg kiss_fftr_alloc(int nfft,int inverse_fft,void * mem, size_t * lenmem);
/*
 nfft must be even

 If you don't care to allocate space, use mem = lenmem = NULL 
*/


void kiss_fftr(kiss_fftr_cfg cfg,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata);
/*
 input timedata has nfft scalar points
 output freqdata has nfft/2+1 complex points
*/

void kiss_fftri(kiss_fftr_cfg cfg,const kiss_fft_cpx *freqdata,kiss_fft_scalar *timedata);
/*
 input freqdata has  nfft/2+1 complex points
 output timedata has nfft scalar points
*/

#define kiss_fftr_free free

#ifndef KISSFFT_USE_CPP_LINKAGE
#ifdef __cplusplus
}
#endif
#endif

#ifdef KISSFFT_USE_CPP_LINKAGE
#define KISSFFT_USED_CPP_LINKAGE 1
#endif

#endif
