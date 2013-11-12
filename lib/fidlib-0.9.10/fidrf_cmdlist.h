//
//	Command-list based filter-running code.  
//
//        Copyright (c) 2002-2003 Jim Peters <http://uazu.net/>.  This
//        file is released under the GNU Lesser General Public License
//        (LGPL) version 2.1 as published by the Free Software
//        Foundation.  See the file COPYING_LIB for details, or visit
//        <http://www.fsf.org/licenses/licenses.html>.
//
//	This version of the filter-running code is based on getting
//	the filter to go as fast as possible with a pre-compiled
//	routine, but without flattening the filter structure.  This
//	gives greater accuracy than the combined filter.  The result
//	is mostly faster than the combined filter (tested on ix86 with
//	gcc -O6), except where the combined filter gets a big
//	advantage from flattening the filter list.  This code is also
//	portable (unlike the JIT option).
//

typedef struct Run {
   int magic;		// Magic: 0x64966325
   int buf_size;	// Length of working buffer required in doubles	
   double *coef;	// Coefficient list
   char *cmd;		// Command list
} Run;

typedef struct RunBuf {
   double *coef;
   char *cmd;
   int mov_cnt;		// Number of bytes to memmove
   double buf[0];
} RunBuf;


//
//	Filter processing routine.  This is designed to avoid too many
//	branches, and references are very localized in the code,
//	keeping the optimizer from trying to store and remember old
//	values.
//

//
//	Step commands:
//	  0  END
//	  1  IIR coefficient (1+0)
//	  2  2x IIR coefficient (2+0)
//	  3  3x IIR coefficient (3+0)
//	  4  4Nx IIR coefficient (4N+0)
//	  5  FIR coefficient (0+1)
//	  6  2x FIR coefficient (0+2)
//	  7  3x FIR coefficient (0+3)
//	  8  4Nx FIR coefficient (0+4N)
//	  9  IIR+FIR coefficients (1+1)
//	 10  2x IIR+FIR coefficients (2+2)
//	 11  3x IIR+FIR coefficients (3+3)
//	 12  4Nx IIR+FIR coefficients (4N+4N)
//	 13  End-stage, pure IIR, assume no FIR done at all (1+0)
//	 14  End-stage with just FIR coeff (0+2)
//	 15  End-stage with IIR+FIR coeff (1+2)
//	 16  IIR + pure-IIR endstage (2+0)
//	 17  FIR + FIR end-stage (0+3)
//	 18  IIR+FIR + IIR+FIR end-stage (2+3)
//	 19  Nx (IIR + pure-IIR endstage) (2+0)
//	 20  Nx (FIR + FIR end-stage) (0+3)
//	 21  Nx (IIR+FIR + IIR+FIR end-stage) (2+3)
//	 22  Gain coefficient (0+1)
//
//	Most filters are made up of 2x2 IIR/FIR pairs, which means a
//	list of command 18 bytes.  The other big job would be long FIR
//	filters.  These have to be handled with a list of 7,6,5
//	commands, plus a 13 command.
//

typedef unsigned char uchar;

