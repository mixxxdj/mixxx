//
//	Fidlib digital filter designer code
//	-----------------------------------
//
//        Copyright (c) 2002-2004 Jim Peters <http://uazu.net/>.  This
//        file is released under the GNU Lesser General Public License
//        (LGPL) version 2.1 as published by the Free Software
//        Foundation.  See the file COPYING_LIB for details, or visit
//        <http://www.fsf.org/licenses/licenses.html>.
//
//	The code in this file was written to go with the Fiview app
//	(http://uazu.net/fiview/), but it may be used as a library for
//	other applications.  The idea behind this library is to allow
//	filters to be designed at run-time, which gives much greater
//	flexibility to filtering applications.
//
//	This file depends on the fidmkf.h file which provides the
//	filter types from Tony Fisher's 'mkfilter' package.  See that
//	file for references and links used there.
//
//
//	Here are some of the sources I used whilst writing this code:
//
//	Robert Bristow-Johnson's EQ cookbook formulae:
//	  http://www.harmony-central.com/Computer/Programming/Audio-EQ-Cookbook.txt
//

#define VERSION "0.9.10"

//
//	Filter specification string
//	---------------------------
//
//	The filter specification string can be used to completely
//	specify the filter, or it can be used with the frequency or
//	frequency range missing, in which case default values are
//	picked up from values passed directly to the routine.
//
//	The spec consists of a series of letters usually followed by
//	the order of the filter and then by any other parameters
//	required, preceded by slashes.  For example:
//
//	  LpBu4/20.4	Lowpass butterworth, 4th order, -3.01dB at 20.4Hz
//	  BpBu2/3-4	Bandpass butterworth, 2nd order, from 3 to 4Hz
//	  BpBu2/=3-4	Same filter, but adjusted exactly to the range given
//	  BsRe/1000/10	Bandstop resonator, Q=1000, frequency 10Hz
//
//	The routines fid_design() or fid_parse() are used to convert
//	this spec-string into filter coefficients and a description
//	(if required).
//
//
//	Typical usage:
//	-------------
//
//	FidFilter *filt, *filt2;
//	char *desc;
//	FidRun *run;
//	FidFunc *funcp;
//	void *fbuf1, *fbuf2;
//	int delay;
//	void my_error_func(char *err);
//
//	// Design a filter, and optionally get its long description
//	filt= fid_design(spec, rate, freq0, freq1, adj, &desc);
//
//	// List all the possible filter types
//	fid_list_filters(stdout);
//	okay= fid_list_filters_buf(buf, buf+sizeof(buf));
//
//	// Calculate the response of the filter at a given frequency
//	// (frequency is given as a proportion of the sampling rate, in
//	// the range 0 to 0.5).  If phase is returned, then this is
//	// given in the range 0 to 1 (for 0 to 2*pi).
//	resp= fid_response(filt, freq);
//	resp= fid_response_pha(filt, freq, &phase);
//
//	// Estimate the signal delay caused by a particular filter, in samples
//	delay= fid_calc_delay(filt);
//
//	// Run a given filter (this will do JIT filter compilation if this is
//	// implemented for this processor / OS)
//	run= fid_run_new(filt, &funcp);
//	fbuf1= fid_run_newbuf(run);
//	fbuf2= fid_run_newbuf(run);
//	while (...) {
//	   out_1= funcp(fbuf1, in_1);
//	   out_2= funcp(fbuf2, in_2);
//	   if (restart_required) fid_run_zapbuf(fbuf1);
//	   ...
//	}
//	fid_run_freebuf(fbuf2);
//	fid_run_freebuf(fbuf1);
//	fid_run_free(run);
//
//	// If you need to allocate your own buffers separately for some
//	// reason, then do it this way:
//	run= fid_run_new(filt, &funcp);
//	len= fid_run_bufsize(run);
//	fbuf1= Alloc(len); fid_run_initbuf(run, fbuf1);
//	fbuf2= Alloc(len); fid_run_initbuf(run, fbuf2);
//	while (...) {
//	   out_1= funcp(fbuf1, in_1);
//	   out_2= funcp(fbuf2, in_2);
//	   if (restart_required) fid_run_zapbuf(fbuf1);
//	   ...
//	}
//	free(fbuf2);
//	free(fbuf1);
//	fid_run_free(run);
//
//	// Convert an arbitrary filter into a new filter which is a single
//	// IIR/FIR pair.  This is done by convolving the coefficients.  This
//	// flattened filter will give the same result, in theory.  However,
//	// in practice this will be less accurate, especially in cases where
//	// the limits of the floating point format are being reached (e.g.
//	// subtracting numbers with small highly significant differences).
//	// The routine also ensures that the IIR first coefficient is 1.0.
//	filt2= fid_flatten(filt);
//	free(filt);
//
//	// Parse an entire filter-spec string possibly containing several FIR,
//	// IIR and predefined filters and return it as a FidFilter at the given
//	// location.  Stops at the first ,; or unmatched )]} character, or the end
//	// of the string.  Returns a strdup'd error string on error, or else 0.
//	err= fid_parse(double rate, char **pp, FidFilter **ffp);
//
//	// Set up your own fatal-error handler (default is to dump a message
//	// to STDERR and exit on fatal conditions)
//	fid_set_error_handler(&my_error_func);
//
//	// Get the version number of the library as a string (e.g. "1.0.0")
//	txt= fid_version();
//
//	// Design a filter and reduce it to a list of all the non-const
//	// coefficients, which is returned in the given double[].  The number
//	// of coefficients expected must be provided (as a check).
//	#define N_COEF <whatever>
//	double coef[N_COEF], gain;
//	gain= fid_design_coef(coef, N_COEF, spec, rate, freq0, freq1, adj);
//
//	// Rewrite a filter spec in a full and/or separated-out form
//	char *full, *min;
//	double minf0, minf1;
//	int minadj;
//	fid_rewrite_spec(spec, freq0, freq1, adj, &full, &min, &minf0, &minf1, &minadj);
//	...
//	free(full); free(min);
//
//	// Create a FidFilter based on coefficients provided in the
//	// given double array.
//	static double array[]= { 'I', 3, 1.0, 0.55, 0.77, 'F', 3, 1, -2, 1, 0 };
//	filt= fid_cv_array(array);
//
//	// Join a number of filters into a single filter (and free them too,
//	// if the first argument is 1)
//	filt= fid_cat(0, filt1, filt2, filt3, filt4, 0);
//
//

//
//	Format of returned filter
//	-------------------------
//
//	The filter returned is a single chunk of allocated memory in
//	which is stored a number of FidFilter instances.  Each
//	instance has variable length according to the coefficients
//	contained in it.  It is probably easier to think of this as a
//	stream of items in memory.  Each sub-filter starts with its
//	type as a short -- either 'I' for IIR filters, or 'F' for FIR
//	filters.  (Other types may be added later, e.g. AM modulation
//	elements, or whatever).  This is followed by a short bitmap
//	which indicates which of the coefficients are constants,
//	aiding code-generation.  Next comes the count of the following
//	coefficients, as an int.  (These header fields normally takes 8
//	bytes, the same as a double, but this might depend on the
//	platform).  Then follow the coefficients, as doubles.  The next
//	sub-filter follows on straight after that.  The end of the list
//	is marked by 8 zero bytes, meaning typ==0, cbm==0 and len==0.
//
//	The filter can be read with the aid of the FidFilter structure
//	(giving typ, cbm, len and val[] elements) and the FFNEXT()
//	macro: using ff= FFNEXT(ff) steps to the next FidFilter
//	structure along the chain.
//
//	Note that within the sub-filters, coefficients are listed in
//	the order that they apply to data, from current-sample
//	backwards in time, i.e. most recent first (so an FIR val[] of
//	0, 0, 1 represents a two-sample delay FIR filter).  IIR
//	filters are *not* necessarily adjusted so that their first
//	coefficient is 1.
//
//	Most filters have their gain pre-adjusted so that some
//	suitable part of the response is at gain==1.0.  However, this
//	depends on the filter type.
//

//
//	Check that a target macro has been set.  This macro selects
//	various fixes required on various platforms:
//
//	  T_LINUX  Linux, or probably any UNIX-like platform with GCC
//	  T_MINGW  MinGW -- either building on Win32 or cross-compiling
//	  T_MSVC   Microsoft Visual C
//
//	(On MSVC, add "T_MSVC" to the preprocessor definitions in the
//	project settings, or add /D "T_MSVC" to the compiler
//	command-line.)
//

#ifndef T_LINUX
#ifndef T_MINGW
#ifndef T_MSVC
#error Please define one of the T_* target macros (e.g. -DT_LINUX); see fidlib.c
#endif
#endif
#endif


//
//	Select which method of filter execution is preferred.
//	RF_CMDLIST is recommended (and is the default).
//
//	  RF_COMBINED -- easy to understand code, lower accuracy
//	  RF_CMDLIST  -- faster pre-compiled code
//	  RF_JIT      -- fastest JIT run-time generated code (no longer supported)
//

#ifndef RF_COMBINED
#ifndef RF_CMDLIST
#ifndef RF_JIT

#define RF_CMDLIST

#endif
#endif
#endif

//
//	Includes
//

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "fidlib.h"

extern FidFilter *mkfilter(char *, ...);

//
//	Target-specific fixes
//

// Macro for local inline routines that shouldn't be visible externally
// See Mixxx Bug #1179683
#if defined(T_MINGW) || defined(T_MSVC)
 #define STATIC_INLINE static __inline
