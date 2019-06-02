
#ifndef QM_DSP_RESTRICT_H
#define QM_DSP_RESTRICT_H

#ifdef _MSC_VER
#define QM_R__ __restrict
#endif

#ifdef __GNUC__
#define QM_R__ __restrict__
#endif

#ifndef QM_R__
#define QM_R__
#endif

#endif
