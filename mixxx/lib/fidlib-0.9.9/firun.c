//
//	Run fidlib filters on raw data
//
//        Copyright (c) 2004 Jim Peters <http://uazu.net/>.
//        Released under the GNU GPL version 2 as published by the
//        Free Software Foundation.  See the file COPYING for details,
//        or visit <http://www.gnu.org/copyleft/gpl.html>.
//
//	Note that this is fast, but speed increases can still be
//	obtained with special-purpose code.  As an example, one tested
//	set of filters for working on stereo 16-bit streams were twice
//	as fast when generated with fiview (using the fixed
//	coefficient example code) and put in a special-purpose wrapper
//	that only handled 16-bit integers.
//

#define NL "\n"

char *usage_text= 
"Usage: firun [options] <sampling_rate> <in_out_formats> <filter-specs...>"
NL ""
NL "Reads ASCII or raw data from STDIN, filters it, writes ASCII or raw data "
NL "to STDOUT.  See firun.txt for full details.  Brief summary:"
NL ""
NL "  <in_out_formats>: <format>  or  <format>/<format>"
NL "  <format>: %I | %R | ([a_bwWcsSf]<count-digit>*)+"
NL ""
NL "Format characters:"
NL "  a   ASCII text-formatted value       _   Dummy input byte"
NL "  b   Unsigned byte                    c   Signed byte (C 'char')"
NL "  w   Unsigned little-endian 16-bit    s   Signed little-endian 16-bit"
NL "  W   Unsigned big-endian 16-bit       S   Signed big-endian 16-bit (C 'short')"
NL "  f   Machine-format 32-bit floating point number"
NL ""
NL "Special input formats:"
NL "  %I  Ignore STDIN, generate impulse as input: 1 0 0 0 0 0 ..."
NL "  %S  Ignore STDIN, generate step as input: 1 1 1 1 1 1 ..."
NL ""
NL "Options:"
NL "  -d <dur>   Limit output to given duration, specified as:"
NL "               [<hours>h][<minutes>m][<seconds>s][<samples>], for example:"
NL "               '1h20m' or '250s' or '45m' or '88200' (samples)."
NL "  -D         Dump filters along with the signal delay in samples."
NL "  -r <cnt>   Ignore STDIN, output response of filters."
NL "  -rp <cnt>  Ignore STDIN, output response of filters, including phase."
NL "  -L         Ignore following arguments, display list of filter types."
;


//
//	Includes
//

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include "fidlib.h"

typedef unsigned char uchar;

//
//	Support code
//

static void
error(char *fmt, ...) {
   va_list ap;
   va_start(ap, fmt);
   fprintf(stderr, "firun: ");
   vfprintf(stderr, fmt, ap);
   fprintf(stderr, "\n");
   exit(1);
}