#else
 #define STATIC_INLINE static inline
#endif

// MinGW and MSVC fixes
#if defined(T_MINGW) || defined(T_MSVC)
 #ifndef vsnprintf
  #define vsnprintf _vsnprintf
 #endif
 #ifndef snprintf
  #define snprintf _snprintf
 #endif
// Not sure if we strictly need this still
 STATIC_INLINE double
 my_asinh(double val) {
    return log(val + sqrt(val*val + 1.0));
 }
 #define asinh(xx) my_asinh(xx)
#endif


//
//	Support code
//

static void (*error_handler)(char *err)= 0;

static void
error(char *fmt, ...) {
   char buf[1024];
   va_list ap;
   va_start(ap, fmt);

   vsnprintf(buf, sizeof(buf), fmt, ap);	// Ignore overflow
   buf[sizeof(buf)-1]= 0;
   if (error_handler) error_handler(buf);

   // If error handler routine returns, we dump to STDERR and exit anyway
   fprintf(stderr, "fidlib error: %s\n", buf);
   exit(1);
}

static char *
strdupf(char *fmt, ...) {
   va_list ap;
   char buf[1024], *rv;
   int len;
   va_start(ap, fmt);
   len= vsnprintf(buf, sizeof(buf), fmt, ap);
   if (len < 0 || len >= (int)sizeof(buf)-1)
      error("strdupf exceeded buffer");
   rv= strdup(buf);
   if (!rv) error("Out of memory");
   return rv;
}

static void *
Alloc(int size) {
   void *vp= calloc(1, size);
   if (!vp) error("Out of memory");
   return vp;
}

#define ALLOC(type) ((type*)Alloc(sizeof(type)))
#define ALLOC_ARR(cnt, type) ((type*)Alloc((cnt) * sizeof(type)))


//
//      Complex multiply: aa *= bb;
//

STATIC_INLINE void
cmul(double *aa, double *bb) {
   double rr= aa[0] * bb[0] - aa[1] * bb[1];
   double ii= aa[0] * bb[1] + aa[1] * bb[0];
   aa[0]= rr;
   aa[1]= ii;
}

//
//      Complex square: aa *= aa;
//

STATIC_INLINE void
csqu(double *aa) {
   double rr= aa[0] * aa[0] - aa[1] * aa[1];
   double ii= 2 * aa[0] * aa[1];
   aa[0]= rr;
   aa[1]= ii;
}

//
//      Complex multiply by real: aa *= bb;
//

STATIC_INLINE void
cmulr(double *aa, double fact) {
   aa[0] *= fact;
   aa[1] *= fact;
}

//
//	Complex conjugate: aa= aa*
//

STATIC_INLINE void
cconj(double *aa) {
   aa[1]= -aa[1];
}

//
//      Complex divide: aa /= bb;
//

STATIC_INLINE void
cdiv(double *aa, double *bb) {
   double rr= aa[0] * bb[0] + aa[1] * bb[1];
   double ii= -aa[0] * bb[1] + aa[1] * bb[0];
   double fact= 1.0 / (bb[0] * bb[0] + bb[1] * bb[1]);
   aa[0]= rr * fact;
   aa[1]= ii * fact;
}

//
//	Complex reciprocal: aa= 1/aa
//

STATIC_INLINE void
crecip(double *aa) {
   double fact= 1.0 / (aa[0] * aa[0] + aa[1] * aa[1]);
   aa[0] *= fact;
   aa[1] *= -fact;
}

//
//	Complex assign: aa= bb
//

STATIC_INLINE void
cass(double *aa, double *bb) {
   memcpy(aa, bb, 2*sizeof(double));  // Assigning doubles is really slow
}

//
//	Complex assign: aa= (rr + ii*j)
//

STATIC_INLINE void
cassz(double *aa, double rr, double ii) {
   aa[0]= rr;
   aa[1]= ii;
}

//
//	Complex add: aa += bb
//

STATIC_INLINE void
cadd(double *aa, double *bb) {
   aa[0] += bb[0];
   aa[1] += bb[1];
}

//
//	Complex add: aa += (rr + ii*j)
//

STATIC_INLINE void
caddz(double *aa, double rr, double ii) {
   aa[0] += rr;
   aa[1] += ii;
}

//
//	Complex subtract: aa -= bb
//

STATIC_INLINE void
csub(double *aa, double *bb) {
   aa[0] -= bb[0];
   aa[1] -= bb[1];
}

//
//	Complex subtract: aa -= (rr + ii*j)
//

STATIC_INLINE void
csubz(double *aa, double rr, double ii) {
   aa[0] -= rr;
   aa[1] -= ii;
}

//
//	Complex negate: aa= -aa
//

STATIC_INLINE void
cneg(double *aa) {
   aa[0]= -aa[0];
   aa[1]= -aa[1];
}

//
//      Evaluate a complex polynomial given the coefficients.
//      rv[0]+i*rv[1] is the result, in[0]+i*in[1] is the input value.
//      Coefficients are real values.
//

STATIC_INLINE void
evaluate(double *rv, double *coef, int n_coef, double *in) {
   double pz[2];        // Powers of Z

   // Handle first iteration by hand
   rv[0]= *coef++;
   rv[1]= 0;

   if (--n_coef > 0) {
      // Handle second iteration by hand
      pz[0]= in[0];
      pz[1]= in[1];
      rv[0] += *coef * pz[0];
      rv[1] += *coef * pz[1];
      coef++; n_coef--;

      // Loop for remainder
      while (n_coef > 0) {
         cmul(pz, in);
         rv[0] += *coef * pz[0];
         rv[1] += *coef * pz[1];
         coef++;
         n_coef--;
      }
   }
}


//
//	Housekeeping
//

void
fid_set_error_handler(void (*rout)(char*)) {
   error_handler= rout;
}

char *
fid_version() {
   return VERSION;
}


//
//	Get the response and phase of a filter at the given frequency
//	(expressed as a proportion of the sampling rate, 0->0.5).
//	Phase is returned as a number from 0 to 1, representing a
//	phase between 0 and two-pi.
//

double
fid_response_pha(FidFilter *filt, double freq, double *phase) {
   double top[2], bot[2];
   double theta= freq * 2 * M_PI;
   double zz[2];

   top[0]= 1;
   top[1]= 0;
   bot[0]= 1;
   bot[1]= 0;
   zz[0]= cos(theta);
   zz[1]= sin(theta);

   while (filt->len) {
      double resp[2];
      int cnt= filt->len;
      evaluate(resp, filt->val, cnt, zz);
      if (filt->typ == 'I')
	 cmul(bot, resp);
      else if (filt->typ == 'F')
	 cmul(top, resp);
      else
	 error("Unknown filter type %d in fid_response_pha()", filt->typ);
      filt= FFNEXT(filt);
   }

   cdiv(top, bot);

   if (phase) {
      double pha= atan2(top[1], top[0]) / (2 * M_PI);
      if (pha < 0) pha += 1.0;
      *phase= pha;
   }

   return hypot(top[1], top[0]);
}

//
//	Get the response of a filter at the given frequency (expressed
//	as a proportion of the sampling rate, 0->0.5).
//
//	Code duplicate, as I didn't want the overhead of a function
//	call to fid_response_pha.  Almost every call in this routine
//	can be inlined.
//

double
fid_response(FidFilter *filt, double freq) {
   double top[2], bot[2];
   double theta= freq * 2 * M_PI;
   double zz[2];

   top[0]= 1;
   top[1]= 0;
   bot[0]= 1;
   bot[1]= 0;
   zz[0]= cos(theta);
   zz[1]= sin(theta);

   while (filt->len) {
      double resp[2];
      int cnt= filt->len;
      evaluate(resp, filt->val, cnt, zz);
      if (filt->typ == 'I')
	 cmul(bot, resp);
      else if (filt->typ == 'F')
	 cmul(top, resp);
      else
	 error("Unknown filter type %d in fid_response()", filt->typ);
      filt= FFNEXT(filt);
   }

   cdiv(top, bot);

   return hypot(top[1], top[0]);
}


//
//	Estimate the delay that a filter causes to the signal by
//	looking for the point at which 50% of the filter calculations
//	are complete.  This involves running test impulses through the
//	filter several times.  The estimated delay in samples is
//	returned.
//
//	Delays longer than 8,000,000 samples are not handled well, as
//	the code drops out at this point rather than get stuck in an
//	endless loop.
//

int
fid_calc_delay(FidFilter *filt) {
   FidRun *run;
   FidFunc *dostep;
   void *f1, *f2;
   double tot, tot100, tot50;
   int cnt;

   run= fid_run_new(filt, &dostep);

   // Run through to find at least the 99.9% point of filter; the r2
   // (tot100) filter runs at 4x the speed of the other one to act as
   // a reference point much further ahead in the impulse response.
   f1= fid_run_newbuf(run);
   f2= fid_run_newbuf(run);

   tot= fabs(dostep(f1, 1.0));
   tot100= fabs(dostep(f2, 1.0));
   tot100 += fabs(dostep(f2, 0.0));
   tot100 += fabs(dostep(f2, 0.0));
   tot100 += fabs(dostep(f2, 0.0));

   for (cnt= 1; cnt < 0x1000000; cnt++) {
      tot += fabs(dostep(f1, 0.0));
      tot100 += fabs(dostep(f2, 0.0));
      tot100 += fabs(dostep(f2, 0.0));
      tot100 += fabs(dostep(f2, 0.0));
      tot100 += fabs(dostep(f2, 0.0));

      if (tot/tot100 >= 0.999) break;
   }
   fid_run_freebuf(f1);
   fid_run_freebuf(f2);

   // Now find the 50% point
   tot50= tot100/2;
   f1= fid_run_newbuf(run);
   tot= fabs(dostep(f1, 1.0));
   for (cnt= 0; tot < tot50; cnt++)
      tot += fabs(dostep(f1, 0.0));
   fid_run_freebuf(f1);

   // Clean up, return
   fid_run_free(run);
   return cnt;
}


