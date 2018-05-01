//
//      mkfilter-derived code
//      ---------------------
//
//        Copyright (c) 2002-2004 Jim Peters <http://uazu.net/>.  This
//        file is released under the GNU Lesser General Public License
//        (LGPL) version 2.1 as published by the Free Software
//        Foundation.  See the file COPYING_LIB for details, or visit
//        <http://www.fsf.org/licenses/licenses.html>.
//
//	This is all code derived from 'mkfilter' by Tony Fisher of the
//	University of York.  I've rewritten it all in C, and given it
//	a thorough overhaul, so there is actually none of his code
//	here any more, but it is all still very strongly based on the
//	algorithms and techniques that he used in 'mkfilter'.
//
//	For those who didn't hear, Tony Fisher died in February 2000
//	at the age of 43.  See his web-site for information and a
//	tribute:
//
//        http://www-users.cs.york.ac.uk/~fisher/
//        http://www-users.cs.york.ac.uk/~fisher/tribute.html
//
//      The original C++ sources and the rest of the mkfilter tool-set
//      are still available from his site:
//
//        http://www-users.cs.york.ac.uk/~fisher/mkfilter/
//
//	
//	I've made a number of improvements and changes whilst
//	rewriting the code in C.  For example, I halved the
//	calculations required in designing the filters by storing only
//	one complex pole/zero out of each conjugate pair.  This also
//	made it much easier to output the filter as a list of
//	sub-filters without lots of searching around to match up
//	conjugate pairs later on.  Outputting as a list of subfilters
//	permits greater accuracy in calculation of the response, and
//	also in the execution of the final filter.  Also, some FIR
//	coefficients can be marked as 'constant', allowing optimised
//	routines to be generated for whole classes of filters, with
//	just the variable coefficients filled in at run-time.
//
//	On the down-side, complex numbers are not portably available
//	in C before C99, so complex calculations here are done on
//	double[] arrays with inline functions, which ends up looking
//	more like assembly language than C.  Never mind.
//

//
//	LEGAL STUFF
//	-----------
//
//	Tony Fisher released his software on his University of York
//	pages for free use and free download.  The software itself has
//	no licence terms attached, nor copyright messages, just the
//	author's name, E-mail address and date.  Nor are there any
//	licence terms indicated on the website.  I understand that
//	under the Berne convention copyright does not have to be
//	claimed explicitly, so these are in fact copyright files by
//	legal default.  However, the intention was obviously that
//	these files should be used by others.
//
//	None of this really helps, though, if we're going to try to be
//	100% legally correct, so I wrote to Anthony Moulds who is the
//	contact name on Tony Fisher's pages now.  I explained what I
//	planned to do with the code, and he answered as follows:
//
//	(Note that I was planning to use it 'as-is' at that time,
//	rather than rewrite it as I have done now)
//
//      > To: "Jim Peters" <jim@uazu.net>
//      > From: "Anthony Moulds" <anthony@cs.york.ac.uk>
//      > Subject: RE: mkfilter source
//      > Date: Tue, 29 Oct 2002 15:30:19 -0000
//      > 
//      > Hi Jim,
//      > 
//      > Thanks for your email.
//      > 
//      > The University will be happy to let you use Dr Fisher's mkfilter
//      > code since your intention is not to profit financially from his work.
//      > 
//      > It would be nice if in some way you could acknowledge his contribution.
//      > 
//      > Best wishes and good luck with your work,
//      > 
//      > Anthony Moulds
//      > Senior Experimental Officer, 
//      > Computer Science Department,  University of York,
//      > York, England, UK. Tel: 44(0)1904 434758  Fax: 44(0)19042767
//      > ============================================================
//      > 
//      > 
//      > > -----Original Message-----
//      > > From: Jim Peters [mailto:jim@uazu.net]
//      > > Sent: Monday, October 28, 2002 12:36 PM
//      > > To: anthony@cs.york.ac.uk
//      > > Subject: mkfilter source
//      > > 
//      > > 
//      > > I'm very sorry to hear (rather late, I know) that Tony Fisher died --
//      > > I've always gone straight to the filter page, rather than through his
//      > > home page.  I hope his work remains available for the future.
//      > > 
//      > > Anyway, the reason I'm writing is to clarify the status of the
//      > > mkfilter source code.  Because copyright is not claimed on the web
//      > > page nor in the source distribution, I guess that Tony's intention was
//      > > that this code should be in the public domain.  However, I would like
//      > > to check this now to avoid complications later.
//      > > 
//      > > I am using his code, modified, to provide a library of filter-design
//      > > routines for a GPL'd filter design app, which is not yet released.
//      > > The library could also be used standalone, permitting apps to design
//      > > filters at run-time rather than using hard-coded compile-time filters.
//      > > My interest in filters is as a part of my work on the OpenEEG project
//
//	So this looks pretty clear to me.  I am not planning to profit
//	from the work, so everything is fine with the University.  I
//	guess others might profit from the work, indirectly, as with
//	any free software release, but so long as I don't, we're fine.
//
//	I hope this is watertight enough for Debian/etc.  Otherwise
//	I'll have to go back to Anthony Moulds for clarification.
//
//	Even though there is no code cut-and-pasted from 'mkfilter'
//	here, it is all very obviously based on that code, so it
//	probably counts as a derived work -- although as ever "I Am
//	Not A Lawyer".
//