static double 
filter_step(void *fbuf, double iir) {
   double *coef= ((RunBuf*)fbuf)->coef;
   uchar *cmd= ((RunBuf*)fbuf)->cmd;
   double *buf= &((RunBuf*)fbuf)->buf[0];
   uchar ch;
   double fir= 0;
   double tmp= buf[0];
   int cnt;

   // Using a memmove first is faster on gcc -O6 / ix86 than moving
   // the values whilst working through the buffers.
   memmove(buf, buf+1, ((RunBuf*)fbuf)->mov_cnt);

#define IIR \
       iir -= *coef++ * tmp; \
       tmp= *buf++;
#define FIR \
       fir += *coef++ * tmp; \
       tmp= *buf++;
#define BOTH \
       iir -= *coef++ * tmp; \
       fir += *coef++ * tmp; \
       tmp= *buf++;
#define ENDIIR \
       iir -= *coef++ * tmp; \
       tmp= *buf++; \
       buf[-1]= iir;
#define ENDFIR \
       fir += *coef++ * tmp; \
       tmp= *buf++; \
       buf[-1]= iir; \
       iir= fir + *coef++ * iir; \
       fir= 0
#define ENDBOTH \
       iir -= *coef++ * tmp; \
       fir += *coef++ * tmp; \
       tmp= *buf++; \
       buf[-1]= iir; \
       iir= fir + *coef++ * iir; \
       fir= 0
#define GAIN \
       iir *= *coef++

   while ((ch= *cmd++)) switch (ch) {
    case 1:
       IIR; break;
    case 2:
       IIR; IIR; break;
    case 3:
       IIR; IIR; IIR; break;
    case 4:
       cnt= *cmd++; 
       do { IIR; IIR; IIR; IIR; } while (--cnt > 0);
       break;
    case 5:
       FIR; break;
    case 6:
       FIR; FIR; break;
    case 7:
       FIR; FIR; FIR; break;
    case 8:
       cnt= *cmd++; 
       do { FIR; FIR; FIR; FIR; } while (--cnt > 0);
       break;
    case 9:
       BOTH; break;
    case 10:
       BOTH; BOTH; break;
    case 11:
       BOTH; BOTH; BOTH; break;
    case 12:
       cnt= *cmd++; 
       do { BOTH; BOTH; BOTH; BOTH; } while (--cnt > 0);
       break;
    case 13:
       ENDIIR; break;
    case 14:
       ENDFIR; break;
    case 15:
       ENDBOTH; break;
    case 16:
       IIR; ENDIIR; break;
    case 17:
       FIR; ENDFIR; break;
    case 18:
       BOTH; ENDBOTH; break;
    case 19:
       cnt= *cmd++; 
       do { IIR; ENDIIR; } while (--cnt > 0);
       break;
    case 20:
       cnt= *cmd++; 
       do { FIR; ENDFIR; } while (--cnt > 0);
       break;
    case 21:
       cnt= *cmd++; 
       do { BOTH; ENDBOTH; } while (--cnt > 0);
       break;
    case 22:
       GAIN; break;
   }

#undef IIR
#undef FIR
#undef BOTH
#undef ENDIIR
#undef ENDFIR
#undef ENDBOTH
#undef GAIN

   return iir;
}


//
//	Create an instance of a filter, ready to run.  This returns a
//	void* handle, and a function to call to execute the filter.
//	Working buffers for the filter instances must be allocated
//	separately using fid_run_newbuf().  This allows many
//	simultaneous instances of the filter to be run.  
//
//	The sub-filters are executed in the precise order that they
//	are given.  This may lead to some inefficiency.  Normally when
//	an IIR filter is followed by an FIR filter, the buffers can be
//	shared.  However, if the sub-filters are not in IIR/FIR pairs,
//	then extra memory accesses are required.
//
//	In any case, factors are extracted from IIR filters (so that
//	the first coefficient is 1), and single-element FIR filters
//	are merged into the global gain factor, and are ignored.
//
//	The returned handle must be released using fid_run_free().
//