//
//	'mkfilter'-derived code
//

#include "fidmkf.h"


//
//	Stack a number of identical filters, generating the required
//	FidFilter* return value
//

static FidFilter*
stack_filter(int order, int n_head, int n_val, ...) {
   FidFilter *rv= FFALLOC(n_head * order, n_val * order);
   FidFilter *p, *q;
   va_list ap;
   int a, b, len;

   if (order == 0) return rv;

   // Copy from ap
   va_start(ap, n_val);
   p= q= rv;
   for (a= 0; a<n_head; a++) {
      p->typ= va_arg(ap, int);
      p->cbm= va_arg(ap, int);
      p->len= va_arg(ap, int);
      for (b= 0; b<p->len; b++)
	 p->val[b]= va_arg(ap, double);
      p= FFNEXT(p);
   }
   order--;

   // Check length
   len= ((char*)p)-((char*)q);
   if (len != (int)FFCSIZE(n_head-1, n_val))
      error("Internal error; bad call to stack_filter(); length mismatch (%d,%d)",
	    len, FFCSIZE(n_head-1, n_val));

   // Make as many additional copies as necessary
   while (order-- > 0) {
      memcpy(p, q, len);
      p= (FidFilter*)(len + (char*)p);
   }

   // List is already terminated due to zeroed allocation
   return rv;
}

//
//	Search for a peak between two given frequencies.  It is
//	assumed that the gradient goes upwards from 'f0' to the peak,
//	and then down again to 'f3'.  If there are any other curves,
//	this routine will get confused and will come up with some
//	frequency, although probably not the right one.
//
//	Returns the frequency of the peak.
//

static double
search_peak(FidFilter *ff, double f0, double f3) {
   double f1, f2;
   double r1, r2;
   int a;

   // Binary search, modified, taking two intermediate points.  Do 20
   // subdivisions, which should give 1/2^20 == 1e-6 accuracy compared
   // to original range.
   for (a= 0; a<20; a++) {
      f1= 0.51 * f0 + 0.49 * f3;
      f2= 0.49 * f0 + 0.51 * f3;
      if (f1 == f2) break;		// We're hitting FP limit
      r1= fid_response(ff, f1);
      r2= fid_response(ff, f2);
      if (r1 > r2)	// Peak is either to the left, or between f1/f2
	 f3= f2;
      else	 	// Peak is either to the right, or between f1/f2
	 f0= f1;
   }
   return (f0+f3)*0.5;
}

//
//	Handle the different 'back-ends' for Bessel, Butterworth and
//	Chebyshev filters.  First argument selects between bilinear
//	(0) and matched-Z (non-0).  The BL and MZ macros makes this a
//	bit more obvious in the code.
//
//	Overall filter gain is adjusted to give the peak at 1.0.  This
//	is easy for all types except for band-pass, where a search is
//	required to find the precise peak.  This is much slower than
//	the other types.
//

#define BL 0
#define MZ 1

static FidFilter*
do_lowpass(struct mk_filter_context* ctx, int mz, double freq) {
   FidFilter *rv;
   lowpass(ctx, prewarp(freq));
   if (mz) s2z_matchedZ(ctx); else s2z_bilinear(ctx);
   rv= z2fidfilter(ctx, 1.0, ~0);	// FIR is constant
   rv->val[0]= 1.0 / fid_response(rv, 0.0);
   return rv;
}

static FidFilter*
do_highpass(struct mk_filter_context* ctx, int mz, double freq) {
   FidFilter *rv;
   highpass(ctx, prewarp(freq));
   if (mz) s2z_matchedZ(ctx); else s2z_bilinear(ctx);
   rv= z2fidfilter(ctx, 1.0, ~0);	// FIR is constant
   rv->val[0]= 1.0 / fid_response(rv, 0.5);
   return rv;
}

static FidFilter*
do_bandpass(struct mk_filter_context* ctx, int mz, double f0, double f1) {
   FidFilter *rv;
   bandpass(ctx, prewarp(f0), prewarp(f1));
   if (mz) s2z_matchedZ(ctx); else s2z_bilinear(ctx);
   rv= z2fidfilter(ctx, 1.0, ~0);	// FIR is constant
   rv->val[0]= 1.0 / fid_response(rv, search_peak(rv, f0, f1));
   return rv;
}

static FidFilter*
do_bandstop(struct mk_filter_context* ctx, int mz, double f0, double f1) {
   FidFilter *rv;
   bandstop(ctx, prewarp(f0), prewarp(f1));
   if (mz) s2z_matchedZ(ctx); else s2z_bilinear(ctx);
   rv= z2fidfilter(ctx, 1.0, 5);	// FIR second coefficient is *non-const* for bandstop
   rv->val[0]= 1.0 / fid_response(rv, 0.0);	// Use 0Hz response as reference
   return rv;
}


//
//	Information passed to individual filter design routines:
//
//	  double* rout(double rate, double f0, double f1,
//		       int order, int n_arg, double *arg);
//
//	'rate' is the sampling rate, or 1 if not set
//	'f0' and 'f1' give the frequency or frequency range as a
//	 	proportion of the sampling rate
//	'order' is the order of the filter (the integer passed immediately
//		after the name)
//	'n_arg' is the number of additional arguments for the filter
//	'arg' gives the additional argument values: arg[n]
//
//	Note that #O #o #F and #R are mapped to the f0/f1/order
//	arguments, and are not included in the arg[] array.
//
//	See the previous description for the required meaning of the
//	return value FidFilter list.
//

//
//	Filter design routines and supporting code
//

static FidFilter*
des_bpre(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;
   (void)order; 
   (void)n_arg;       
   struct mk_filter_context ctx;
   bandpass_res(&ctx, f0, arg[0]);
   return z2fidfilter(&ctx, 1.0, ~0);	// FIR constant
}

static FidFilter*
des_bsre(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;
   (void)order; 
   (void)n_arg;     
   struct mk_filter_context ctx;
   bandstop_res(&ctx, f0, arg[0]);
   return z2fidfilter(&ctx, 1.0, 0);	// FIR not constant, depends on freq
}

static FidFilter*
des_apre(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;
   (void)order; 
   (void)n_arg;     
   struct mk_filter_context ctx;
   allpass_res(&ctx, f0, arg[0]);
   return z2fidfilter(&ctx, 1.0, 0);	// FIR not constant, depends on freq
}

static FidFilter*
des_pi(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;
   (void)order; 
   (void)n_arg;   
   (void)arg;    
   struct mk_filter_context ctx;
   prop_integral(&ctx, prewarp(f0));
   s2z_bilinear(&ctx);
   return z2fidfilter(&ctx, 1.0, 0);	// FIR not constant, depends on freq
}

static FidFilter*
des_piz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;
   (void)order; 
   (void)n_arg;   
   (void)arg;    
   struct mk_filter_context ctx;
   prop_integral(&ctx, prewarp(f0));
   s2z_matchedZ(&ctx);
   return z2fidfilter(&ctx, 1.0, 0);	// FIR not constant, depends on freq
}

static FidFilter*
des_lpbe(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg;    
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_lowpass(&ctx, BL, f0);
}

static FidFilter*
des_hpbe(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_highpass(&ctx, BL, f0);
}

static FidFilter*
des_bpbe(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_bandpass(&ctx, BL, f0, f1);
}

static FidFilter*
des_bsbe(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_bandstop(&ctx, BL, f0, f1);
}

static FidFilter*
des_lpbez(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_lowpass(&ctx, MZ, f0);
}

static FidFilter*
des_hpbez(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_highpass(&ctx, MZ, f0);
}

static FidFilter*
des_bpbez(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_bandpass(&ctx, MZ, f0, f1);
}

static FidFilter*
des_bsbez(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   bessel(&ctx, order);
   return do_bandstop(&ctx, MZ, f0, f1);
}

static FidFilter*	// Butterworth-Bessel cross
des_lpbube(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;   
   (void)n_arg; 
   double tmp[MAXPZ];
   int a;
   struct mk_filter_context ctx;
   bessel(&ctx, order); memcpy(tmp, ctx.pol, order * sizeof(double));
   butterworth(&ctx, order);
   for (a= 0; a<order; a++) ctx.pol[a] += (tmp[a]-ctx.pol[a]) * 0.01 * arg[0];
   //for (a= 1; a<order; a+=2) ctx.pol[a] += arg[1] * 0.01;
   return do_lowpass(&ctx, BL, f0);
}

static FidFilter*
des_lpbu(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg;    
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_lowpass(&ctx, BL, f0);
}

static FidFilter*
des_hpbu(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_highpass(&ctx, BL, f0);
}

static FidFilter*
des_bpbu(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_bandpass(&ctx, BL, f0, f1);
}

static FidFilter*
des_bsbu(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_bandstop(&ctx, BL, f0, f1);
}