#ifndef FIDMK_H
#define FIDMK_H

#ifndef T_MSVC
 #ifdef HUGE_VAL
  #define INF HUGE_VAL
 #else
  #define INF (1.0/0.0)
 #endif
#endif

//Hacks for crappy linker error in MSVC... - Albert
#ifdef T_MSVC
 #undef HUGE_VAL
 #define HUGE_VAL 1.797693134862315E+308
 #define INF HUGE_VAL
#endif

#define TWOPI (2*M_PI)


//
//	Complex square root: aa= aa^0.5
//

STATIC_INLINE double
my_sqrt(double aa) {
   return aa <= 0.0 ? 0.0 : sqrt(aa);
}

// 'csqrt' clashes with builtin in GCC 4, so call it 'c_sqrt'
STATIC_INLINE void 
c_sqrt(double *aa) {
   double mag= hypot(aa[0], aa[1]);
   double rr= my_sqrt((mag + aa[0]) * 0.5);
   double ii= my_sqrt((mag - aa[0]) * 0.5);
   if (aa[1] < 0.0) ii= -ii;
   aa[0]= rr;
   aa[1]= ii;
}

//
//	Complex imaginary exponent: aa= e^i.theta
//

STATIC_INLINE void 
cexpj(double *aa, double theta) {
   aa[0]= cos(theta);
   aa[1]= sin(theta);
}

//
//	Complex exponent: aa= e^aa
//

// 'cexp' clashes with builtin in GCC 4, so call it 'c_exp'
STATIC_INLINE void 
c_exp(double *aa) {
   double mag= exp(aa[0]);
   aa[0]= mag * cos(aa[1]);
   aa[1]= mag * sin(aa[1]);
}

//
//	Context for generating filters.
//
//	Note that the poles and zeros are stored in a strange way.
//	Rather than storing both a pole (or zero) and its complex
//	conjugate, I'm storing just one of the pair.  Also, for real
//	poles, I'm not storing the imaginary part (which is zero).
//	This results in a list of numbers exactly half the length you
//	might otherwise expect.  However, since some of these numbers
//	are in pairs, and some are single, we need a separate flag
//	array to indicate which is which.  poltyp[] serves this
//	purpose.  An entry is 1 if the corresponding offset is a real
//	pole, or 2 if it is the first of a pair of values making up a
//	complex pole.  The second value of the pair has an entry of 0
//	attached.  (Similarly for zeros in zertyp[])
//

#define MAXPZ 64 

struct mk_filter_context {
   int n_pol;		// Number of poles
   double pol[MAXPZ];	// Pole values (see above)
   char poltyp[MAXPZ];	// Pole value types: 1 real, 2 first of complex pair, 0 second
   int n_zer;		// Same for zeros ...
   double zer[MAXPZ];
   char zertyp[MAXPZ];
};

//
//	Pre-warp a frequency
//

STATIC_INLINE double 
prewarp(double val) {
   return tan(val * M_PI) / M_PI;
}


//
//	Bessel poles; final one is a real value for odd numbers of
//	poles
//

static double bessel_1[]= {
 -1.00000000000e+00
};

static double bessel_2[]= {
 -1.10160133059e+00, 6.36009824757e-01,
};

static double bessel_3[]= {
 -1.04740916101e+00, 9.99264436281e-01,
 -1.32267579991e+00,
};