void *
fid_run_new(FidFilter *filt, double (**funcpp)(void *,double)) {
   int buf_size= 0;
   uchar *cp, prev;
   FidFilter *ff;
   double *dp;
   double gain= 1.0;
   int a;
   double *coef_tmp;
   uchar *cmd_tmp;
   int coef_cnt, coef_max;
   int cmd_cnt, cmd_max;
   int filt_cnt= 0;
   Run *rr;

   for (ff= filt; ff->len; ff= FFNEXT(ff))
      filt_cnt += ff->len;

   // Allocate worst-case sizes for temporary arrays
   coef_tmp= ALLOC_ARR(coef_max= filt_cnt + 1, double);
   cmd_tmp= (uchar*)ALLOC_ARR(cmd_max= filt_cnt + 4, char);
   dp= coef_tmp;
   cp= cmd_tmp;
   prev= 0;

   // Generate command and coefficient lists
   while (filt->len) {
      int n_iir, n_fir, cnt;
      double *iir, *fir;
      double adj;
      if (filt->typ == 'F' && filt->len == 1) {
	 gain *= filt->val[0];
	 filt= FFNEXT(filt);
	 continue;
      }
      if (filt->typ == 'F') {
	 iir= 0; n_iir= 0;
	 fir= filt->val; n_fir= filt->len;
	 filt= FFNEXT(filt);
      } else if (filt->typ == 'I') {
	 iir= filt->val; n_iir= filt->len;
	 fir= 0; n_fir= 0;
	 filt= FFNEXT(filt);
	 while (filt->typ == 'F' && filt->len == 1) {
	    gain *= filt->val[0]; 
	    filt= FFNEXT(filt);
	 }
	 if (filt->typ == 'F') {
	    fir= filt->val; n_fir= filt->len;
	    filt= FFNEXT(filt); 
	 }
      } else 
	 error("Internal error: fid_run_new can only handle IIR + FIR types");
      
      // Okay, we now have an IIR/FIR pair to process, possibly with
      // n_iir or n_fir == 0 if one half is missing
      cnt= n_iir > n_fir ? n_iir : n_fir;
      buf_size += cnt-1;
      if (n_iir) {
	 adj= 1.0 / iir[0];
	 gain *= adj;
      }
      if (n_fir == 3 && n_iir == 3) {
	 if (prev == 18) { cp[-1]= prev= 21; *cp++= 2; }
	 else if (prev == 21) { cp[-1]++; }
	 else *cp++= prev= 18;
	 *dp++= iir[2]*adj; *dp++= fir[2];
	 *dp++= iir[1]*adj; *dp++= fir[1];
	 *dp++= fir[0];
      } else if (n_fir == 3 && n_iir == 0) {
	 if (prev == 17) { cp[-1]= prev= 20; *cp++= 2; }
	 else if (prev == 20) { cp[-1]++; }
	 else *cp++= prev= 17;
	 *dp++= fir[2];
	 *dp++= fir[1];
	 *dp++= fir[0];
      } else if (n_fir == 0 && n_iir == 3) {
	 if (prev == 16) { cp[-1]= prev= 19; *cp++= 2; }
	 else if (prev == 19) { cp[-1]++; }
	 else *cp++= prev= 16;
	 *dp++= iir[2]*adj;
	 *dp++= iir[1]*adj;
      } else {
	 prev= 0;	// Just cancel 'prev' as we only use it for 16-18,19-21
	 if (cnt > n_fir) {
	    a= 0; 
	    while (cnt > n_fir && cnt > 2) {
	       *dp++= iir[--cnt] * adj; a++;
	    }
	    while (a >= 4) { 
	       int nn= a/4; if (nn > 255) nn= 255;
	       *cp++= 4; *cp++= nn; a -= nn*4; 
	    }
	    if (a) *cp++= a;
	 }
	 if (cnt > n_iir) {
	    a= 0; 
	    while (cnt > n_iir && cnt > 2) {
	       *dp++= fir[--cnt]; a++;
	    }
	    while (a >= 4) { 
	       int nn= a/4; if (nn > 255) nn= 255;
	       *cp++= 8; *cp++= nn; a -= nn*4; 
	    }
	    if (a) *cp++= 4+a;
	 }
	 a= 0;
	 while (cnt > 2) {
	    cnt--; a++;
	    *dp++= iir[cnt]*adj; *dp++= fir[cnt];
	 }
	 while (a >= 4) { 
	    int nn= a/4; if (nn > 255) nn= 255;
	    *cp++= 12; *cp++= nn; a -= nn*4; 
	 }
	 if (a) *cp++= 8+a;

	 if (!n_fir) {
	    *cp++= 13;
	    *dp++= iir[1];
	 } else if (!n_iir) {
	    *cp++= 14;
	    *dp++= fir[1];
	    *dp++= fir[0];
	 } else {
	    *cp++= 15;
	    *dp++= iir[1];
	    *dp++= fir[1];
	    *dp++= fir[0];
	 }
      }
   }
   
   if (gain != 1.0) {
      *cp++= 22;
      *dp++= gain;
   }
   *cp++= 0;

   // Sanity checks
   coef_cnt= dp-coef_tmp;
   cmd_cnt= cp-cmd_tmp;
   if (coef_cnt > coef_max ||
       cmd_cnt > cmd_max) 
      error("fid_run_new internal error; arrays exceeded");

   // Allocate the final Run structure to return
   rr= (Run*)Alloc(sizeof(Run) +
		   coef_cnt*sizeof(double) +
		   cmd_cnt*sizeof(char));
   rr->magic= 0x64966325;
   rr->buf_size= buf_size;
   rr->coef= (double*)(rr+1);
   rr->cmd= (char*)(rr->coef + coef_cnt);
   memcpy(rr->coef, coef_tmp, coef_cnt*sizeof(double));
   memcpy(rr->cmd, cmd_tmp, cmd_cnt*sizeof(char));

   //DEBUG   {
   //DEBUG      int a;
   //DEBUG      for (cp= cmd_tmp; *cp; cp++) printf("%d ", *cp);
   //DEBUG      printf("\n");
   //DEBUG      //for (a= 0; a<coef_cnt; a++) printf("%g ", coef_tmp[a]);
   //DEBUG      //printf("\n");
   //DEBUG   }
   
   free(coef_tmp);
   free(cmd_tmp);

   *funcpp= filter_step;
   return rr;
}