static FidFilter*
des_lpbuz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg; 
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_lowpass(&ctx, MZ, f0);
}

static FidFilter*
des_hpbuz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1; 
   (void)n_arg;   
   (void)arg;   
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_highpass(&ctx, MZ, f0);
}

static FidFilter*
des_bpbuz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg;
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_bandpass(&ctx, MZ, f0, f1);
}

static FidFilter*
des_bsbuz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;   
   (void)arg;
   struct mk_filter_context ctx;
   butterworth(&ctx, order);
   return do_bandstop(&ctx, MZ, f0, f1);
}

static FidFilter*
des_lpch(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;   
   (void)n_arg;
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_lowpass(&ctx, BL, f0);
}

static FidFilter*
des_hpch(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;   
   (void)n_arg;   
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_highpass(&ctx, BL, f0);
}

static FidFilter*
des_bpch(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)n_arg;
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_bandpass(&ctx, BL, f0, f1);
}

static FidFilter*
des_bsch(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;  
   (void)n_arg;
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_bandstop(&ctx, BL, f0, f1);
}

static FidFilter*
des_lpchz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;   
   (void)n_arg; 
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_lowpass(&ctx, MZ, f0);
}

static FidFilter*
des_hpchz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;
   (void)f1;   
   (void)n_arg;
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_highpass(&ctx, MZ, f0);
}

static FidFilter*
des_bpchz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;   
   (void)n_arg;
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_bandpass(&ctx, MZ, f0, f1);
}

static FidFilter*
des_bschz(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate;   
   (void)n_arg; 
   struct mk_filter_context ctx;
   chebyshev(&ctx, order, arg[0]);
   return do_bandstop(&ctx, MZ, f0, f1);
}

static FidFilter*
des_lpbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg; 
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   return stack_filter(order, 3, 7,
		       'I', 0x0, 3, 1 + alpha, -2 * cosv, 1 - alpha,
		       'F', 0x7, 3, 1.0, 2.0, 1.0,
		       'F', 0x0, 1, (1-cosv) * 0.5);
}

static FidFilter*
des_hpbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg; 
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   return stack_filter(order, 3, 7,
		       'I', 0x0, 3, 1 + alpha, -2 * cosv, 1 - alpha,
		       'F', 0x7, 3, 1.0, -2.0, 1.0,
		       'F', 0x0, 1, (1+cosv) * 0.5);
}

static FidFilter*
des_bpbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg; 
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   return stack_filter(order, 3, 7,
		       'I', 0x0, 3, 1 + alpha, -2 * cosv, 1 - alpha,
		       'F', 0x7, 3, 1.0, 0.0, -1.0,
		       'F', 0x0, 1, alpha);
}

static FidFilter*
des_bsbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg;    
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   return stack_filter(order, 2, 6,
		       'I', 0x0, 3, 1 + alpha, -2 * cosv, 1 - alpha,
		       'F', 0x5, 3, 1.0, -2 * cosv, 1.0);
}

static FidFilter*
des_apbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg;    
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   return stack_filter(order, 2, 6,
		       'I', 0x0, 3, 1 + alpha, -2 * cosv, 1 - alpha,
		       'F', 0x0, 3, 1 - alpha, -2 * cosv, 1 + alpha);
}

static FidFilter*
des_pkbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg;    
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double alpha= sin(omega) / 2 / arg[0];
   double A= pow(10, arg[1]/40);
   return stack_filter(order, 2, 6,
		       'I', 0x0, 3, 1 + alpha/A, -2 * cosv, 1 - alpha/A,
		       'F', 0x0, 3, 1 + alpha*A, -2 * cosv, 1 - alpha*A);
}

static FidFilter*
des_lsbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg; 
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double sinv= sin(omega);
   double A= pow(10, arg[1]/40);
   double beta= sqrt((A*A+1)/arg[0] - (A-1)*(A-1));
   return stack_filter(order, 2, 6,
		       'I', 0x0, 3,
		       (A+1) + (A-1)*cosv + beta*sinv,
		       -2 * ((A-1) + (A+1)*cosv),
		       (A+1) + (A-1)*cosv - beta*sinv,
		       'F', 0x0, 3,
		       A*((A+1) - (A-1)*cosv + beta*sinv),
		       2*A*((A-1) - (A+1)*cosv),
		       A*((A+1) - (A-1)*cosv - beta*sinv));
}

static FidFilter*
des_hsbq(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)n_arg;   
   double omega= 2 * M_PI * f0;
   double cosv= cos(omega);
   double sinv= sin(omega);
   double A= pow(10, arg[1]/40);
   double beta= sqrt((A*A+1)/arg[0] - (A-1)*(A-1));
   return stack_filter(order, 2, 6,
		       'I', 0x0, 3,
		       (A+1) - (A-1)*cosv + beta*sinv,
		       2 * ((A-1) - (A+1)*cosv),
		       (A+1) - (A-1)*cosv - beta*sinv,
		       'F', 0x0, 3,
		       A*((A+1) + (A-1)*cosv + beta*sinv),
		       -2*A*((A-1) + (A+1)*cosv),
		       A*((A+1) + (A-1)*cosv - beta*sinv));
}

static FidFilter*
des_lpbl(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)order;
   (void)n_arg;   
   (void)arg;  
   double wid= 0.4109205/f0;
   double tot, adj;
   int max= (int)floor(wid);
   int a;
   FidFilter *ff= (FidFilter*)Alloc(FFCSIZE(1, max*2+1));
   ff->typ= 'F';
   ff->cbm= 0;
   ff->len= max*2+1;
   ff->val[max]= tot= 1.0;
   for (a= 1; a<=max; a++) {
      double val= 0.42 +
	 0.5 * cos(M_PI * a / wid) +
	 0.08 * cos(M_PI * 2.0 * a / wid);
      ff->val[max-a]= val;
      ff->val[max+a]= val;
      tot += val * 2.0;
   }
   adj= 1/tot;
   for (a= 0; a<=max*2; a++) ff->val[a] *= adj;
   return ff;
}

static FidFilter*
des_lphm(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)order;
   (void)n_arg;   
   (void)arg;  
   double wid= 0.3262096/f0;
   double tot, adj;
   int max= (int)floor(wid);
   int a;
   FidFilter *ff= (FidFilter*)Alloc(FFCSIZE(1, max*2+1));
   ff->typ= 'F';
   ff->cbm= 0;
   ff->len= max*2+1;
   ff->val[max]= tot= 1.0;
   for (a= 1; a<=max; a++) {
      double val= 0.54 +
	 0.46 * cos(M_PI * a / wid);
      ff->val[max-a]= val;
      ff->val[max+a]= val;
      tot += val * 2.0;
   }
   adj= 1/tot;
   for (a= 0; a<=max*2; a++) ff->val[a] *= adj;
   return ff;
}

static FidFilter*
des_lphn(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)order;
   (void)n_arg;   
   (void)arg;    
   double wid= 0.360144/f0;
   double tot, adj;
   int max= (int)floor(wid);
   int a;
   FidFilter *ff= (FidFilter*)Alloc(FFCSIZE(1, max*2+1));
   ff->typ= 'F';
   ff->cbm= 0;
   ff->len= max*2+1;
   ff->val[max]= tot= 1.0;
   for (a= 1; a<=max; a++) {
      double val= 0.5 +
	 0.5 * cos(M_PI * a / wid);
      ff->val[max-a]= val;
      ff->val[max+a]= val;
      tot += val * 2.0;
   }
   adj= 1/tot;
   for (a= 0; a<=max*2; a++) ff->val[a] *= adj;
   return ff;
}

static FidFilter*
des_lpba(double rate, double f0, double f1, int order, int n_arg, double *arg) {
   (void)rate; 
   (void)f1;   
   (void)order;
   (void)n_arg;   
   (void)arg;  
   double wid= 0.3189435/f0;
   double tot, adj;
   int max= (int)floor(wid);
   int a;
   FidFilter *ff= (FidFilter*)Alloc(FFCSIZE(1, max*2+1));
   ff->typ= 'F';
   ff->cbm= 0;
   ff->len= max*2+1;
   ff->val[max]= tot= 1.0;
   for (a= 1; a<=max; a++) {
      double val= 1.0 - a/wid;
      ff->val[max-a]= val;
      ff->val[max+a]= val;
      tot += val * 2.0;
   }
   adj= 1/tot;
   for (a= 0; a<=max*2; a++) ff->val[a] *= adj;
   return ff;
}


//
//	Filter table
//

