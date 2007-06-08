#error "JIT code is no longer maintained -- cmdlist is almost as fast on ix86"

//
//	JIT-compiled filter-running code.  
//
//        Copyright (c) 2002-2003 Jim Peters <http://uazu.net/>.  This
//        file is released under the GNU Lesser General Public License
//        (LGPL) version 2.1 as published by the Free Software
//        Foundation.  See the file COPYING_LIB for details, or visit
//	  <http://www.fsf.org/licenses/licenses.html>.
//
//	The aim of this version of the filter-running code is to go as
//	fast as possible (without flattening the sub-filters together)
//	by generating the necessary code at run-time.
//
//	This runs the filter exactly as specified, without convolving
//	the sub-filters together or changing their order.  The only
//	rearrangement performed is making the IIR first coefficient
//	1.0, and gathering any lone 1-coefficient FIR filters together
//	into a single initial gain adjustment.  For this reason, the
//	routine runs fastest if IIR and FIR sub-filters are grouped
//	together in IIR/FIR pairs, as these can then share common
//	working buffers.
//
//	The generated code is cached, and is reused for more than one
//	filter if possible.  This means that a bank of 1000s of
//	filters of similar types will probably all end up sharing the
//	same generated routine, which improves processor cache and
//	memory usage.
//
//	Probably the generated code could be improved, but it is not
//	too bad.  Copying the buffer values using 'rep movsl' turned
//	out to be much faster than loading and storing the floating
//	point values individually whilst working through the buffer.
//
//	The generated code was tested for speed on a Celeron-900 and
//	on a Pentium-133.  It always beats the RF_CMDLIST option.  It
//	can be slightly slower than the RF_COMBINED option, but only
//	where that option gets a big advantage from flattening the
//	sub-filters.  For pre-flattened filters, it is faster.
//
//	The generated code can be dumped out at any point in .s format
//	using fid_run_dump().  This can be assembled using 'gas' and
//	then disassembled with 'objdump -d' to see all the generated
//	code.
//
//	Things that could be improved:
//
//	- Don't keep the fir running total on the stack at all times.
//	Instead create it at the first FIR operation.  This means
//	generating about 10 new special-case macros.  This would save
//	an add for every filter stage, and some of the messing around
//	at start and end currently done to set up / clean up this
//	value on the FP stack.
//

typedef struct Routine Routine;
struct Routine {
   Routine *nxt;	// Next in list, or 0
   int ref;		// Reference count
   int hash;		// Hash of routine
   char *code;		// Routine itself
   int len;		// Length of code in bytes
};   

typedef struct Run {
   int magic;		// Magic: 0x64966325
   int n_buf;		// Length of working buffer required in doubles	
   double *coef;	// Coefficient list
   Routine *rout;	// Routine used
} Run;

typedef struct RunBuf {
   double *coef;	// Coefficient array
   int mov_cnt;		// Number of 4-byte chunks to copy from &buf[1] to &buf[0]
   double buf[0];	// Buffer itself
} RunBuf;

static unsigned long int do_hash(unsigned char *, unsigned long int, unsigned long int);
#define HASH(p,len) ((int)do_hash((unsigned char *)p, (unsigned long int)len, 0))


//	Code generation
//
//	%edx is the working buffer pointer
//	%eax is the coefficient pointer
//	%ecx is the loop counter
//	floating point stack contains working values at the top, then
//	  previous buffer value, then running iir total, then running
//	  fir total
//
//	Codes in the add() string:
//
//	  %C  4-byte long value count for loop
//	  %L  Label -- remember this address for looping back to
//	  %R  1-byte relative jump back to %L address
//	  %D  1-byte relative address of buffer value.  If zero, this adjusts the 
//		previous byte by ^=0x40 to make it a pure (%edx) form instead of 0(%edx)
//	  %D+ 1-byte relative address of buffer value as above, plus increment %edx 
//		if we are getting close to the end of the range
//	  %A  1-byte relative address of coefficient value.  If zero does same as for %D.
//	  %A+ 1-byte relative address of coefficient value, plus %eax inc 
//		if necessary
//	  %=  Insert code to update %edx and %eax to point to the given offsets
//	
//	Startup code
//
//	  pushl %ebp
//	  movl %esp,%ebp
//	  movl 8(%ebp),%edx
//	  movl (%edx),%eax
//	  movl 4(%edx),%ecx
//	  fldz
//	  fldl 12(%ebp)
//	  fldl 8(%edx)
//	  fmull (%eax)
//	  leal 8(%edx),%edi
//	  leal 16(%edx),%esi
//	  cld
//        rep movsl

