/*
Copyright (c) 2003-2004, Mark Borgerding

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "kiss_fftr.h"
#include "_kiss_fft_guts.h"

struct kiss_fftr_state{
    kiss_fft_cfg substate;
    kiss_fft_cpx * tmpbuf;
    kiss_fft_cpx * super_twiddles;
};

kiss_fftr_cfg kiss_fftr_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem)
{
    int i;
    kiss_fftr_cfg st = NULL;
    size_t subsize, memneeded;

    if (nfft & 1) {
        fprintf(stderr,"Real FFT optimization must be even.\n");
        return NULL;
    }
    nfft >>= 1;

    kiss_fft_alloc (nfft, inverse_fft, NULL, &subsize);
    memneeded = sizeof(struct kiss_fftr_state) + subsize + sizeof(kiss_fft_cpx) * ( nfft * 2);

    if (lenmem == NULL) {
        st = (kiss_fftr_cfg) malloc (memneeded);
    } else {
        if (*lenmem >= memneeded)
            st = (kiss_fftr_cfg) mem;
        *lenmem = memneeded;
    }
    if (!st)
        return NULL;

    st->substate = (kiss_fft_cfg) (st + 1); /*just beyond kiss_fftr_state struct */
    st->tmpbuf = (kiss_fft_cpx *) (((char *) st->substate) + subsize);
    st->super_twiddles = st->tmpbuf + nfft;
    kiss_fft_alloc(nfft, inverse_fft, st->substate, &subsize);

    for (i = 0; i < nfft; ++i) {
        double phase =
            -3.14159265358979323846264338327 * ((double) i / nfft + .5);
        if (inverse_fft)
            phase *= -1;
        kf_cexp (st->super_twiddles+i,phase);
    }
    return st;
}

void kiss_fftr(kiss_fftr_cfg st,const kiss_fft_scalar *timedata,kiss_fft_cpx *freqdata)
{
    /* input buffer timedata is stored row-wise */
    int k,N;

    if ( st->substate->inverse) {
        fprintf(stderr,"kiss fft usage error: improper alloc\n");
        exit(1);
    }

    N = st->substate->nfft;

    /*perform the parallel fft of two real signals packed in real,imag*/
    kiss_fft( st->substate , (const kiss_fft_cpx*)timedata, st->tmpbuf );
 
    freqdata[0].r = st->tmpbuf[0].r + st->tmpbuf[0].i;
    freqdata[0].i = 0;
    C_FIXDIV(freqdata[0],2);

    for (k=1;k <= N/2 ; ++k ) {
        kiss_fft_cpx fpnk,fpk,f1k,f2k,tw;

        fpk = st->tmpbuf[k]; 
        fpnk.r =  st->tmpbuf[N-k].r;
        fpnk.i = -st->tmpbuf[N-k].i;
        C_FIXDIV(fpk,2);
        C_FIXDIV(fpnk,2);

        C_ADD( f1k, fpk , fpnk );
        C_SUB( f2k, fpk , fpnk );
        C_MUL( tw , f2k , st->super_twiddles[k]);

        C_ADD( freqdata[k] , f1k ,tw);
        freqdata[k].r = (f1k.r + tw.r) / 2;
        freqdata[k].i = (f1k.i + tw.i) / 2;

        freqdata[N-k].r =   (f1k.r - tw.r)/2;
        freqdata[N-k].i = - (f1k.i - tw.i)/2;
    }
    freqdata[N].r = st->tmpbuf[0].r - st->tmpbuf[0].i;
    freqdata[N].i = 0;
    C_FIXDIV(freqdata[N],2);
}

void kiss_fftri(kiss_fftr_cfg st,const kiss_fft_cpx *freqdata,kiss_fft_scalar *timedata)
{
    /* input buffer timedata is stored row-wise */
    int k, N;

    if (st->substate->inverse == 0) {
        fprintf (stderr, "kiss fft usage error: improper alloc\n");
        exit (1);
    }

    N = st->substate->nfft;

    st->tmpbuf[0].r = freqdata[0].r + freqdata[N].r;
    st->tmpbuf[0].i = freqdata[0].r - freqdata[N].r;

    for (k = 1; k <= N / 2; ++k) {
        kiss_fft_cpx fk, fnkc, fek, fok, tmpbuf;
        fk = freqdata[k];
        fnkc.r = freqdata[N - k].r;
        fnkc.i = -freqdata[N - k].i;

        C_ADD (fek, fk, fnkc);
        C_SUB (tmpbuf, fk, fnkc);
        C_MUL (fok, tmpbuf, st->super_twiddles[k]);
        C_ADD (st->tmpbuf[k], fek, fok);
        C_SUB (st->tmpbuf[N - k], fek, fok);
        st->tmpbuf[N - k].i *= -1;
    }
    kiss_fft (st->substate, st->tmpbuf, (kiss_fft_cpx *) timedata);
}