static double bessel_4[]= {
 -9.95208764350e-01, 1.25710573945e+00,
 -1.37006783055e+00, 4.10249717494e-01,
};

static double bessel_5[]= {
 -9.57676548563e-01, 1.47112432073e+00,
 -1.38087732586e+00, 7.17909587627e-01,
 -1.50231627145e+00,
};

static double bessel_6[]= {
 -9.30656522947e-01, 1.66186326894e+00,
 -1.38185809760e+00, 9.71471890712e-01,
 -1.57149040362e+00, 3.20896374221e-01,
};

static double bessel_7[]= {
 -9.09867780623e-01, 1.83645135304e+00,
 -1.37890321680e+00, 1.19156677780e+00,
 -1.61203876622e+00, 5.89244506931e-01,
 -1.68436817927e+00,
};

static double bessel_8[]= {
 -8.92869718847e-01, 1.99832584364e+00,
 -1.37384121764e+00, 1.38835657588e+00,
 -1.63693941813e+00, 8.22795625139e-01,
 -1.75740840040e+00, 2.72867575103e-01,
};

static double bessel_9[]= {
 -8.78399276161e-01, 2.14980052431e+00,
 -1.36758830979e+00, 1.56773371224e+00,
 -1.65239648458e+00, 1.03138956698e+00,
 -1.80717053496e+00, 5.12383730575e-01,
 -1.85660050123e+00,
};

static double bessel_10[]= {
 -8.65756901707e-01, 2.29260483098e+00,
 -1.36069227838e+00, 1.73350574267e+00,
 -1.66181024140e+00, 1.22110021857e+00,
 -1.84219624443e+00, 7.27257597722e-01,
 -1.92761969145e+00, 2.41623471082e-01,
};

static double *bessel_poles[10]= {
   bessel_1, bessel_2, bessel_3, bessel_4, bessel_5,
   bessel_6, bessel_7, bessel_8, bessel_9, bessel_10
};

//
//	Generate Bessel poles for the given order.
//

static void 
bessel(struct mk_filter_context* ctx, int order) {
   int a;

   if (order > 10) error("Maximum Bessel order is 10");
   ctx->n_pol= order;
   memcpy(ctx->pol, bessel_poles[order-1], ctx->n_pol * sizeof(double));

   for (a= 0; a<order-1; ) {
      ctx->poltyp[a++]= 2;
      ctx->poltyp[a++]= 0;
   }
   if (a < order) 
      ctx->poltyp[a++]= 1;
}

//
//	Generate Butterworth poles for the given order.  These are
//	regularly-spaced points on the unit circle to the left of the
//	real==0 line.
//

static void 
butterworth(struct mk_filter_context* ctx, int order) {
   int a;
   if (order > MAXPZ) 
      error("Maximum butterworth/chebyshev order is %d", MAXPZ);
   ctx->n_pol= order;
   for (a= 0; a<order-1; a += 2) {
      ctx->poltyp[a]= 2;
      ctx->poltyp[a+1]= 0;
      cexpj(ctx->pol+a, M_PI - (order-a-1) * 0.5 * M_PI / order);
   }
   if (a < order) {
      ctx->poltyp[a]= 1;
      ctx->pol[a]= -1.0;
   }
}

//
//	Generate Chebyshev poles for the given order and ripple.
//

static void 
chebyshev(struct mk_filter_context* ctx, int order, double ripple) {
   double eps, y;
   double sh, ch;
   int a;

   butterworth(ctx, order);
   if (ripple >= 0.0) error("Chebyshev ripple in dB should be -ve");

   eps= sqrt(-1.0 + pow(10.0, -0.1 * ripple));
   y= asinh(1.0 / eps) / order;
   if (y <= 0.0) error("Internal error; chebyshev y-value <= 0.0: %g", y);
   sh= sinh(y);
   ch= cosh(y);

   for (a= 0; a<ctx->n_pol; ) {
      if (ctx->poltyp[a] == 1)
	 ctx->pol[a++] *= sh;
      else {
	 ctx->pol[a++] *= sh;
	 ctx->pol[a++] *= ch;
      }
   }
}


//
//	Adjust raw poles to LP filter
//

static void 
lowpass(struct mk_filter_context* ctx, double freq) {
   int a;

   // Adjust poles
   freq *= TWOPI;
   for (a= 0; a<ctx->n_pol; a++)
      ctx->pol[a] *= freq;

   // Add zeros
   ctx->n_zer= ctx->n_pol;
   for (a= 0; a<ctx->n_zer; a++) {
      ctx->zer[a]= -INF;
      ctx->zertyp[a]= 1;
   }
}