#define STARTUP add("55 89E5 8B5508 8B02 8B4A04 D9EE DD450C DD4208 DC08 8D7A08 8D7210 FC F3A5")

//	Return
//
//	  fstp %st(0)	// pop
//	  fstp %st(1)
//	  leave
//	  ret

#define RETURN add("DDD8 DDD9 C9 C3")

//	Looping
//
//	  movl $100,%ecx
//	.LXX
//	  ...
//	  loop .LXX
//
//	//WAS  decl %ecx
//	//WAS  testl %ecx,%ecx
//	//WAS  jg .LXX

#define FOR(xx, nnd, nna) add("B9%C %= %L", xx, (nnd)*8, (nna)*8)
//WAS #define NEXT(nnd, nna) add("%= 49 85C9 7F%R", (nnd)*8, (nna)*8)
#define NEXT(nnd, nna) add("%= E2%R", (nnd)*8, (nna)*8)

//	Fetching/storing buffer values
//	
//	tmp= buf[n];
//	  fldl nn(%edx)
//	
//	buf[nn]= iir;
//	  fld %st(1)
//	  fstpl nn(%edx)

#define GETB(nn) add("DD42%D+", (nn)*8)
#define PUTB(nn) add("D9C1 DD5A%D+", (nn)*8)

//	FIR element with following IIR element
//	
//	fir -= 2 * tmp;
//	  fsub %st(0),%st(2)
//	  fsub %st(0),%st(2)
//	fir -= tmp;
//	  fsub %st(0),%st(2)
//	fir += tmp;
//	  fadd %st(0),%st(2)
//	fir += 2 * tmp;
//	  fadd %st(0),%st(2)
//	  fadd %st(0),%st(2)
//	fir += coef[nn] * tmp;
//	  fld %st(0)
//	  fmull nn(%eax)
//	  faddp %st(0),%st(3)

#define FIRc_M2 add("DCEA DCEA")
#define FIRc_M1 add("DCEA")
#define FIRc_P1 add("DCC2")
#define FIRc_P2 add("DCC2 DCC2")
#define FIRc(nn) add("D9C0 DC48%A+ DEC3", (nn)*8)

//	FIR element with no following IIR element
//	
//	fir -= 2 * tmp;
//	  fsub %st(0),%st(2)
//	  fsubp %st(0),%st(2)
//	fir -= tmp;
//	  fsubp %st(0),%st(2)
//	fir += 0 * tmp;
//	  fstp %st(0),%st(0)	// Really I just want to pop the top value
//	fir += tmp;
//	  faddp %st(0),%st(2)
//	fir += 2 * tmp;
//	  fadd %st(0),%st(2)
//	  faddp %st(0),%st(2)
//	fir += coef[0] * tmp;
//	  fmull nn(%eax)
//	  faddp %st(0),%st(2)

#define FIR_M2 add("DCEA DEEA")
#define FIR_M1 add("DEEA")
#define FIR_0 add("DDD8")
#define FIR_P1 add("DEC2")
#define FIR_P2 add("DCC2 DEC2")
#define FIR(nn) add("DC48%A+ DEC2", (nn)*8)

//	IIR element
//	
//	iir -= coef[nn] * tmp;
//	  fmull nn(%eax)
//	  fsubp %st(0),%st(1)

#define IIR(nn) add("DC48%A+ DEE9", (nn)*8)

//	Final FIR element of pure-FIR or mixed FIR-IIR stage
//	
//	iir= fir + coef[nn] * iir; fir= 0;
//	  fxch
//	  fmull nn(%eax)
//	  faddp %st(2)
//	  fldz
//	  fstp %st(3)
//	iir= fir + 1.0 * iir; fir= 0;
//	  fxch
//	  faddp %st(2)
//	  fldz
//	  fstp %st(3)
//	iir= fir - 1.0 * iir; fir= 0;
//	  fxch
//	  fsubp %st(2)
//	  fldz
//	  fstp %st(3)

#define FIREND(nn) add("D9C9 DC48%A+ DEC2 D9EE DDDB", (nn)*8)
#define FIREND_P1 add("D9C9 DEC2 D9EE DDDB")
#define FIREND_M1 add("D9C9 DEEA D9EE DDDB")

//
//	Globals for handling routines
//

static char *r_buf;	// Buffer address
static char *r_end;	// Curent end of buffer
static char *r_cp;	// Current write-position
static char *r_lab;	// Current loop-back label, or 0
static int r_loop;	// Loop count
static int r_edx;	// %edx offset relative to initial position
static int r_eax;	// %eax offset relative to initial position
static Routine *r_list;	// List of routines or 0