static struct {
   FidFilter *(*rout)(double,double,double,int,int,double*); // Designer routine address
   char *fmt;	// Format for spec-string
   char *txt;	// Human-readable description of filter
} filter[]= {
   { des_bpre, "BpRe/#V/#F",
     "Bandpass resonator, Q=#V (0 means Inf), frequency #F" },
   { des_bsre, "BsRe/#V/#F",
     "Bandstop resonator, Q=#V (0 means Inf), frequency #F" },
   { des_apre, "ApRe/#V/#F",
     "Allpass resonator, Q=#V (0 means Inf), frequency #F" },
   { des_pi, "Pi/#F",
     "Proportional-integral filter, frequency #F" },
   { des_piz, "PiZ/#F",
     "Proportional-integral filter, matched z-transform, frequency #F" },
   { des_lpbe, "LpBe#O/#F",
     "Lowpass Bessel filter, order #O, -3.01dB frequency #F" },
   { des_hpbe, "HpBe#O/#F",
     "Highpass Bessel filter, order #O, -3.01dB frequency #F" },
   { des_bpbe, "BpBe#O/#R",
     "Bandpass Bessel filter, order #O, -3.01dB frequencies #R" },
   { des_bsbe, "BsBe#O/#R",
     "Bandstop Bessel filter, order #O, -3.01dB frequencies #R" },
   { des_lpbu, "LpBu#O/#F",
     "Lowpass Butterworth filter, order #O, -3.01dB frequency #F" },
   { des_hpbu, "HpBu#O/#F",
     "Highpass Butterworth filter, order #O, -3.01dB frequency #F" },
   { des_bpbu, "BpBu#O/#R",
     "Bandpass Butterworth filter, order #O, -3.01dB frequencies #R" },
   { des_bsbu, "BsBu#O/#R",
     "Bandstop Butterworth filter, order #O, -3.01dB frequencies #R" },
   { des_lpch, "LpCh#O/#V/#F",
     "Lowpass Chebyshev filter, order #O, passband ripple #VdB, -3.01dB frequency #F" },
   { des_hpch, "HpCh#O/#V/#F",
     "Highpass Chebyshev filter, order #O, passband ripple #VdB, -3.01dB frequency #F" },
   { des_bpch, "BpCh#O/#V/#R",
     "Bandpass Chebyshev filter, order #O, passband ripple #VdB, -3.01dB frequencies #R" },
   { des_bsch, "BsCh#O/#V/#R",
     "Bandstop Chebyshev filter, order #O, passband ripple #VdB, -3.01dB frequencies #R" },
   { des_lpbez, "LpBeZ#O/#F",
     "Lowpass Bessel filter, matched z-transform, order #O, -3.01dB frequency #F" },
   { des_hpbez, "HpBeZ#O/#F",
     "Highpass Bessel filter, matched z-transform, order #O, -3.01dB frequency #F" },
   { des_bpbez, "BpBeZ#O/#R",
     "Bandpass Bessel filter, matched z-transform, order #O, -3.01dB frequencies #R" },
   { des_bsbez, "BsBeZ#O/#R",
     "Bandstop Bessel filter, matched z-transform, order #O, -3.01dB frequencies #R" },
   { des_lpbuz, "LpBuZ#O/#F",
     "Lowpass Butterworth filter, matched z-transform, order #O, -3.01dB frequency #F" },
   { des_hpbuz, "HpBuZ#O/#F",
     "Highpass Butterworth filter, matched z-transform, order #O, -3.01dB frequency #F" },
   { des_bpbuz, "BpBuZ#O/#R",
     "Bandpass Butterworth filter, matched z-transform, order #O, -3.01dB frequencies #R" },
   { des_bsbuz, "BsBuZ#O/#R",
     "Bandstop Butterworth filter, matched z-transform, order #O, -3.01dB frequencies #R" },
   { des_lpchz, "LpChZ#O/#V/#F",
     "Lowpass Chebyshev filter, matched z-transform, order #O, "
     "passband ripple #VdB, -3.01dB frequency #F" },
   { des_hpchz, "HpChZ#O/#V/#F",
     "Highpass Chebyshev filter, matched z-transform, order #O, "
     "passband ripple #VdB, -3.01dB frequency #F" },
   { des_bpchz, "BpChZ#O/#V/#R",
     "Bandpass Chebyshev filter, matched z-transform, order #O, "
     "passband ripple #VdB, -3.01dB frequencies #R" },
   { des_bschz, "BsChZ#O/#V/#R",
     "Bandstop Chebyshev filter, matched z-transform, order #O, "
     "passband ripple #VdB, -3.01dB frequencies #R" },
   { des_lpbube, "LpBuBe#O/#V/#F",
     "Lowpass Butterworth-Bessel #V% cross, order #O, -3.01dB frequency #F" },
   { des_lpbq, "LpBq#o/#V/#F",
     "Lowpass biquad filter, order #O, Q=#V, -3.01dB frequency #F" },
   { des_hpbq, "HpBq#o/#V/#F",
     "Highpass biquad filter, order #O, Q=#V, -3.01dB frequency #F" },
   { des_bpbq, "BpBq#o/#V/#F",
     "Bandpass biquad filter, order #O, Q=#V, centre frequency #F" },
   { des_bsbq, "BsBq#o/#V/#F",
     "Bandstop biquad filter, order #O, Q=#V, centre frequency #F" },
   { des_apbq, "ApBq#o/#V/#F",
     "Allpass biquad filter, order #O, Q=#V, centre frequency #F" },
   { des_pkbq, "PkBq#o/#V/#V/#F",
     "Peaking biquad filter, order #O, Q=#V, dBgain=#V, frequency #F" },
   { des_lsbq, "LsBq#o/#V/#V/#F",
     "Lowpass shelving biquad filter, S=#V, dBgain=#V, frequency #F" },
   { des_hsbq, "HsBq#o/#V/#V/#F",
     "Highpass shelving biquad filter, S=#V, dBgain=#V, frequency #F" },
   { des_lpbl, "LpBl/#F",
     "Lowpass Blackman window, -3.01dB frequency #F" },
   { des_lphm, "LpHm/#F",
     "Lowpass Hamming window, -3.01dB frequency #F" },
   { des_lphn, "LpHn/#F",
     "Lowpass Hann window, -3.01dB frequency #F" },
   { des_lpba, "LpBa/#F",
     "Lowpass Bartlet (triangular) window, -3.01dB frequency #F" },
   { 0, 0, 0 }
};

//
//	Design a filter.  Spec and range are passed as arguments.  The
//	return value is a pointer to a FidFilter as documented earlier
//	in this file.  This needs to be free()d once finished with.
//
//	If 'f_adj' is set, then the frequencies fed to the design code
//	are adjusted automatically to get true sqrt(0.5) (-3.01dB)
//	values at the provided frequencies.  (This is obviously a
//	slower operation)
//
//	If 'descp' is non-0, then a long description of the filter is
//	generated and returned as a strdup'd string at the given
//	location.
//
//	Any problem with the spec causes the program to die with an
//	error message.
//
//	'spec' gives the specification string.  The 'rate' argument
//	gives the sampling rate for the data that will be passed to
//	the filter.  This is only used to interpret the frequencies
//	given in the spec or given in 'freq0' and 'freq1'.  Use 1.0 if
//	the frequencies are given as a proportion of the sampling
//	rate, in the range 0 to 0.5.  'freq0' and 'freq1' provide the
//	default frequency or frequency range if this is not included
//	in the specification string.  These should be -ve if there is
//	no default range (causing an error if they are omitted from
//	the 'spec').
//

typedef struct Spec Spec;
static char* parse_spec(Spec*);
static FidFilter *auto_adjust_single(Spec *sp, double rate, double f0);
static FidFilter *auto_adjust_dual(Spec *sp, double rate, double f0, double f1);
struct Spec {
#define MAXARG 10
   const char *spec;
   double in_f0, in_f1;
   int in_adj;
   double argarr[MAXARG];
   double f0, f1;
   int adj;
   int n_arg;
   int order;
   int minlen;		// Minimum length of spec-string, assuming f0/f1 passed separately
   int n_freq;		// Number of frequencies provided: 0,1,2
   int fi;		// Filter index (filter[fi])
};

FidFilter *
fid_design(const char *spec, double rate, double freq0, double freq1, int f_adj, char **descp) {
   FidFilter *rv;
   Spec sp;
   double f0, f1;
   char *err;

   // Parse the filter-spec
   sp.spec= spec;
   sp.in_f0= freq0;
   sp.in_f1= freq1;
   sp.in_adj= f_adj;
   err= parse_spec(&sp);
   if (err) error("%s", err);
   f0= sp.f0;
   f1= sp.f1;

   // Adjust frequencies to range 0-0.5, and check them
   f0 /= rate;
   if (f0 > 0.5) error("Frequency of %gHz out of range with sampling rate of %gHz", f0*rate, rate);
   f1 /= rate;
   if (f1 > 0.5) error("Frequency of %gHz out of range with sampling rate of %gHz", f1*rate, rate);

   // Okay we now have a successful spec-match to filter[sp.fi], and sp.n_arg
   // args are now in sp.argarr[]

   // Generate the filter
   if (!sp.adj)
      rv= filter[sp.fi].rout(rate, f0, f1, sp.order, sp.n_arg, sp.argarr);
   else if (strstr(filter[sp.fi].fmt, "#R"))
      rv= auto_adjust_dual(&sp, rate, f0, f1);
   else
      rv= auto_adjust_single(&sp, rate, f0);

   // Generate a long description if required
   if (descp) {
      char *fmt= filter[sp.fi].txt;
      int max= strlen(fmt) + 60 + sp.n_arg * 20;
      char *desc= (char*)Alloc(max);
      char *p= desc;
      char ch;
      double *arg= sp.argarr;
      int n_arg= sp.n_arg;

      while ((ch= *fmt++)) {
	 if (ch != '#') {
	    *p++= ch;
	    continue;
	 }

	 switch (*fmt++) {
	  case 'O':
	     p += sprintf(p, "%d", sp.order);
	     break;
	  case 'F':
	     p += sprintf(p, "%g", f0*rate);
	     break;
	  case 'R':
	     p += sprintf(p, "%g-%g", f0*rate, f1*rate);
	     break;
	  case 'V':
	     if (n_arg <= 0)
		error("Internal error -- disagreement between filter short-spec\n"
		      " and long-description over number of arguments");
	     n_arg--;
	     p += sprintf(p, "%g", *arg++);
	     break;
	  default:
	     error("Internal error: unknown format in long description: #%c", fmt[-1]);
	 }
      }
      *p++= 0;
      if (p-desc >= max) error("Internal error: exceeded estimated description buffer");
      *descp= desc;
   }

   return rv;
}