//
//	Adjust raw poles to HP filter
//

static void 
highpass(struct mk_filter_context* ctx, double freq) {
   int a;

   // Adjust poles
   freq *= TWOPI;
   for (a= 0; a<ctx->n_pol; ) {
      if (ctx->poltyp[a] == 1) {
	 ctx->pol[a]= freq / ctx->pol[a];
	 a++;
      } else {
	 crecip(ctx->pol + a);
	 ctx->pol[a++] *= freq;
	 ctx->pol[a++] *= freq;
      }
   }

   // Add zeros
   ctx->n_zer= ctx->n_pol;
   for (a= 0; a<ctx->n_zer; a++) {
      ctx->zer[a]= 0.0;
      ctx->zertyp[a]= 1;
   }
}

//
//	Adjust raw poles to BP filter.  The number of poles is
//	doubled.
//

static void 
bandpass(struct mk_filter_context* ctx, double freq1, double freq2) {
   double w0= TWOPI * sqrt(freq1*freq2);
   double bw= 0.5 * TWOPI * (freq2-freq1);
   int a, b;

   if (ctx->n_pol * 2 > MAXPZ) 
      error("Maximum order for bandpass filters is %d", MAXPZ/2);
   
   // Run through the list backwards, expanding as we go
   for (a= ctx->n_pol, b= ctx->n_pol*2; a>0; ) {
      // hba= pole * bw;
      // temp= c_sqrt(1.0 - square(w0 / hba));
      // pole1= hba * (1.0 + temp);
      // pole2= hba * (1.0 - temp);

      if (ctx->poltyp[a-1] == 1) {
	 double hba;
	 a--; b -= 2;
	 ctx->poltyp[b]= 2; ctx->poltyp[b+1]= 0;
	 hba= ctx->pol[a] * bw;
	 cassz(ctx->pol+b, 1.0 - (w0 / hba) * (w0 / hba), 0.0);
	 c_sqrt(ctx->pol+b);
	 caddz(ctx->pol+b, 1.0, 0.0);
	 cmulr(ctx->pol+b, hba);
      } else {		// Assume poltyp[] data is valid
	 double hba[2];
	 a -= 2; b -= 4;
	 ctx->poltyp[b]= 2; ctx->poltyp[b+1]= 0;
	 ctx->poltyp[b+2]= 2; ctx->poltyp[b+3]= 0;
	 cass(hba, ctx->pol+a);
	 cmulr(hba, bw);
	 cass(ctx->pol+b, hba);
	 crecip(ctx->pol+b);
	 cmulr(ctx->pol+b, w0);
	 csqu(ctx->pol+b);
	 cneg(ctx->pol+b);
	 caddz(ctx->pol+b, 1.0, 0.0);
	 c_sqrt(ctx->pol+b);
	 cmul(ctx->pol+b, hba);
	 cass(ctx->pol+b+2, ctx->pol+b);
	 cneg(ctx->pol+b+2);
	 cadd(ctx->pol+b, hba);
	 cadd(ctx->pol+b+2, hba);
      } 
   }
   ctx->n_pol *= 2;
   
   // Add zeros
   ctx->n_zer= ctx->n_pol; 
   for (a= 0; a<ctx->n_zer; a++) {
      ctx->zertyp[a]= 1;
      ctx->zer[a]= (a<ctx->n_zer/2) ? 0.0 : -INF;
   }
}

//
//	Adjust raw poles to BS filter.  The number of poles is
//	doubled.
//