//
//	Add code to the current routine.  This uses global variables,
//	and so is not thread-safe.
//

static void 
add(char *fmt, ...) {
   va_list ap;
   int ch, val;
   va_start(ap, fmt);

   if (r_end - r_cp < 32) 
      error("JIT error: routine buffer exceeded");

   while ((ch= *fmt++)) {
      if (isspace(ch)) continue;
      if (isdigit(ch) || (ch >= 'A' && ch <= 'F')) {
	 val= ch >= 'A' ? ch - 'A' + 10 : ch - '0';
	 ch= *fmt++;
	 if (!isdigit(ch) && !(ch >= 'A' && ch <= 'F')) 
	    error("JIT error: Bad format for add() routine");
	 val= (val*16) + (ch >= 'A' ? ch - 'A' + 10 : ch - '0');
	 *r_cp++= val;
	 continue;
      }
      if (ch != '%') 
	 error("JIT error: add() routine bad format string");
      switch (ch= *fmt++) {
       case 'C':
	  val= va_arg(ap, int);
	  r_loop= val;
	  *r_cp++= val;
	  *r_cp++= val>>8;
	  *r_cp++= val>>16;
	  *r_cp++= val>>24;
	  break;
       case 'L':
	  if (r_lab) error("JIT error: two stacked %L formats");
	  r_lab= r_cp;
	  break;
       case 'R':
	  if (!r_lab) error("JIT error: %R without matching %L");
	  val= r_lab - (r_cp+1);
	  if (val < -128) error("JIT error: %R too far from %L");
	  *r_cp++= val;
	  r_lab= 0;
	  break;
       case 'D':
	  val= va_arg(ap, int) - r_edx;
	  if (val < -128 || val >= 128) error("JIT error: %%edx offset out of range");
	  if (val == 0) 
	     r_cp[-1] ^= 0x40;
	  else 
	     *r_cp++= val;
	  if (*fmt == '+') {
	     fmt++; 
	     if (val >= 120) {
		*r_cp++= 0x83;	// addl $120,%edx
		*r_cp++= 0xC2;
		*r_cp++= 0x78;
		r_edx += 120;
	     }
	  }
	  break;
       case 'A':
	  val= va_arg(ap, int) - r_eax;
	  if (val < -128 || val >= 128) error("JIT error: %%eax offset out of range");
	  if (val == 0) 
	     r_cp[-1] ^= 0x40;
	  else 
	     *r_cp++= val;
	  if (*fmt == '+') {
	     fmt++; 
	     if (val >= 120) {
		*r_cp++= 0x83;	// addl $120,%eax
		*r_cp++= 0xC0;
		*r_cp++= 0x78;
		r_eax += 120;
	     }
	  }
	  break;
       case '=':
	  val= va_arg(ap, int) - r_edx;
	  if (val != 0) {
	     if (val < -128 || val >= 128)
		error("JIT error: %%= adjust for %%edx is out of range");
	     *r_cp++= 0x83;	// addl $120,%edx
	     *r_cp++= 0xC2;
	     *r_cp++= val;
	     r_edx += val * (r_lab ? r_loop : 1);
	  }	     
	  val= va_arg(ap, int) - r_eax;
	  if (val != 0) {
	     if (val < -128 || val >= 128)
		error("JIT error: %%= adjust for %%edx is out of range");
	     *r_cp++= 0x83;	// addl $120,%edx
	     *r_cp++= 0xC0;
	     *r_cp++= val;
	     r_eax += val * (r_lab ? r_loop : 1);
	  }	     
	  break;
       default:
	  error("JIT error: bad format for add()");
      }
   }      
}	  
      

//
//	Create an instance of a filter, ready to run.  This returns a
//	void* handle, and a JIT-compiled function to call to execute
//	the filter.  (The functions are cached, so if several versions
//	of the same filter are generated with different parameters, it
//	is likely that the same routine will end up servicing all of
//	them).
//
//	Working buffers for the filter instances must be allocated
//	separately using fid_run_newbuf().  This allows many
//	simultaneous instances of the same filter to be run.
//
//	The sub-filters are executed in the precise order that they
//	are given.  This may lead to some inefficiency, because
//	normally when an IIR filter is followed by an FIR filter, the
//	buffers can be shared.  So, if the sub-filters are not in
//	IIR/FIR pairs, then extra memory accesses are required.
//
//	In any case, factors are extracted from IIR filters (so that
//	the first coefficient is 1), and single-element FIR filters
//	are merged into the global gain factor, and are ignored.
//
//	The returned handle must be released using fid_run_free().
//