//
//	Create a new instance of the given filter
//

void *
fid_run_newbuf(void *run) {
   Run *rr= (Run*)run;
   RunBuf *rb;
   int siz;

   if (rr->magic != 0x64966325)
      error("Bad handle passed to fid_run_newbuf()");
   
   siz= rr->buf_size ? rr->buf_size : 1;   // Minimum one element to avoid problems
   rb= (RunBuf*)Alloc(sizeof(RunBuf) + siz * sizeof(double));
   rb->coef= rr->coef;
   rb->cmd= rr->cmd;
   rb->mov_cnt= (siz-1) * sizeof(double);
   // rb->buf[] already zerod

   return rb;
}

//
//	Find the space required for a filter buffer
//

int 
fid_run_bufsize(void *run) {
   Run *rr= (Run*)run;
   int siz;

   if (rr->magic != 0x64966325)
      error("Bad handle passed to fid_run_bufsize()");
   
   siz= rr->buf_size ? rr->buf_size : 1;   // Minimum one element to avoid problems
   return sizeof(RunBuf) + siz * sizeof(double);
}

//
//	Initialise a filter buffer allocated separately.  Usually
//	fid_run_newbuf() is easier, but in the case where filter
//	buffers must be allocated as part of a larger structure, a
//	call to fid_run_newbuf can be replaced with a call to
//	fid_run_bufsize() to get the space required, and then a call
//	to fid_run_initbuf() once it has been allocated.
//

void 
fid_run_initbuf(void *run, void *buf) {
   Run *rr= (Run*)run;
   RunBuf *rb= (RunBuf*)buf;
   int siz;

   if (rr->magic != 0x64966325)
      error("Bad handle passed to fid_run_initbuf()");
   
   siz= rr->buf_size ? rr->buf_size : 1;   // Minimum one element to avoid problems
   rb->coef= rr->coef;
   rb->cmd= rr->cmd;
   rb->mov_cnt= (siz-1) * sizeof(double);
   memset(rb->buf, 0, rb->mov_cnt + sizeof(double));
}

//
//	Reinitialise an instance of the filter, allowing it to start
//	afresh.  It assumes that the buffer was correctly initialised
//	previously, either through a call to fid_run_newbuf() or
//	fid_run_initbuf().
//

void 
fid_run_zapbuf(void *buf) {
   RunBuf *rb= (RunBuf*)buf;
   memset(rb->buf, 0, rb->mov_cnt + sizeof(double));
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
   free(run);
}

// END //