//
//	Auto-adjust input frequency to give correct sqrt(0.5)
//	(~-3.01dB) point to 6 figures
//

#define M301DB (0.707106781186548)

static FidFilter *
auto_adjust_single(Spec *sp, double rate, double f0) {
   double a0, a1, a2;
   FidFilter *(*design)(double,double,double,int,int,double*)= filter[sp->fi].rout;
   FidFilter *rv= 0;
   double resp;
   double r0, r2;
   int incr;		// Increasing (1) or decreasing (0)
   int a;

#define DESIGN(aa) design(rate, aa, aa, sp->order, sp->n_arg, sp->argarr)
#define TEST(aa) { if (rv) {free(rv);rv= 0;} rv= DESIGN(aa); resp= fid_response(rv, f0); }

   // Try and establish a range within which we can find the point
   a0= f0; TEST(a0); r0= resp;
   for (a= 2; 1; a*=2) {
      a2= f0/a; TEST(a2); r2= resp;
      if ((r0 < M301DB) != (r2 < M301DB)) break;
      a2= 0.5-((0.5-f0)/a); TEST(a2); r2= resp;
      if ((r0 < M301DB) != (r2 < M301DB)) break;
      if (a == 32) 	// No success
	 error("auto_adjust_single internal error -- can't establish enclosing range");
   }

   incr= r2 > r0;
   if (a0 > a2) {
      a1= a0; a0= a2; a2= a1;
      incr= !incr;
   }

   // Binary search
   while (1) {
      a1= 0.5 * (a0 + a2);
      if (a1 == a0 || a1 == a2) break;		// Limit of double, sanity check
      TEST(a1);
      if (resp >= 0.9999995 * M301DB && resp < 1.0000005 * M301DB) break;
      if (incr == (resp > M301DB))
	 a2= a1;
      else
	 a0= a1;
   }

#undef TEST
#undef DESIGN

   return rv;
}


//
//	Auto-adjust input frequencies to give response of sqrt(0.5)
//	(~-3.01dB) correct to 6sf at the given frequency-points
//

static FidFilter *
auto_adjust_dual(Spec *sp, double rate, double f0, double f1) {
   double mid= 0.5 * (f0+f1);
   double wid= 0.5 * fabs(f1-f0);
   FidFilter *(*design)(double,double,double,int,int,double*)= filter[sp->fi].rout;
   FidFilter *rv= 0;
   int bpass= -1;
   double delta;
   double mid0, mid1;
   double wid0, wid1;
   double r0, r1, err0, err1;
   double perr;
   int cnt;
   int cnt_design= 0;

#define DESIGN(mm,ww) { if (rv) {free(rv);rv= 0;} \
   rv= design(rate, mm-ww, mm+ww, sp->order, sp->n_arg, sp->argarr); \
   r0= fid_response(rv, f0); r1= fid_response(rv, f1); \
   err0= fabs(M301DB-r0); err1= fabs(M301DB-r1); cnt_design++; }

#define INC_WID ((r0+r1 < 1.0) == bpass)
#define INC_MID ((r0 > r1) == bpass)
#define MATCH (err0 < 0.000000499 && err1 < 0.000000499)
#define PERR (err0+err1)

   DESIGN(mid, wid);
   bpass= (fid_response(rv, 0) < 0.5);
   delta= wid * 0.5;

   // Try delta changes until we get there
   for (cnt= 0; 1; cnt++, delta *= 0.51) {
      DESIGN(mid, wid);		// I know -- this is redundant
      perr= PERR;

      mid0= mid;
      wid0= wid;
      mid1= mid + (INC_MID ? delta : -delta);
      wid1= wid + (INC_WID ? delta : -delta);

      if (mid0 - wid1 > 0.0 && mid0 + wid1 < 0.5) {
	 DESIGN(mid0, wid1);
	 if (MATCH) break;
	 if (PERR < perr) { perr= PERR; mid= mid0; wid= wid1; }
      }

      if (mid1 - wid0 > 0.0 && mid1 + wid0 < 0.5) {
	 DESIGN(mid1, wid0);
	 if (MATCH) break;
	 if (PERR < perr) { perr= PERR; mid= mid1; wid= wid0; }
      }

      if (mid1 - wid1 > 0.0 && mid1 + wid1 < 0.5) {
	 DESIGN(mid1, wid1);
	 if (MATCH) break;
	 if (PERR < perr) { perr= PERR; mid= mid1; wid= wid1; }
      }

      if (cnt > 1000)
	 error("auto_adjust_dual -- design not converging");
   }

#undef INC_WID
#undef INC_MID
#undef MATCH
#undef PERR
#undef DESIGN

   return rv;
}


//
//	Expand a specification string to the given buffer; if out of
//	space, drops dead
//

static void
expand_spec(char *buf, char *bufend, char *str) {
   int ch;
   char *p= buf;

   while ((ch= *str++)) {
      if (p + 10 >= bufend)
	 error("Buffer overflow in fidlib expand_spec()");
      if (ch == '#') {
	 switch (*str++) {
	  case 'o': p += sprintf(p, "<optional-order>"); break;
	  case 'O': p += sprintf(p, "<order>"); break;
	  case 'F': p += sprintf(p, "<freq>"); break;
	  case 'R': p += sprintf(p, "<range>"); break;
	  case 'V': p += sprintf(p, "<value>"); break;
	  default: p += sprintf(p, "<%c>", str[-1]); break;
	 }
      } else {
	 *p++= ch;
      }
   }
   *p= 0;
}

//
//	Design a filter and reduce it to a list of all the non-const
//	coefficients.  Arguments are as for fid_filter().  The
//	coefficients are written into the given double array.  If the
//	number of coefficients doesn't match the array length given,
//	then a fatal error is generated.
//
//	Note that all 1-element FIRs and IIR first-coefficients are
//	merged into a single gain coefficient, which is returned
//	rather than being included in the coefficient list.  This is
//	to allow it to be merged with other gains within a stack of
//	filters.
//
//	The algorithm used here (merging 1-element FIRs and adjusting
//	IIR first-coefficients) must match that used in the code-
//	generating code, or else the coefficients won't match up.  The
//	'n_coef' argument provides a partial safeguard.
//

double
fid_design_coef(double *coef, int n_coef, const char *spec, double rate,
		double freq0, double freq1, int adj) {
   FidFilter *filt= fid_design(spec, rate, freq0, freq1, adj, 0);
   FidFilter *ff= filt;
   int a, len;
   int cnt= 0;
   double gain= 1.0;
   double *iir, *fir, iir_adj;
   static double const_one= 1;
   int n_iir, n_fir;
   int iir_cbm, fir_cbm;

   while (ff->typ) {
      if (ff->typ == 'F' && ff->len == 1) {
	 gain *= ff->val[0];
	 ff= FFNEXT(ff);
	 continue;
      }

      if (ff->typ != 'I' && ff->typ != 'F')
	 error("fid_design_coef can't handle FidFilter type: %c", ff->typ);

      // Initialise to safe defaults
      iir= fir= &const_one;
      n_iir= n_fir= 1;
      iir_cbm= fir_cbm= ~0;

      // See if we have an IIR filter
      if (ff->typ == 'I') {
	 iir= ff->val;
	 n_iir= ff->len;
	 iir_cbm= ff->cbm;
	 iir_adj= 1.0 / ff->val[0];
	 ff= FFNEXT(ff);
	 gain *= iir_adj;
      }

      // See if we have an FIR filter
      if (ff->typ == 'F') {
	 fir= ff->val;
	 n_fir= ff->len;
	 fir_cbm= ff->cbm;
	 ff= FFNEXT(ff);
      }

      // Dump out all non-const coefficients in reverse order
      len= n_fir > n_iir ? n_fir : n_iir;
      for (a= len-1; a>=0; a--) {
	 // Output IIR if present and non-const
	 if (a < n_iir && a>0 &&
	     !(iir_cbm & (1<<(a<15?a:15)))) {
	    if (cnt++ < n_coef) *coef++= iir_adj * iir[a];
	 }

	 // Output FIR if present and non-const
	 if (a < n_fir &&
	     !(fir_cbm & (1<<(a<15?a:15)))) {
	    if (cnt++ < n_coef) *coef++= fir[a];
	 }
      }
   }

   if (cnt != n_coef)
      error("fid_design_coef called with the wrong number of coefficients.\n"
	    "  Given %d, expecting %d: (\"%s\",%g,%g,%g,%d)",
	    n_coef, cnt, spec, rate, freq0, freq1, adj);

   free(filt);
   return gain;
}

//
//	List all the known filters to the given file handle
//

void
fid_list_filters(FILE *out) {
   int a;

   for (a= 0; filter[a].fmt; a++) {
      char buf[4096];
      expand_spec(buf, buf+sizeof(buf), filter[a].fmt);
      fprintf(out, "%s\n    ", buf);
      expand_spec(buf, buf+sizeof(buf), filter[a].txt);
      fprintf(out, "%s\n", buf);
   }
}