// Loop the generated code above LOOP repeats (8)
#define LOOP 8

void *
fid_run_new(FidFilter *filt, double (**funcpp)(void *,double)) {
   FidFilter *ff;
   double *dp;
   double gain= 1.0;
   int a, val;
   double *coef_tmp;
   char *rout_tmp;
   int coef_cnt, coef_max;
   int rout_cnt, rout_max;
   int filt_cnt= 0;
   Run *rr;
   int o_buf= 1;	// Current offset into working buffer
   int o_coef= 1;	// Current offset into coefficient array
   int hash;
   Routine *rout;

   for (ff= filt; ff->len; ff= FFNEXT(ff))
      filt_cnt += ff->len;

   // Allocate rough worst-case sizes for temporary arrays
   coef_tmp= ALLOC_ARR(coef_max= filt_cnt+1, double);
   rout_tmp= ALLOC_ARR(rout_max= filt_cnt * 16 + 20 + 32, char);
   dp= coef_tmp+o_coef;	// Leave space to put gain back in later

   // Setup JIT globals
   r_buf= rout_tmp;
   r_end= rout_tmp + rout_max;
   r_cp= r_buf;
   r_lab= 0;
   r_loop= 0;
   r_edx= 0;
   r_eax= 0;

   STARTUP;	// Setup iir/fir running totals on stack, apply gain 

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
      if (n_iir) {
	 adj= 1.0 / iir[0];
	 gain *= adj;
      }

      // Sort out any trailing IIR coefficients where there are more
      // IIR than FIR
      if (cnt > n_fir) {
	 a= cnt - (n_fir ? n_fir : 1);
	 if (a >= LOOP) {
	    FOR(a, o_buf, o_coef);
	    IIR(o_coef); o_coef++; 
	    GETB(o_buf); o_buf++;
	    NEXT(o_buf, o_coef);
	    o_buf += (a-1);
	    o_coef += (a-1);
	    while (a-- > 0) *dp++= iir[--cnt] * adj;
	 } else while (a-- > 0) {
	    *dp++= iir[--cnt] * adj;
	    IIR(o_coef); o_coef++; 
	    GETB(o_buf); o_buf++;
	 }
      }

      // Sort out any trailing FIR coefficients where there are more
      // FIR than IIR
      if (cnt > n_iir) {
	 a= cnt - (n_iir ? n_iir : 1);
	 if (a >= LOOP) {
	    FOR(a, o_buf, o_coef);
	    FIR(o_coef); o_coef++; 
	    GETB(o_buf); o_buf++;
	    NEXT(o_buf, o_coef);
	    o_buf += (a-1);
	    o_coef += (a-1);
	    while (a-- > 0) *dp++= fir[--cnt];
	 } else while (a-- > 0) {
	    val= fir[--cnt];
	    if (val == -2.0) FIR_M2;
	    else if (val == -1.0) FIR_M1;
	    else if (val == 0.0) FIR_0;
	    else if (val == 1.0) FIR_P1;
	    else if (val == 2.0) FIR_P2;
	    else { *dp++= val; FIR(o_coef); o_coef++; }
	    GETB(o_buf); o_buf++; 
	 }
      }

      // Sort out any common IIR/FIR coefficients remaining
      if (cnt > 1) {
	 a= cnt - 1;
	 if (a >= LOOP) {
	    FOR(a, o_buf, o_coef);
	    FIRc(o_coef); o_coef++; 
	    IIR(o_coef); o_coef++; 
	    GETB(o_buf); o_buf++;
	    NEXT(o_buf, o_coef);
	    o_buf += (a-1);
	    o_coef += 2 * (a-1);
	    while (a-- > 0) {
	       *dp++= fir[--cnt] * adj;
	       *dp++= iir[cnt] * adj;
	    }
	 } else while (a-- > 0) {
	    val= fir[--cnt];
	    if (val == -2.0) FIRc_M2;
	    else if (val == -1.0) FIRc_M1;
	    else if (val == 0.0) ; 
	    else if (val == 1.0) FIRc_P1;
	    else if (val == 2.0) FIRc_P2;
	    else { *dp++= val; FIRc(o_coef); o_coef++; }
	    
	    *dp++= iir[cnt] * adj;
	    IIR(o_coef); o_coef++;
	    GETB(o_buf); o_buf++;
	 }
      }

      // Handle the final element, according to whether there was any
      // FIR activity in this filter stage
      PUTB(o_buf-1);
      if (n_fir) {
	 if (fir[0] == 1.0) { FIREND_P1; }
	 else if (fir[0] == -1.0) { FIREND_M1; }
	 else { *dp++= fir[0]; FIREND(o_coef); o_coef++; }
      }
   }

   coef_tmp[0]= gain;
   RETURN;

   // Sanity checks
   coef_cnt= dp-coef_tmp;
   rout_cnt= r_cp-r_buf;
   if (coef_cnt > coef_max ||
       rout_cnt > rout_max) 
      error("fid_run_new internal error; arrays exceeded");

   // Now generate a hash of the code we've created, and see if we've
   // already got a cached version of that routine
   hash= HASH(rout_tmp, rout_cnt);
   for (rout= r_list; rout; rout= rout->nxt) {
      if (rout->hash == hash &&
	  rout->len == rout_cnt &&
	  0 == memcmp(rout->code, rout_tmp, rout_cnt)) 
	 break;
   }
   if (!rout) {
      rout= Alloc(sizeof(Routine) + rout_cnt);
      rout->nxt= r_list; r_list= rout;
      rout->ref= 0;
      rout->hash= hash;
      rout->code= (char*)(rout+1);
      rout->len= rout_cnt;
      memcpy(rout->code, rout_tmp, rout_cnt);
      // Maybe flush caches at this point on processors other than x86
   }
   free(rout_tmp);

   // Allocate the final Run structure to return
   rr= (Run*)Alloc(sizeof(Run) +
		   coef_cnt*sizeof(double));
   rr->magic= 0x64966325;
   rr->n_buf= o_buf;
   rr->coef= (double*)(rr+1);
   memcpy(rr->coef, coef_tmp, coef_cnt*sizeof(double));
   rr->rout= rout; 
   rout->ref++;
   
   free(coef_tmp);

   *funcpp= (void*)rout->code;
   return rr;
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
   
   rb= (RunBuf*)ALLOC_ARR(rr->n_buf, double);
   rb->coef= rr->coef;
   rb->mov_cnt= (rr->n_buf-1) * sizeof(double) / 4;

   return rb;
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
   Routine *rout= ((Run*)run)->rout;
   rout->ref--;
   if (!rout->ref) {
      // Delete the routine out of the cache
      Routine *p, **prvp;
      for (prvp= &r_list; (p= *prvp); prvp= &p->nxt) 
	 if (p == rout) {
	    *prvp= p->nxt;
	    break;
	 }
      free(rout);
   }
   free(run);
}