static void 
bandstop(struct mk_filter_context* ctx, double freq1, double freq2) {
   double w0= TWOPI * sqrt(freq1*freq2);
   double bw= 0.5 * TWOPI * (freq2-freq1);
   int a, b;

   if (ctx->n_pol * 2 > MAXPZ) 
      error("Maximum order for bandstop filters is %d", MAXPZ/2);

   // Run through the list backwards, expanding as we go
   for (a= ctx->n_pol, b= ctx->n_pol*2; a>0; ) {
      // hba= bw / pole;
      // temp= c_sqrt(1.0 - square(w0 / hba));
      // pole1= hba * (1.0 + temp);
      // pole2= hba * (1.0 - temp);

      if (ctx->poltyp[a-1] == 1) {
	 double hba;
	 a--; b -= 2;
	 ctx->poltyp[b]= 2; ctx->poltyp[b+1]= 0;
	 hba= bw / ctx->pol[a];
	 cassz(ctx->pol+b, 1.0 - (w0 / hba) * (w0 / hba), 0.0);
	 c_sqrt(ctx->pol+b);
	 caddz(ctx->pol+b, 1.0, 0.0);
	 cmulr(ctx->pol+b, hba);
      } else {		// Assume poltyp[] data is valid
	 double hba[2];
	 a -= 2; b -= 4;
	 ctx->poltyp[b]= 2; ctx->poltyp[b+1]= 0;
	 ctx->poltyp[b+2]= 2; ctx->poltyp[b+3]= 0;
	 cass(hba, ctx->pol+a);
	 crecip(hba);
	 cmulr(hba, bw);
	 cass(ctx->pol+b, hba);
	 crecip(ctx->pol+b);
	 cmulr(ctx->pol+b, w0);
	 csqu(ctx->pol+b);
	 cneg(ctx->pol+b);
	 caddz(ctx->pol+b, 1.0, 0.0);
	 c_sqrt(ctx->pol+b);
	 cmul(ctx->pol+b, hba);
	 cass(ctx->pol+b+2, ctx->pol+b);
	 cneg(ctx->pol+b+2);
	 cadd(ctx->pol+b, hba);
	 cadd(ctx->pol+b+2, hba);
      } 
   }
   ctx->n_pol *= 2;
   
   // Add zeros
   ctx->n_zer= ctx->n_pol; 
   for (a= 0; a<ctx->n_zer; a+=2) {
      ctx->zertyp[a]= 2; ctx->zertyp[a+1]= 0;
      ctx->zer[a]= 0.0; ctx->zer[a+1]= w0;
   }
}

//
//	Convert list of poles+zeros from S to Z using bilinear
//	transform
//

static void 
s2z_bilinear(struct mk_filter_context* ctx) {
   int a;
   for (a= 0; a<ctx->n_pol; ) {
      // Calculate (2 + val) / (2 - val)
      if (ctx->poltyp[a] == 1) {
	 if (ctx->pol[a] == -INF) 
	    ctx->pol[a]= -1.0;
	 else 
	    ctx->pol[a]= (2 + ctx->pol[a]) / (2 - ctx->pol[a]);
	 a++;
      } else {
	 double val[2];
	 cass(val, ctx->pol+a);
	 cneg(val);
	 caddz(val, 2, 0);
	 caddz(ctx->pol+a, 2, 0);
	 cdiv(ctx->pol+a, val);
	 a += 2;
      }
   }
   for (a= 0; a<ctx->n_zer; ) {
      // Calculate (2 + val) / (2 - val)
      if (ctx->zertyp[a] == 1) {
	 if (ctx->zer[a] == -INF) 
	    ctx->zer[a]= -1.0;
	 else 
	    ctx->zer[a]= (2 + ctx->zer[a]) / (2 - ctx->zer[a]);
	 a++;
      } else {
	 double val[2];
	 cass(val, ctx->zer+a);
	 cneg(val);
	 caddz(val, 2, 0);
	 caddz(ctx->zer+a, 2, 0);
	 cdiv(ctx->zer+a, val);
	 a += 2;
      }
   }
}

//
//	Convert S to Z using matched-Z transform
//
    
static void 
s2z_matchedZ(struct mk_filter_context* ctx) {
   int a;
   
   for (a= 0; a<ctx->n_pol; ) {
      // Calculate cexp(val)
      if (ctx->poltyp[a] == 1) {
	 if (ctx->pol[a] == -INF) 
	    ctx->pol[a]= 0.0;
	 else 
	    ctx->pol[a]= exp(ctx->pol[a]);
	 a++;
      } else {
	 c_exp(ctx->pol+a);
	 a += 2;
      }
   }

   for (a= 0; a<ctx->n_zer; ) {
      // Calculate cexp(val)
      if (ctx->zertyp[a] == 1) {
	 if (ctx->zer[a] == -INF) 
	    ctx->zer[a]= 0.0;
	 else 
	    ctx->zer[a]= exp(ctx->zer[a]);
	 a++;
      } else {
	 c_exp(ctx->zer+a);
	 a += 2;
      }
   }
}