//
//	List all the known filters to the given buffer; the buffer is
//	NUL-terminated; returns 1 okay, 0 not enough space
//

int
fid_list_filters_buf(char *buf, char *bufend) {
   int a, cnt;
   char tmp[4096];

   for (a= 0; filter[a].fmt; a++) {
      expand_spec(tmp, tmp+sizeof(tmp), filter[a].fmt);
      buf += (cnt= snprintf(buf, bufend-buf, "%s\n    ", tmp));
      if (cnt < 0 || buf >= bufend) return 0;
      expand_spec(tmp, tmp+sizeof(tmp), filter[a].txt);
      buf += (cnt= snprintf(buf, bufend-buf, "%s\n", tmp));
      if (cnt < 0 || buf >= bufend) return 0;
   }
   return 1;
}

//
//      Do a convolution of parameters in place
//

STATIC_INLINE int
convolve(double *dst, int n_dst, double *src, int n_src) {
   int len= n_dst + n_src - 1;
   int a, b;

   for (a= len-1; a>=0; a--) {
      double val= 0;
      for (b= 0; b<n_src; b++)
         if (a-b >= 0 && a-b < n_dst)
            val += src[b] * dst[a-b];
      dst[a]= val;
   }

   return len;
}

//
//	Generate a combined filter -- merge all the IIR/FIR
//	sub-filters into a single IIR/FIR pair, and make sure the IIR
//	first coefficient is 1.0.
//

FidFilter *
fid_flatten(FidFilter *filt) {
   int m_fir= 1;	// Maximum values
   int m_iir= 1;
   int n_fir, n_iir;	// Stored counts during convolution
   FidFilter *ff;
   FidFilter *rv;
   double *fir, *iir;
   double adj;
   int a;

   // Find the size of the output filter
   ff= filt;
   while (ff->len) {
      if (ff->typ == 'I')
	 m_iir += ff->len-1;
      else if (ff->typ == 'F')
	 m_fir += ff->len-1;
      else
	 error("fid_flatten doesn't know about type %d", ff->typ);
      ff= FFNEXT(ff);
   }

   // Setup the output array
   rv= FFALLOC(2, m_iir + m_fir);
   rv->typ= 'I';
   rv->len= m_iir;
   iir= rv->val;
   ff= FFNEXT(rv);
   ff->typ= 'F';
   ff->len= m_fir;
   fir= ff->val;

   iir[0]= 1.0; n_iir= 1;
   fir[0]= 1.0; n_fir= 1;

   // Do the convolution
   ff= filt;
   while (ff->len) {
      if (ff->typ == 'I')
	 n_iir= convolve(iir, n_iir, ff->val, ff->len);
      else
	 n_fir= convolve(fir, n_fir, ff->val, ff->len);
      ff= FFNEXT(ff);
   }

   // Sanity check
   if (n_iir != m_iir ||
       n_fir != m_fir)
      error("Internal error in fid_combine() -- array under/overflow");

   // Fix iir[0]
   adj= 1.0/iir[0];
   for (a= 0; a<n_iir; a++) iir[a] *= adj;
   for (a= 0; a<n_fir; a++) fir[a] *= adj;

   return rv;
}

//
//	Parse a filter-spec and freq0/freq1 arguments.  Returns a
//	strdup'd error string on error, or else 0.
//

static char *
parse_spec(Spec *sp) {
   double *arg;
   int a;

   arg= sp->argarr;
   sp->n_arg= 0;
   sp->order= 0;
   sp->f0= 0;
   sp->f1= 0;
   sp->adj= 0;
   sp->minlen= -1;
   sp->n_freq= 0;

   for (a= 0; 1; a++) {
      char *fmt= filter[a].fmt;
      const char *p= sp->spec;
      char ch, *q;

      if (!fmt) return strdupf("Spec-string \"%s\" matches no known format", sp->spec);

      while (*p && (ch= *fmt++)) {
	 if (ch != '#') {
	    if (ch == *p++) continue;
	    p= 0; break;
	 }

	 if (isalpha(*p)) { p= 0; break; }

	 // Handling a format character
	 switch (ch= *fmt++) {
	  default:
	     return strdupf("Internal error: Unknown format #%c in format: %s",
			    fmt[-1], filter[a].fmt);
	  case 'o':
	  case 'O':
	     sp->order= (int)strtol(p, &q, 10);
	     if (p == q) {
		if (ch == 'O') goto bad;
		sp->order= 1;
	     }
	     if (sp->order <= 0)
		return strdupf("Bad order %d in spec-string \"%s\"", sp->order, sp->spec);
	     p= q; break;
	  case 'V':
	     sp->n_arg++;
	     *arg++= strtod(p, &q);
	     if (p == q) goto bad;
	     p= q; break;
	  case 'F':
	     sp->minlen= p-1-sp->spec;
	     sp->n_freq= 1;
	     sp->adj= (p[0] == '=');
	     if (sp->adj) p++;
	     sp->f0= strtod(p, &q);
	     sp->f1= 0;
	     if (p == q) goto bad;
	     p= q; break;
	  case 'R':
	     sp->minlen= p-1-sp->spec;
	     sp->n_freq= 2;
	     sp->adj= (p[0] == '=');
	     if (sp->adj) p++;
	     sp->f0= strtod(p, &q);
	     if (p == q) goto bad;
	     p= q;
	     if (*p++ != '-') goto bad;
	     sp->f1= strtod(p, &q);
	     if (p == q) goto bad;
	     if (sp->f0 > sp->f1)
		return strdupf("Backwards frequency range in spec-string \"%s\"", sp->spec);
	     p= q; break;
	 }
      }

      if (p == 0) continue;

      if (fmt[0] == '/' && fmt[1] == '#' && fmt[2] == 'F') {
	 sp->minlen= p-sp->spec;
	 sp->n_freq= 1;
	 if (sp->in_f0 < 0.0)
	    return strdupf("Frequency omitted from filter-spec, and no default provided");
	 sp->f0= sp->in_f0;
	 sp->f1= 0;
	 sp->adj= sp->in_adj;
	 fmt += 3;
      } else if (fmt[0] == '/' && fmt[1] == '#' && fmt[2] == 'R') {
	 sp->minlen= p-sp->spec;
	 sp->n_freq= 2;
	 if (sp->in_f0 < 0.0 || sp->in_f1 < 0.0)
	    return strdupf("Frequency omitted from filter-spec, and no default provided");
	 sp->f0= sp->in_f0;
	 sp->f1= sp->in_f1;
	 sp->adj= sp->in_adj;
	 fmt += 3;
      }

      // Check for trailing unmatched format characters
      if (*fmt) {
      bad:
	 return strdupf("Bad match of spec-string \"%s\" to format \"%s\"",
			sp->spec, filter[a].fmt);
      }
      if (sp->n_arg > MAXARG)
	 return strdupf("Internal error -- maximum arguments exceeded");

      // Set the minlen to the whole string if unset
      if (sp->minlen < 0) sp->minlen= p-sp->spec;

      // Save values, return
      sp->fi= a;
      return 0;
   }
   return 0;
}


//
//	Parse a filter-spec and freq0/freq1 arguments and rewrite them
//	to give an all-in-one filter spec and/or a minimum spec plus
//	separate freq0/freq1 arguments.  The all-in-one spec is
//	returned in *spec1p (strdup'd), and the minimum separated-out
//	spec is returned in *spec2p (strdup'd), *freq0p and *freq1p.
//	If either of spec1p or spec2p is 0, then that particular
//	spec-string is not generated.
//

void
fid_rewrite_spec(const char *spec, double freq0, double freq1, int adj,
		 char **spec1p,
		 char **spec2p, double *freq0p, double *freq1p, int *adjp) {
   Spec sp;
   char *err;
   sp.spec= spec;
   sp.in_f0= freq0;
   sp.in_f1= freq1;
   sp.in_adj= adj;
   err= parse_spec(&sp);
   if (err) error("%s", err);

   if (spec1p) {
      char buf[128];
      int len;
      char *rv;
      switch (sp.n_freq) {
       case 1: sprintf(buf, "/%s%.15g", sp.adj ? "=" : "", sp.f0); break;
       case 2: sprintf(buf, "/%s%.15g-%.15g", sp.adj ? "=" : "", sp.f0, sp.f1); break;
       default: buf[0]= 0;
      }
      len= strlen(buf);
      rv= (char*)Alloc(sp.minlen + len + 1);
      memcpy(rv, spec, sp.minlen);
      strcpy(rv+sp.minlen, buf);
      *spec1p= rv;
   }

   if (spec2p) {
      char *rv= (char*)Alloc(sp.minlen + 1);
      memcpy(rv, spec, sp.minlen);
      *spec2p= rv;
      *freq0p= sp.f0;
      *freq1p= sp.f1;
      *adjp= sp.adj;
   }
}

//
//	Create a FidFilter from the given double array.  The double[]
//	should contain one or more sections, each starting with the
//	filter type (either 'I' or 'F', as a double), then a count of
//	the number of coefficients following, then the coefficients
//	themselves.  The end of the list is marked with a type of 0.
//
//	This is really just a convenience function, allowing a filter
//	to be conveniently dumped to C source code and then
//	reconstructed.
//
//	Note that for more general filter generation, FidFilter
//	instances can be created simply by allocating the memory and
//	filling them in (see fidlib.h).
//

