//
//	fidlib include file
//
#ifndef FIDLIB_H
#define FIDLIB_H
typedef struct FidFilter FidFilter;
struct FidFilter {
   short typ;		// Type of filter element 'I' IIR, 'F' FIR, or 0 for end of list
   short cbm;		// Constant bitmap.  Bits 0..14, if set, indicate that val[0..14]
   			//   is a constant across changes in frequency for this filter type
   			//   Bit 15, if set, indicates that val[15..inf] are constant.
   int len;		// Number of doubles stored in val[], or 0 for end of list
   double val[1];
};

// Lets you write: for (; ff->typ; ff= FFNEXT(ff)) { ... }
#define FFNEXT(ff) ((FidFilter*)((ff)->val + (ff)->len))

// Size of a sub-filter with 'cnt' double values attached
#define FFSIZE(cnt) (sizeof(FidFilter) + ((cnt)-1)*sizeof(double))

// Size required for the memory chunk to contain the given number
// headers and values, plus termination
#define FFCSIZE(n_head,n_val) ((sizeof(FidFilter)-sizeof(double))*((n_head)+1) + sizeof(double)*(n_val))

// Allocate the chunk of memory to hold a list of FidFilters, with
// n_head FidFilters and n_val total double values in the whole list.
// Includes space for the final termination, and zeros the memory.
#define FFALLOC(n_head,n_val) (FidFilter*)Alloc(FFCSIZE(n_head, n_val))

// These are so you can use easier names to refer to running filters
typedef void FidRun;
typedef double (FidFunc)(void*, double);


//
//	Prototypes
//
#ifdef __cplusplus
extern "C" {
#endif

extern void fid_set_error_handler(void(*rout)(char *));
extern char *fid_version();
extern double fid_response_pha(FidFilter *filt, double freq, double *phase);
extern double fid_response(FidFilter *filt, double freq);
extern int fid_calc_delay(FidFilter *filt);
extern FidFilter *fid_design(const char *spec, double rate, double freq0, double freq1, 
			     int f_adj, char **descp);
extern double fid_design_coef(double *coef, int n_coef, const char *spec, 
			      double rate, double freq0, double freq1, int adj);
extern void fid_list_filters(FILE *out);
extern int fid_list_filters_buf(char *buf, char *bufend);
extern FidFilter *fid_flatten(FidFilter *filt);
extern void fid_rewrite_spec(const char *spec, double freq0, double freq1, int adj, 
			     char **spec1p, char **spec2p, 
			     double *freq0p, double *freq1p, int *adjp);
extern FidFilter *fid_cv_array(double *arr);
extern FidFilter *fid_cat(int freeme, ...);
extern char *fid_parse(double rate, char **pp, FidFilter **ffp);

//
//	Filter running prototypes
//

extern void *fid_run_new(FidFilter *filt, double(**funcpp)(void *, double));
extern void *fid_run_newbuf(void *run);
extern int fid_run_bufsize(void *run);
extern void fid_run_initbuf(void *run, void *buf);
extern void fid_run_zapbuf(void *buf);
extern void fid_run_freebuf(void *runbuf);
extern void fid_run_free(void *run);

#ifdef __cplusplus
}
#endif
#endif