//
//	Generate a FidFilter for the current set of poles and zeros.
//	The given gain is inserted at the start of the FidFilter as a
//	one-coefficient FIR filter.  This is positioned to be easily
//	adjusted later to correct the filter gain.
//
//	'cbm' should be a bitmap indicating which FIR coefficients are
//	constants for this filter type.  Normal values are ~0 for all
//	constant, or 0 for none constant, or some other bitmask for a
//	mixture.  Filters generated with lowpass(), highpass() and
//	bandpass() above should pass ~0, but bandstop() requires 0x5.
//
//	This routine requires that any lone real poles/zeros are at
//	the end of the list.  All other poles/zeros are handled in
//	pairs (whether pairs of real poles/zeros, or conjugate pairs).
//

static FidFilter*
z2fidfilter(struct mk_filter_context* ctx, double gain, int cbm) {
   int n_head, n_val;
   int a;
   FidFilter *rv;
   FidFilter *ff;

   n_head= 1 + ctx->n_pol + ctx->n_zer;	 // Worst case: gain + 2-element IIR/FIR
   n_val= 1 + 2 * (ctx->n_pol+ctx->n_zer); //   for each pole/zero

   rv= ff= FFALLOC(n_head, n_val);

   ff->typ= 'F';
   ff->len= 1;
   ff->val[0]= gain;
   ff= FFNEXT(ff);

   // Output as much as possible as 2x2 IIR/FIR filters
   for (a= 0; a <= ctx->n_pol-2 && a <= ctx->n_zer-2; a += 2) {
      // Look for a pair of values for an IIR
      if (ctx->poltyp[a] == 1 && ctx->poltyp[a+1] == 1) {
	 // Two real values
         ff->typ= 'I';
         ff->len= 3;
         ff->val[0]= 1;
         ff->val[1]= -(ctx->pol[a] + ctx->pol[a+1]);
         ff->val[2]= ctx->pol[a] * ctx->pol[a+1];
	 ff= FFNEXT(ff); 
      } else if (ctx->poltyp[a] == 2) {
	 // A complex value and its conjugate pair
         ff->typ= 'I';
         ff->len= 3;
         ff->val[0]= 1;
         ff->val[1]= -2 * ctx->pol[a];
         ff->val[2]= ctx->pol[a] * ctx->pol[a] + ctx->pol[a+1] * ctx->pol[a+1];
	 ff= FFNEXT(ff); 
      } else error("Internal error -- bad poltyp[] values for z2fidfilter()");	 

      // Look for a pair of values for an FIR
      if (ctx->zertyp[a] == 1 && ctx->zertyp[a+1] == 1) {
	 // Two real values
	 // Skip if constant and 0/0
	 if (!cbm || ctx->zer[a] != 0.0 || ctx->zer[a+1] != 0.0) {
	    ff->typ= 'F';
	    ff->cbm= cbm;
	    ff->len= 3;
	    ff->val[0]= 1;
	    ff->val[1]= -(ctx->zer[a] + ctx->zer[a+1]);
	    ff->val[2]= ctx->zer[a] * ctx->zer[a+1];
	    ff= FFNEXT(ff); 
	 }
      } else if (ctx->zertyp[a] == 2) {
	 // A complex value and its conjugate pair
	 // Skip if constant and 0/0
	 if (!cbm || ctx->zer[a] != 0.0 || ctx->zer[a+1] != 0.0) {
	    ff->typ= 'F';
	    ff->cbm= cbm;
	    ff->len= 3;
	    ff->val[0]= 1;
	    ff->val[1]= -2 * ctx->zer[a];
	    ff->val[2]= ctx->zer[a] * ctx->zer[a] + ctx->zer[a+1] * ctx->zer[a+1];
	    ff= FFNEXT(ff); 
	 }
      } else error("Internal error -- bad zertyp[] values");	 
   }

   // Clear up any remaining bits and pieces.  Should only be a 1x1
   // IIR/FIR.
   if (ctx->n_pol-a == 0 && ctx->n_zer-a == 0) 
      ;
   else if (ctx->n_pol-a == 1 && ctx->n_zer-a == 1) {
      if (ctx->poltyp[a] != 1 || ctx->zertyp[a] != 1) 
	 error("Internal error; bad poltyp or zertyp for final pole/zero");
      ff->typ= 'I';
      ff->len= 2;
      ff->val[0]= 1;
      ff->val[1]= -ctx->pol[a];
      ff= FFNEXT(ff); 

      // Skip FIR if it is constant and zero
      if (!cbm || ctx->zer[a] != 0.0) {
	 ff->typ= 'F';
	 ff->cbm= cbm;
	 ff->len= 2;
	 ff->val[0]= 1;
	 ff->val[1]= -ctx->zer[a];
	 ff= FFNEXT(ff); 
      }
   } else 
      error("Internal error: unexpected poles/zeros at end of list");

   // End of list
   ff->typ= 0;
   ff->len= 0;
   ff= FFNEXT(ff);
   
   rv= (FidFilter*)realloc(rv, ((char*)ff)-((char*)rv));
   if (!rv) error("Out of memory");
   return rv;
}