FidFilter *
fid_cv_array(double *arr) {
   double *dp;
   FidFilter *ff, *rv;
   int n_head= 0;
   int n_val= 0;

   // Scan through for sizes
   for (dp= arr; *dp; ) {
      int len, typ;

      typ= (int)(*dp++);
      if (typ != 'F' && typ != 'I')
	 error("Bad type in array passed to fid_cv_array: %g", dp[-1]);

      len= (int)(*dp++);
      if (len < 1)
	 error("Bad length in array passed to fid_cv_array: %g", dp[-1]);

      n_head++;
      n_val += len;
      dp += len;
   }

   rv= ff= (FidFilter*)Alloc(FFCSIZE(n_head, n_val));

   // Scan through to fill in FidFilter
   for (dp= arr; *dp; ) {
      int len, typ;
      typ= (int)(*dp++);
      len= (int)(*dp++);

      ff->typ= typ;
      ff->cbm= ~0;
      ff->len= len;
      memcpy(ff->val, dp, len * sizeof(double));
      dp += len;
      ff= FFNEXT(ff);
   }

   // Final element already zero'd thanks to allocation

   return rv;
}

//
//	Create a single filter from the given list of filters in
//	order.  If 'freeme' is set, then all the listed filters are
//	free'd once read; otherwise they are left untouched.  The
//	newly allocated resultant filter is returned, which should be
//	released with free() when finished with.
//

FidFilter *
fid_cat(int freeme, ...) {
   va_list ap;
   FidFilter *rv, *ff, *ff0;
   int len= 0;
   int cnt;
   char *dst;

   // Find the memory required to store the combined filter
   va_start(ap, freeme);
   while ((ff0= va_arg(ap, FidFilter*))) {
      for (ff= ff0; ff->typ; ff= FFNEXT(ff))
	 ;
      len += ((char*)ff) - ((char*)ff0);
   }
   va_end(ap);

   rv= (FidFilter*)Alloc(FFCSIZE(0,0) + len);
   dst= (char*)rv;

   va_start(ap, freeme);
   while ((ff0= va_arg(ap, FidFilter*))) {
      for (ff= ff0; ff->typ; ff= FFNEXT(ff))
	 ;
      cnt= ((char*)ff) - ((char*)ff0);
      memcpy(dst, ff0, cnt);
      dst += cnt;
      if (freeme) free(ff0);
   }
   va_end(ap);

   // Final element already zero'd
   return rv;
}

//
//	Support for fid_parse
//

// Skip white space (including comments)
static void
skipWS(char **pp) {
   char *p= *pp;

   while (*p) {
      if (isspace(*p)) { p++; continue; }
      if (*p == '#') {
         while (*p && *p != '\n') p++;
         continue;
      }
      break;
   }
   *pp= p;
}

// Grab a word from the input into the given buffer.  Returns 0: end
// of file or error, else 1: success.  Error is indicated when the
// word doesn't fit in the buffer.
static int
grabWord(char **pp, char *buf, int buflen) {
   char *p, *q;
   int len;

   skipWS(pp);
   p= *pp;
   if (!*p) return 0;

   q= p;
   if (*q == ',' || *q == ';' || *q == ')' || *q == ']' || *q == '}') {
      q++;
   } else {
      while (*q && *q != '#' && !isspace(*q) &&
	     (*q != ',' && *q != ';' && *q != ')' && *q != ']' && *q != '}'))
	 q++;
   }
   len= q-p;
   if (len >= buflen) return 0;

   memcpy(buf, p, len);
   buf[len]= 0;

   *pp= q;
   return 1;
}

//
//	Parse an entire filter specification, perhaps consisting of
//	several FIR, IIR and predefined filters.  Stops at the first
//	,; or unmatched )]}.  Returns either 0 on success, or else a
//	strdup'd error string.
//
//	This duplicates code from Fiview filter.c, I know, but this
//	may have to expand in the future to handle '+' operations, and
//	special filter types like tunable heterodyne filters.  At that
//	point, the filter.c code will have to be modified to call a
//	version of this routine.
//

char *
fid_parse(double rate, char **pp, FidFilter **ffp) {
   char buf[128];
   char *p= *pp, *rew;
#define INIT_LEN 128
   char *rv= (char*)Alloc(INIT_LEN);
   char *rvend= rv + INIT_LEN;
   char *rvp= rv;
   char *tmp;
#undef INIT_LEN
   FidFilter *curr;
   int xtra= FFCSIZE(0,0);
   int typ= -1;		// First time through
   double val;
   char dmy;

#define ERR(ptr, msg) { *pp= ptr; *ffp= 0; return msg; }
#define INCBUF { tmp= (char*)realloc(rv, (rvend-rv) * 2); if (!tmp) error("Out of memory"); \
 rvend= (rvend-rv) * 2 + tmp; rvp= (rvp-rv) + tmp; \
 curr= (FidFilter*)(((char*)curr) - rv + tmp); rv= tmp; }

   while (1) {
      rew= p;
      if (!grabWord(&p, buf, sizeof(buf))) {
	 if (*p) ERR(p, strdupf("Filter element unexpectedly long -- syntax error?"));
	 buf[0]= 0;
      }
      if (!buf[0] || !buf[1]) switch (buf[0]) {
       default:
	  break;
       case 0:
       case ',':
       case ';':
       case ')':
       case ']':
       case '}':
	  // End of filter, return it
	  tmp= (char*)realloc(rv, (rvp-rv) + xtra);
	  if (!tmp) error("Out of memory");
	  curr= (FidFilter*)((rvp-rv) + tmp);
	  curr->typ= 0; curr->cbm= 0; curr->len= 0;
	  *pp= buf[0] ? (p-1) : p;
	  *ffp= (FidFilter*)tmp;
	  return 0;
       case '/':
	  if (typ > 0) ERR(rew, strdupf("Filter syntax error; unexpected '/'"));
	  typ= 'I';
	  continue;
       case 'x':
	  if (typ > 0) ERR(rew, strdupf("Filter syntax error; unexpected 'x'"));
	  typ= 'F';
	  continue;
      }

      if (typ < 0) typ= 'F';		// Assume 'x' if missing
      if (!typ) ERR(p, strdupf("Expecting a 'x' or '/' before this"));

      if (1 != sscanf(buf, "%lf %c", &val, &dmy)) {
	 // Must be a predefined filter
	 FidFilter *ff;
	 FidFilter *ff1;
	 Spec sp;
	 double f0, f1;
	 char *err;
	 int len;

	 if (typ != 'F') ERR(rew, strdupf("Predefined filters cannot be used with '/'"));

	 // Parse the filter-spec
	 memset(&sp, 0, sizeof(sp));
	 sp.spec= buf;
	 sp.in_f0= sp.in_f1= -1;
	 if ((err= parse_spec(&sp))) ERR(rew, err);
	 f0= sp.f0;
	 f1= sp.f1;

	 // Adjust frequencies to range 0-0.5, and check them
	 f0 /= rate;
	 if (f0 > 0.5) ERR(rew, strdupf("Frequency of %gHz out of range with "
					"sampling rate of %gHz", f0*rate, rate));
	 f1 /= rate;
	 if (f1 > 0.5) ERR(rew, strdupf("Frequency of %gHz out of range with "
					"sampling rate of %gHz", f1*rate, rate));

	 // Okay we now have a successful spec-match to filter[sp.fi], and sp.n_arg
	 // args are now in sp.argarr[]

	 // Generate the filter
	 if (!sp.adj)
	    ff= filter[sp.fi].rout(rate, f0, f1, sp.order, sp.n_arg, sp.argarr);
	 else if (strstr(filter[sp.fi].fmt, "#R"))
	    ff= auto_adjust_dual(&sp, rate, f0, f1);
	 else
	    ff= auto_adjust_single(&sp, rate, f0);

	 // Append it to our FidFilter to return
	 for (ff1= ff; ff1->typ; ff1= FFNEXT(ff1)) ;
	 len= ((char*)ff1-(char*)ff);
	 while (rvp + len + xtra >= rvend) INCBUF;
	 memcpy(rvp, ff, len); rvp += len;
	 free(ff);
	 typ= 0;
	 continue;
      }

      // Must be a list of coefficients
      curr= (FidFilter*)rvp;
      rvp += xtra;
      while (rvp + sizeof(double) >= rvend) INCBUF;
      curr->typ= typ;
      curr->cbm= ~0;
      curr->len= 1;
      *(double*)rvp= val;
      rvp += sizeof(double);

      // See how many more coefficients we can pick up
      while (1) {
	 rew= p;
	 if (!grabWord(&p, buf, sizeof(buf))) {
	    if (*p) ERR(p, strdupf("Filter element unexpectedly long -- syntax error?"));
	    buf[0]= 0;
	 }
	 if (1 != sscanf(buf, "%lf %c", &val, &dmy)) {
	    p= rew;
	    break;
	 }
	 while (rvp + sizeof(double) >= rvend) INCBUF;
	 curr->len++;
	 *(double*)rvp= val;
	 rvp += sizeof(double);
      }
      typ= 0;
      continue;
   }

#undef INCBUF
#undef ERR

   return strdupf("Internal error, shouldn't reach here");
}


//
//	Filter-running code
//

#ifdef RF_COMBINED
#include "fidrf_combined.h"
#endif

#ifdef RF_CMDLIST
#include "fidrf_cmdlist.h"
#endif

#ifdef RF_JIT
#include "fidrf_jit.h"
#endif


// END //
