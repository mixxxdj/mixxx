//
//	Combined-filter based filter-running code.  
//
//        Copyright (c) 2002-2003 Jim Peters <http://uazu.net/>.  This
//        file is released under the GNU Lesser General Public License
//        (LGPL) version 2.1 as published by the Free Software
//        Foundation.  See the file COPYING_LIB for details, or visit
//        <http://www.fsf.org/licenses/licenses.html>.
//
//	Convolves all the filters into a single IIR/FIR pair, and runs
//	that directly through static code.  Compiled with GCC -O6 on
//	ix86 this is surprisingly fast -- at worst half the speed of
//	assembler code, at best matching it.  The downside of
//	convolving all the sub-filters together like this is loss of
//	accuracy and instability in some kinds of filters, especially
//	high-order ones.  The one big advantage of this approach is
//	that the code is easy to understand.
//

#ifndef FIDCOMBINED_H
#define FIDCOMBINED_H

typedef struct Run {
   int magic;		// Magic: 0x64966325
   double *fir;         // FIR parameters
   int n_fir;           // Number of FIR parameters
   double *iir;         // IIR parameters
   int n_iir;           // Number of IIR parameters
   int n_buf;           // Number of entries in buffer
   FidFilter *filt;	// Combined filter
} Run;

typedef struct RunBuf {
   Run *run;
   double buf[0];
} RunBuf;

static double 
filter_step(void *rb, double val) {
   Run *rr= ((RunBuf*)rb)->run;
   double *buf= ((RunBuf*)rb)->buf;
   int a;

   // Shift the whole internal array up one
   memmove(buf+1, buf, (rr->n_buf-1)*sizeof(buf[0]));
   
   // Do IIR
   for (a= 1; a<rr->n_iir; a++) val -= rr->iir[a] * buf[a];
   buf[0]= val;

   // Do FIR
   val= 0;
   for (a= 0; a<rr->n_fir; a++) val += rr->fir[a] * buf[a];

   return val;
}


//
//	Create an instance of a filter, ready to run.  This returns a
//	void* handle, and a function to call to execute the filter.
//	Working buffers for the filter instances must be allocated
//	separately using fid_run_newbuf().  This allows many
//	simultaneous instances of the filter to be run.  
//
//	The returned handle must be released using fid_run_free().
//

void *
fid_run_new(FidFilter *filt, double (**funcpp)(void *,double)) {
   Run *rr= ALLOC(Run);
   FidFilter *ff;

   rr->magic= 0x64966325;
   rr->filt= fid_flatten(filt);

   ff= rr->filt;
   if (ff->typ != 'I') goto bad;
   rr->n_iir= ff->len;
   rr->iir= ff->val;	
   ff= FFNEXT(ff);
   if (ff->typ != 'F') goto bad;
   rr->n_fir= ff->len;
   rr->fir= ff->val;
   ff= FFNEXT(ff);
   if (ff->len) goto bad;
   
   rr->n_buf= rr->n_fir > rr->n_iir ? rr->n_fir : rr->n_iir;
   
   *funcpp= filter_step;
   
   return rr;
   
 bad:
   error("Internal error: fid_run_new() expecting IIR+FIR in flattened filter");
   return 0;
}

//
//	Create a new instance of the given filter
//

void *
fid_run_newbuf(void *run) {
   Run *rr= run;
   RunBuf *rb;

   if (rr->magic != 0x64966325)
      error("Bad handle passed to fid_run_newbuf()");
   
   rb= Alloc(sizeof(RunBuf) + rr->n_buf * sizeof(double));
   rb->run= run;
   // rb->buf[] already zerod

   return rb;
}

//
//	Reinitialise an instance ready to start afresh
//

void
fid_run_zapbuf(void *buf) {
   RunBuf *rb;
   Run *rr= rb->run;
   memset(rb->buf, 0, rr->n_buf * sizeof(double));
}

//
//	Delete an instance
//

void 
fid_run_freebuf(void *runbuf) {
   free(runbuf);
}

//
//	Delete the filter
//

void 
fid_run_free(void *run) {
   Run *rr= run;
   free(rr->filt);
   free(rr);
}

// END //
#endif