//
//	Setup poles/zeros for a band-pass resonator.  'qfact' gives
//	the Q-factor; 0 is a special value indicating +infinity,
//	giving an oscillator.
//

static void 
bandpass_res(struct mk_filter_context* ctx, double freq, double qfact) {
   double mag;
   double th0, th1, th2;
   double theta= freq * TWOPI;
   double val[2];
   double tmp1[2], tmp2[2], tmp3[2], tmp4[2];
   int cnt;

   ctx->n_pol= 2;
   ctx->poltyp[0]= 2; ctx->poltyp[1]= 0;
   ctx->n_zer= 2;
   ctx->zertyp[0]= 1; ctx->zertyp[1]= 1;
   ctx->zer[0]= 1; ctx->zer[1]= -1;

   if (qfact == 0.0) {
      cexpj(ctx->pol, theta);
      return;
   }

   // Do a full binary search, rather than seeding it as Tony Fisher does
   cexpj(val, theta);
   mag= exp(-theta / (2.0 * qfact));
   th0= 0; th2= M_PI;
   for (cnt= 60; cnt > 0; cnt--) {
      th1= 0.5 * (th0 + th2);
      cexpj(ctx->pol, th1);
      cmulr(ctx->pol, mag);
      
      // Evaluate response of filter for Z= val
      memcpy(tmp1, val, 2*sizeof(double));
      memcpy(tmp2, val, 2*sizeof(double));
      memcpy(tmp3, val, 2*sizeof(double));
      memcpy(tmp4, val, 2*sizeof(double));
      csubz(tmp1, 1, 0);
      csubz(tmp2, -1, 0);
      cmul(tmp1, tmp2);
      csub(tmp3, ctx->pol); cconj(ctx->pol);
      csub(tmp4, ctx->pol); cconj(ctx->pol);
      cmul(tmp3, tmp4);
      cdiv(tmp1, tmp3);
      
      if (fabs(tmp1[1] / tmp1[0]) < 1e-10) break;

      //printf("%-24.16g%-24.16g -> %-24.16g%-24.16g\n", th0, th2, tmp1[0], tmp1[1]);

      if (tmp1[1] > 0.0) th2= th1;
      else th0= th1;
   }

   if (cnt <= 0) fprintf(stderr, "Resonator binary search failed to converge");
}

//
//	Setup poles/zeros for a bandstop resonator
//

static void 
bandstop_res(struct mk_filter_context* ctx, double freq, double qfact) {
   bandpass_res(ctx, freq, qfact);
   ctx->zertyp[0]= 2; ctx->zertyp[1]= 0;
   cexpj(ctx->zer, TWOPI * freq);
}

//
//	Setup poles/zeros for an allpass resonator
//

static void 
allpass_res(struct mk_filter_context* ctx, double freq, double qfact) {
   bandpass_res(ctx, freq, qfact);
   ctx->zertyp[0]= 2; ctx->zertyp[1]= 0;
   memcpy(ctx->zer, ctx->pol, 2*sizeof(double));
   cmulr(ctx->zer, 1.0 / (ctx->zer[0]*ctx->zer[0] + ctx->zer[1]*ctx->zer[1]));
}

//
//	Setup poles/zeros for a proportional-integral filter
//

static void 
prop_integral(struct mk_filter_context* ctx, double freq) {
   ctx->n_pol= 1;
   ctx->poltyp[0]= 1;
   ctx->pol[0]= 0.0;
   ctx->n_zer= 1;
   ctx->zertyp[0]= 1;
   ctx->zer[0]= -TWOPI * freq;
}
   
// END //
#endif