static void 
usage() {
   error("based on fidlib version %s.\n"
	 "Copyright (c) 2004 Jim Peters http://uazu.net, licensed under the GNU GPL v2.\n"
	 "\n%s", fid_version(), usage_text);
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
//	Globals
//

char *f_dur;		// Duration specification, or 0
int dur;		// Duration of output, or -1 for endless
int f_dump;		// Dump filters?
int f_resp;		// Output response? (count value if non-0)
int f_phase;		// Output phase with response?
int f_impulse;		// Input is impulse?
int f_step;		// Input is step?

double rate;		// Sampling rate

char *ispec;		// Input spec (expanded), or 0 for %I or %S
char *ospec;		// Output spec (expanded)
int n_chan;		// Number of output channels

int n_filt;		// Number of filters
FidFilter **filt;	// Loaded filters

uchar *inbuf;		// Input buffer
uchar *inbufend;	// End of input buffer allocated space, +1
uchar *inp;		// Input pointer
uchar *inend;		// End of currently-loaded data, +1
int ineof;		// Hit EOF?

//
//	Decode an input/output spec and return a char array containing
//	a NUL-term list of format characters (expanded in full,
//	without counts).  'in' should be 1 for an input-spec.
//

static char *
decode_spec(char *spec, int in) {
   char *p;
   int len= 0;
   char *rv, *sp;
   int pass;

   // Check for specials
   if (0 == strcmp(spec, "%I")) { 
      if (!in) error("%%I not valid as an output spec");
      f_impulse= 1; return 0; 
   }
   if (0 == strcmp(spec, "%S")) { 
      if (!in) error("%%S not valid as an output spec");
      f_step= 1; return 0; 
   }

   // First pass to check everything and count length
   // Second pass to fill in string
   for (pass= 0; pass<2; pass++) {
      for (p= spec; *p; ) {
	 int cnt= 0;
	 int typ= *p++;
	 if (!strchr(in ? "_abwWcsSf" : "abwWcsSf", typ)) 
	    error("Bad %s format, unknown type '%c': %s",
		  in ? "input" : "output", typ, spec);
	 if (isdigit(*p)) {
	    while (isdigit(*p)) cnt= cnt*10 + *p++ - '0';
	 } else {
	    cnt= 1;
	 }
	 if (pass == 0) {
	    len += cnt;
	 } else {
	    while (cnt-- > 0) 
	       *sp++= typ;
	 }
      }
      if (pass == 0) {
	 rv= ALLOC_ARR(len+1, char);
	 sp= rv;
      }
   }

   return rv;
}

//
//	Count the number of channels represented in an expanded spec
//

static int 
spec_count(char *spec) {
   int cnt= 0;
   char typ;
   while ((typ= *spec++)) 
      if (typ != '_') cnt++;
   return cnt;
}

//
//	Parse a list of filters from the given string.  Fill in n_filt
//	and filt[].
//

static void 
parse_filters(char *txt) {
   FidFilter *ff, **arr;
   n_filt= 0;

   while (*txt) {
      char *err= fid_parse(rate, &txt, &ff);
      if (err) error("Bad filter-specification:\n  %s", err);

      // Realloc each time, but not big hit
      arr= ALLOC_ARR(n_filt+1, FidFilter*);
      if (filt) {
	 memcpy(arr, filt, n_filt * sizeof(FidFilter*));
	 free(filt);
      }
      filt= arr;
      filt[n_filt++]= ff;

      while (isspace(*txt)) txt++;
      if (*txt == ',') txt++;
      while (isspace(*txt)) txt++;
   }
}

//
//	Output a value to STDOUT
//

static char *
output(char *op, double val) {
   char buf[32];
   int len= 0;
   int iv;

   switch (*op++) {
    case 'a': 
       len= sprintf(buf, "%.16g%c", val, *op ? ' ' : '\n'); break;
    case 'b': iv= (val+1) * 128; iv= iv<0 ? 0 : iv>255 ? 255 : iv;
       buf[0]= iv; len= 1; break;
    case 'w': iv= (val+1) * 32768; iv= iv<0 ? 0 : iv>65535 ? 65535 : iv;
       buf[0]= iv; buf[1]= iv >> 8; len= 2; break;
    case 'W': iv= (val+1) * 32768; iv= iv<0 ? 0 : iv>65535 ? 65535 : iv;
       buf[1]= iv; buf[0]= iv >> 8; len= 2; break;
    case 'c': iv= val * 128; iv= iv<-128 ? -128 : iv>127 ? 127 : iv;
       buf[0]= iv; len= 1; break;
    case 's': iv= val * 32768; iv= iv<-32768 ? -32768 : iv>32767 ? 32767 : iv;
       buf[0]= iv; buf[1]= iv >> 8; len= 2; break;
    case 'S': iv= val * 32768; iv= iv<-32768 ? -32768 : iv>32767 ? 32767 : iv;
       buf[1]= iv; buf[0]= iv >> 8; len= 2; break;
    case 'f':
       *(float*)&buf[0]= val; len= sizeof(float); break;
    case 0:
       error("Internal error in output() -- ran out of format characters"); break;
    default: 
       error("Internal error in output() -- bad format '%c'", op[-1]); break;
   }

   if (1 != fwrite(buf, len, 1, stdout))
      error("Write error");

   return op;
}

//
//	Refill input buffer as far as possible.  'ineof' is set on
//	EOF.
//

void 
refill_input() {
   if (!inbuf) {
      int len= 16384;	// 100ms of data at 44100Hz 16-bit stereo
      inbuf= ALLOC_ARR(len+1, uchar);
      inbufend= inbuf+len;
      *inbufend= 0;   // Trailing NUL so that string-reading functions can never overrun
      inp= inend= inbuf;
      ineof= 0;
   }

   // Shift down unread data
   memmove(inbuf, inp, inend-inp);
   inend -= (inp-inbuf);
   inp= inbuf;

   // Try to fill the rest of the space
   while (inend != inbufend && !ineof) {
      int rv= read(0, inend, inbufend-inend);
      if (rv <= 0) {
	 if (rv == 0) { ineof= 1; continue; }
	 if (errno == EINTR || errno == EAGAIN) continue;
	 error("Read error on stdin: %s", strerror(errno));
      }
      inend += rv;
   }
}

//
//	Input a value from STDIN.  In the case of format-code 'a',
//	care is taken not to read more than one trailing character, so
//	in theory it should be possible to mix text formats and binary
//	formats reliably (but why would anyone want to do that?)
//

static inline double
input(char **ipp) {
   char *ip= *ipp;
   double val;
   float fv;
   int ch, ch1, ch2;
   int avail;

   if (inend - inp < 32 && !ineof)
      refill_input();

   avail= inend-inp;

   while (1) {
      switch (*ip++) {
       case '_':
	  if (avail < 1) goto badeof;
	  inp++; continue;
       case 'a':
	  {
	     int cnt;
	     uchar *tmp;

	     // Skip WS (maybe lots of it)
	     while (1) {
		while (inp < inend && (ch= *inp) && 
		       (isspace(ch) || ch == ',' || ch == ';')) 
		   inp++;
		if (inp != inend) break;
		refill_input();
	     }
	     if (inend - inp < 128 && !ineof) refill_input();
	     if (inp == inend) goto badeof;

	     val= strtod(inp, (char**)&tmp);
	     if (inp == tmp) 
		error("Bad floating-point value:\n %.20s", inp);
	     inp= tmp;
	     if (inp < inend && isspace(*inp)) inp++;
	     break;
	  }
       case 'b':
	  if (avail < 1) goto badeof;
	  ch= *inp++;
	  val= (*inp++ - 128) / 128.0; 
	  break;
       case 'w':
	  if (avail < 2) goto badeof;
	  ch1= *inp++; ch2= *inp++;
	  val= (ch1 + (ch2<<8) - 32768) / 32768.0;
	  break;
       case 'W':
	  if (avail < 2) goto badeof;
	  ch1= *inp++; ch2= *inp++;
	  val= ((ch1<<8) + ch2 - 32768) / 32768.0;
	  break;
       case 'c':
	  if (avail < 1) goto badeof;
	  ch= *inp++;
	  val= ((ch^128) - 128) / 128.0; 
	  break;
       case 's':
	  if (avail < 2) goto badeof;
	  ch1= *inp++; ch2= *inp++;
	  val= (((ch1 + (ch2<<8)) ^ 32768) - 32768) / 32768.0;
	  break;
       case 'S':
	  if (avail < 2) goto badeof;
	  ch1= *inp++; ch2= *inp++;
	  val= ((((ch2<<8) + ch1) ^ 32768) - 32768) / 32768.0;
	  break;
       case 'f':
	  if (avail < sizeof(float)) goto badeof;
	  memcpy((void*)&fv, inp, sizeof(float));
	  inp += sizeof(float);
	  val= fv;
	  break;
       case 0:
	  error("Ran out of input format characters in input()");
       default:
	  error("Bad format code: '%c'", ip[-1]);
      }
      
      // We've read a value.  Now handle trailing '_'s
      while (*ip == '_') { 
	 if (inp == inend) goto badeof;
	 inp++; ip++; 
      }
      *ipp= ip;
      return val;
   }

 badeof:
   error("End of file within an input record");
   return 0;
}

//
//	Main
//

int 
main(int ac, char **av) {
   char dmy; 
   int a, b;

   // Decode arguments
   ac--; av++;
   while (ac > 0 && av[0][0] == '-') {
      char ch, *p= 1 + *av++; ac--;
      while ((ch= *p++)) {
	 switch (ch) {
	  case 'd':
	     if (ac <= 0) usage();
	     ac--; f_dur= *av++;
	     break;
	  case 'D':
	     f_dump= 1;
	     break;
	  case 'r':
	     if (ac-- <= 0) usage();
	     if (1 != sscanf(*av++, "%d %c", &f_resp, &dmy) ||
		 f_resp <= 0)
		error("Bad argument to -r: %s", av[-1]);
	     break;
	  case 'p':
	     f_phase= 1;
	     break;
	  case 'L':
	     fid_list_filters(stdout);
	     return 0;
	  default:
	     usage();
	     break;
	 }
      }
   }
   
   // Get sampling rate
   {
      if (ac-- < 1) usage();
      if (1 != sscanf(*av++, "%lf %c", &rate, &dmy))
	 usage();
      if (rate <= 0) error("Bad sampling rate: %g", rate);
   }
   
   // Get in/out spec
   {
      char *p, *spec= *av++;
      if (ac-- < 1) usage();
      p= strchr(spec, '/');
      if (p) {
	 p[0]= 0;
	 ispec= decode_spec(spec, 1);
	 ospec= decode_spec(p+1, 0);
      } else {
	 ispec= decode_spec(spec, 1);
	 ospec= decode_spec(spec, 0);
      }
   }

   // Join remaining arguments into one big string and parse
   {
      int len= 0;
      int a;
      char *arg, *p;

      for (a= 0; a<ac; a++)
	 len += 1 + strlen(av[a]);

      arg= Alloc(len+1);
      for (p= arg; ac>0; ac--, av++)
	 p += sprintf(p, "%s ", *av);
      p[-1]= 0;

      parse_filters(arg);
      free(arg);
   }

   // Dump filters, calculate delay
   if (f_dump) {
      for (a= 0; a<n_filt; a++) {
	 FidFilter *ff= filt[a];
	 int val= fid_calc_delay(ff);
	 printf("# Filter %d signal delay: %d samples\n", a+1, val);
	 while (ff->typ) {
	    printf(ff->typ == 'F' ? "  x" : "  /");
	    for (b= 0; b<ff->len; b++)
	       printf(" %.16g", ff->val[b]);
	    ff= FFNEXT(ff);
	    printf(ff->typ ? "\n" : ";\n");
	 }
      }
      return 0;
   }

   // Check everything
   {
      n_chan= spec_count(ospec);
      if (ispec && spec_count(ispec) != n_chan) 
	 error("Input and output formats must have the same number of channels");
      
      if (f_resp) {
	 int cnt= n_filt * (f_phase ? 2 : 1);
	 if (n_chan != cnt && n_chan != cnt+1)
	    error("Expecting %d or %d output channels for response output, not %d",
		  cnt, cnt+1, n_chan);
      } else {
	 if (n_filt != 1 && n_chan != n_filt) 
	    error("Number of filters (%d) must match number of channels (%d) if more than \n"
		  "one filter is specified", n_filt, n_chan);
      }
   }

   // Decode duration
   dur= -1;
   if (f_dur) {
      int val;
      char *p= f_dur;
      
      dur= 0;
      while (*p) {
	 val= 0;
	 if (!isdigit(*p)) error("Bad duration specification: '%s'", f_dur);
	 while (isdigit(*p)) val= val * 10 + *p++ - '0';
	 switch (*p) {
	  case 'h': dur += val * 60 * 60 * rate; p++; break;
	  case 'm': dur += val * 60 * rate; p++; break;
	  case 's': dur += val * rate; p++; break;
	  default: dur += val; break;
	 }
      }
   }

   // Handle response calculation
   if (f_resp) {
      int f_freq= n_chan == (n_filt * (f_phase ? 2 : 1) + 1);
      for (a= 0; a<=f_resp; a++) {
	 double freq= a * 0.5 / f_resp;
	 char *op= ospec;
	 if (f_freq) op= output(op, rate * freq);
	 for (b= 0; b<n_filt; b++) {
	    double pha;
	    double amp= fid_response_pha(filt[b], freq, &pha);
	    op= output(op, amp);
	    if (f_phase) op= output(op, pha);
	 }
      }
      return 0;
   }

   // Handle filtering operation
   {
      int noin= f_impulse || f_step;
      double val= 1.0;
      double nextval= f_step ? 1.0 : 0.0;

      FidRun **run= ALLOC_ARR(n_chan, FidRun*);
      FidFunc **dostep= ALLOC_ARR(n_chan, FidFunc*);
      void **buf= ALLOC_ARR(n_chan, void*);

      for (a= 0; a<n_chan; a++) {
	 run[a]= fid_run_new(filt[n_filt==1 ? 0 : a], &dostep[a]);
	 buf[a]= fid_run_newbuf(run[a]);
      }

      while (dur != 0) {
	 if (dur > 0) dur--;
	 
	 // Handle impulse and step response generation
	 if (noin) {
	    char *op= ospec;
	    for (a= 0; a<n_chan; a++) 
	       op= output(op, dostep[a](buf[a], val));
	    val= nextval;
	    continue;
	 }

	 // Read from input
	 {
	    char *ip= ispec;
	    char *op= ospec;
	    int chan= 0;
	    int ch;

	    if (inend - inp < 128 && !ineof)
	       refill_input();

	    if (inend == inp) return 0;

	    while (*ip) {
	       op= output(op, dostep[chan](buf[chan], input(&ip)));
	       chan++;
	    }
	 }
      }
   }

   return 0;
}

// END //	  