//
//	Dump all the routines in memory
//

void 
fid_run_dump(FILE *out) {
   Routine *rr;
   int a, cnt= 0;
   fprintf(out, 
	   "	.file	\"fid_run_dump.s\"\n"
	   "	.version	\"01.01\"\n"
	   ".text\n"
	   "	.align 4\n");
   for (rr= r_list; rr; rr= rr->nxt, cnt++) {
      fprintf(out, 
	      ".globl	process_%d\n"
	      "	.type	process_%d,@function\n"
	      "process_%d:\n",
	      cnt, cnt, cnt);
      for (a= 0; a<rr->len; a++) 
	 fprintf(out, "	.byte 0x%02X\n", 255&rr->code[a]);
      fprintf(out, 
	      ".Lfe1%d:\n"
	      "	.size	process_%d,.Lfe1%d-process_%d\n",
	      cnt, cnt, cnt, cnt);
   }
}


//
//	Hashing function.  Overkill for this job, but might as well
//	use a good one as it's available.  See below for credits.
//

typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
  * If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
  * If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a
  structure that could supported 2x parallelism, like so:
      a -= b;
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

static ub4 
do_hash(register ub1 *k,        /* the key */
	register ub4  length,   /* the length of the key */
	register ub4  initval)  /* the previous hash, or an arbitrary value */
{
   register ub4 a,b,c,len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
      {
	 a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
	 b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
	 c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
	 mix(a,b,c);
	 k += 12; len -= 12;
      }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch(len)              /* all the case statements fall through */
      {
       case 11: c+=((ub4)k[10]<<24);
       case 10: c+=((ub4)k[9]<<16);
       case 9 : c+=((ub4)k[8]<<8);
	  /* the first byte of c is reserved for the length */
       case 8 : b+=((ub4)k[7]<<24);
       case 7 : b+=((ub4)k[6]<<16);
       case 6 : b+=((ub4)k[5]<<8);
       case 5 : b+=k[4];
       case 4 : a+=((ub4)k[3]<<24);
       case 3 : a+=((ub4)k[2]<<16);
       case 2 : a+=((ub4)k[1]<<8);
       case 1 : a+=k[0];
	  /* case 0: nothing left to add */
      }
   mix(a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}

// END //
