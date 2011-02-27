/***************************************************************************
                          enginefilteriir.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
                           (C) 2010 Gabriel M. Beddingfield <gabriel@teuton.org>
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "enginefilteriir.h"
#include <cstring>
#include <cassert>
#include <memory>

#ifdef __SSE3__
#define IIR_ENABLE_SSE3
#else
#warning "Using NON-SSE3 optimized version of filter"
#endif

#ifdef IIR_ENABLE_SSE3
#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3
#endif

namespace DetailsEngineFilterIIR
{

#ifdef IIR_ENABLE_SSE3

    static const unsigned short UNALIGNED = 4;

    // Vector of 2 Double-precision Floats
    typedef __m128d __v2df;
    typedef union {
        __v2df v;
        double d[2];
    } v2df;

    typedef __m128 __v4sf;
    typedef union {
        __v4sf v;
        float f[4];
    } v4sf;

    inline bool not_aligned_16(const void* ptr)
    {
        return (((intptr_t)ptr) % 16) != 0;
    }

    inline bool aligned_16(const void* ptr)
    {
        return (((intptr_t)ptr) % 16) == 0;
    }

    /* class DetailsEngineFilterIIR::FilterSSE<Order>
     *
     * For documentation, see the process() method.
     */
    template< short Order >
    class FilterSSE : public EngineObject
    {
    public:
        FilterSSE(const double *coefs);
        ~FilterSSE();
        void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    protected:
        enum {
            ORDER = Order
        };
        unsigned short _k;
        const unsigned short _k_mask;
        v2df * _gain;
        v2df * _CXY[Order];
        v2df *_xv; // Circular buffer
        v2df *_yv; // Circular buffer, adjacent to _xv
        std::auto_ptr<v2df> _memory;
    };

#endif // IIR_ENABLE_SSE3

    class FilterReference : public EngineObject
    {
    public:
        FilterReference(int order, const double* coefs);
        ~FilterReference();
        void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    protected:
        int order;
        const double *coefs;
        enum {
            MAXNZEROS=8,
            MAXNPOLES=8
        };
        double xv1[MAXNZEROS+1], yv1[MAXNPOLES+1];
        double xv2[MAXNZEROS+1], yv2[MAXNPOLES+1];
    };

} // namespace DetailsEngineFilterIIR


EngineFilterIIR::EngineFilterIIR(const double *pCoefs, int iOrder)
{

#ifdef IIR_ENABLE_SSE3
    switch(iOrder) {
        DetailsEngineFilterIIR::FilterSSE<2> *fo2;
        DetailsEngineFilterIIR::FilterSSE<4> *fo4;
        DetailsEngineFilterIIR::FilterSSE<8> *fo8;
    case 2:
        fo2 = new DetailsEngineFilterIIR::FilterSSE<2>(pCoefs);
        _d = dynamic_cast<EngineObject*>(fo2);
        break;
    case 4:
        fo4 = new DetailsEngineFilterIIR::FilterSSE<4>(pCoefs);
        _d = dynamic_cast<EngineObject*>(fo4);
        break;
    case 8:
        fo8 = new DetailsEngineFilterIIR::FilterSSE<8>(pCoefs);
        _d = dynamic_cast<EngineObject*>(fo8);
        break;
    default:
        assert(false);
    }
#else // IIR_ENABLE_SSE3
    DetailsEngineFilterIIR::FilterReference *f;
    f = new DetailsEngineFilterIIR::FilterReference(iOrder, pCoefs);
    _d = dynamic_cast<EngineObject*>(f);
#endif

    assert(_d);
}

EngineFilterIIR::~EngineFilterIIR()
{
    delete _d;
    _d = 0;
}

void EngineFilterIIR::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    _d->process(pIn, pOut, iBufferSize);
}

namespace DetailsEngineFilterIIR
{
#ifdef IIR_ENABLE_SSE3
    template< short Order >
    FilterSSE<Order>::FilterSSE(const double *coefs) :
        _k(0),
        _k_mask(Order-1)
    {
        // ORDER must be a power of 2:
        assert( (ORDER & (ORDER-1)) == 0 );

        /* ALLOCATE ALIGNED MEMORY FOR INTERNAL VARIABLES
         *
         * SSE ops work best with 16-byte aligned memory.  This
         * allocates a large block of memory for _xv, _yv, and _CXY.
         * These variables are assigned to adjacent memory.
         *
         * Since the address returned might not be properly aligned,
         * we add extra padding so that we can ignore up to 15 bytes
         * at the beginning.  This ensures that our arrays are 16-byte
         * aligned.
         */
        int mem_reqd = 2 * (ORDER*ORDER) + 2 * ORDER + 4; // mem for _CX, _CY, _xv, _yv, padding
        _memory.reset( new v2df[mem_reqd] );
        memset(_memory.get(), 0, mem_reqd * sizeof(v2df));

        char* tmp = reinterpret_cast<char*>(_memory.get());
        v2df* end = (v2df*)tmp + mem_reqd;
        while( not_aligned_16(tmp) ) ++tmp;
        v2df* beg = (v2df*)tmp;
        assert( beg < end );

        int k;
        _gain = beg;
        _CXY[0] = _gain + 1;
        for( k=1 ; k<ORDER ; ++k ) _CXY[k] = _CXY[k-1] + (2*ORDER);
        _xv = _CXY[ORDER-1] + (2*ORDER);
        _yv = _xv + ORDER;
        assert( _yv < end );

        _gain->d[0] = coefs[0];
        _gain->d[1] = coefs[0];

        /* Initialize the _CX and _CY matrices
         * See FilterSSE::process() for documentation on what this is doing.
         */
        const short turnaround = ORDER/2;
        short c;
        _CXY[0][0].d[0] = 1.0;
        for( c=k=1 ; k <= turnaround ; ++k, ++c ) {
            _CXY[0][k].d[0] = coefs[c];
        }
        for( c-=2 ; k<ORDER ; ++k, --c ) {
            _CXY[0][k].d[0] = coefs[c];
        }
        assert( k == ORDER );
        assert( c == 0 );

        for( c=turnaround+1 ; k <= (2*ORDER) ; ++k, ++c ) {
            _CXY[0][k].d[0] = coefs[c];
        }

        // Initialize the 2nd half of the vectors.
        for( k=0 ; k<(2*ORDER) ; ++k ) {
            _CXY[0][k].d[1] = _CXY[0][k].d[0];
        }

        // Pre-shuffle the coefficients
        for( k=1 ; k < ORDER ; ++k ) { // Rows
            for( c=0 ; c < ORDER ; ++c ) { // Cols
                _CXY[k][c].v = _CXY[k-1][ _k_mask & (c-1) ].v;
                _CXY[k][ORDER+c].v = _CXY[k-1][ ORDER + (_k_mask & (c-1)) ].v;
            }
        }

    }

    template< short Order >
    FilterSSE<Order>::~FilterSSE()
    {
    }

    /* Load the 4 packed-single floats at memory location `src` and
     * store them in dest0 and dest1 as packed-doubles.  `src` is
     * assumed to be 16-byte aligned.
     */
    static inline void read_4_samples_aligned( v2df& dest0,
                                               v2df& dest1,
                                               const CSAMPLE* __restrict const src )
    {
        v4sf tmp;

        assert( aligned_16(src) );
        tmp.v = _mm_load_ps( src );
        dest0.v = _mm_cvtps_pd( tmp.v );
        tmp.v = _mm_movehl_ps( tmp.v, tmp.v );
        dest1.v = _mm_cvtps_pd( tmp.v );
    }

    /* Load previous_leftover and src[0] into dest0 as a packed
     * double.  Load src[1] and src[2] into dest1 as a packed double.
     * Load src[3] into previous_leftover.  `src` is assumed to be
     * 16-byte aligned.
     */
    static inline void read_4_samples_par_aligned( v2df& dest0,
                                                   v2df& dest1,
                                                   const CSAMPLE* __restrict const src,
                                                   double& previous_leftover )
    {
        v4sf tmp;

        assert( aligned_16(src) );
        tmp.v = _mm_load_ps( src );
        dest0.d[0] = previous_leftover;
        dest0.d[1] = (double)tmp.f[0];
        dest1.d[0] = (double)tmp.f[1];
        dest1.d[1] = (double)tmp.f[2];
        previous_leftover = (double)tmp.f[3];
    }

    /* Load the 4 packed-single floats at memory location `src` and
     * store them in dest0 and dest1 as packed-doubles.  `src` is
     * assumed to be unaligned.
     */
    static inline void read_4_samples_lame( v2df& dest0,
                                            v2df& dest1,
                                            const CSAMPLE* __restrict const src )
    {
        dest0.d[0] = (double)src[0];
        dest0.d[1] = (double)src[1];
        dest1.d[0] = (double)src[2];
        dest1.d[1] = (double)src[3];
    }

    /* Write the 4 packed-double floats in src0 and src1 to `dest` as
     * packed floats.  `dest` is assumed to be 16-byte aligned.
     */
    static inline void write_4_samples_aligned( CSAMPLE * dest,
                                                const v2df& src0,
                                                const v2df& src1 )
    {
        v4sf tmp0, tmp1;
        tmp0.v = _mm_cvtpd_ps( src0.v );
        tmp1.v = _mm_cvtpd_ps( src1.v );
        tmp0.v = _mm_movelh_ps( tmp0.v, tmp1.v );
        _mm_store_ps( dest, tmp0.v );
    }


    /* In order for write_4_samples_par_aligned() to work, the value
     * of `pending` must be initialized to have the leading unaligned
     * data... while part of the data need to be written to `dest`.
     * This function does that, according to the variable `alignment`.
     * `dest` is assumed to have an alignment matching `alignment.`
     * `dest` is assumed to NOT be 16-byte aligned.
     *
     * This function should not be used if alignment == 0.  Otherwise,
     * it should be used at the beginning of the inner loop.
     *
     * If alignment != UNALIGNED, then the number of bytes actually
     * written will be (4-alignement).  Also, the next `dest` address
     * (dest + 4 - alignment) will be 16-byte aligned.
     *
     * If alignment == UNALIGNED, then 4 bytes will be written.
     */
    static inline void write_4_samples_par_aligned_init( CSAMPLE * dest,
                                                         const v2df& src0,
                                                         const v2df& src1,
                                                         const unsigned short alignment,
                                                         v4sf& pending )
    {
        switch(alignment) {
        case 1:
            *dest++ = (float)src0.d[0];
            *dest++ = (float)src0.d[1];
            *dest++ = (float)src1.d[0];
            pending.f[0] = (float)src1.d[1];
            break;
        case 2:
            *dest++ = (float)src0.d[0];
            *dest++ = (float)src0.d[1];
            pending.f[0] = (float)src1.d[0];
            pending.f[1] = (float)src1.d[1];
            break;
        case 3:
            *dest++ = (float)src0.d[0];
            pending.f[0] = (float)src0.d[1];
            pending.f[1] = (float)src1.d[0];
            pending.f[2] = (float)src1.d[1];
            break;
        default:
            (*dest++) = (float)src0.d[0];
            (*dest++) = (float)src0.d[1];
            (*dest++) = (float)src1.d[0];
            (*dest++) = (float)src1.d[1];
        }
    }

    /* Writes the first 4 double floats in pending[], src0, and src1
     * as packed-single floats to `dest`.  `dest` is assumed to be
     * 16-byte aligned if alignment != UNALIGNED.  The first float
     * used will be pending.f[alignment].  The leftover floats will be
     * written to pending.f[alignment-1].
     *
     * If alignment == 0 or aligment == UNALIGNED, then pending is not used.
     */
    static inline void write_4_samples_par_aligned( CSAMPLE * dest,
                                                    const v2df& src0,
                                                    const v2df& src1,
                                                    const unsigned short alignment,
                                                    v4sf& pending )
    {
        assert( (alignment == UNALIGNED) || (aligned_16(dest)) );
        switch(alignment) {
        case 0:
            write_4_samples_aligned(dest, src0, src1);
            break;
        case 1:
            pending.f[1] = (float)src0.d[0];
            pending.f[2] = (float)src0.d[1];
            pending.f[3] = (float)src1.d[0];
            _mm_store_ps( dest, pending.v );
            pending.f[0] = (float)src1.d[1];
            break;
        case 2: {
            v4sf tmp0, tmp1;
            tmp0.v = pending.v;
            tmp1.v = _mm_cvtpd_ps( src0.v );
            tmp0.v = _mm_movelh_ps( tmp0.v, tmp1.v );
            _mm_store_ps( dest, tmp0.v );
            tmp0.v = _mm_cvtpd_ps( src1.v );
            _mm_store_ps( &pending.f[0], tmp0.v );
        }   break;
        case 3:
            pending.f[3] = (float)src0.d[0];
            _mm_store_ps( dest, pending.v );
            pending.f[0] = (float)src0.d[1];
            pending.f[1] = (float)src1.d[0];
            pending.f[2] = (float)src1.d[1];
            break;
        default:
            (*dest++) = (float)src0.d[0];
            (*dest++) = (float)src0.d[1];
            (*dest++) = (float)src1.d[0];
            (*dest++) = (float)src1.d[1];
        }
    }

    /* This macro requires that these variables be defined in the
     * local scope:
     *
     * const short Order;
     * __m128d *xy, *XY;
     * __m128d *CXY, *CXY_start;
     * unsigned short k, _k_mask;
     *
     * Why a macro?  Here's the other approaches:
     *
     *   - COPY/PASTE: This yielded the fastest code, but
     *     makes this critical section harder to maintain.
     *
     *   - FUNCTION: This has a significant performance
     *     penalty, so it's not an option.
     *
     *   - INLINE FUNCTION: This actually has a significant 
     *     performance penalty, too.
     *
     *   - MACRO: While it's difficult to maintain a
     *     macro, it's better than COPY/PASTE.  It's not
     *     quite as fast as COPY/PASTE, but it comes close.
     *
     * The MACRO was chosen as a compromise.
     */
    #define PROCESS_ONE_SAMPLE(in, out)               \
    {                                                 \
        __m128d xmm0, xmm1, acc, x8;                  \
        unsigned short iters;                         \
                                                      \
        x8 = (in).v / GAIN;                           \
        acc = x8;                                     \
        iters =  2*Order;                             \
        xy = XY;                                      \
        /* Compute dot product */                     \
        while(iters--) {                              \
            /* out.v += (*CXY).v * (*xy).v */         \
            /* ++CXY; ++xy; */                        \
            xmm0 = _mm_load_pd((double*)CXY++);       \
            xmm1 = _mm_load_pd((double*)xy++);        \
            xmm0 = _mm_mul_pd(xmm0, xmm1);            \
            acc = _mm_add_pd(xmm0, acc);              \
        }                                             \
        _mm_store_pd( &(out).d[0], acc );             \
        _mm_store_pd( (double*)(XY+k), x8 );          \
        _mm_store_pd( (double*)(XY+k+Order), acc );   \
                                                      \
        ++k;                                          \
        k &= _k_mask;                                 \
        if( k == 0 )                                  \
            CXY = CXY_start;                          \
    }


    /* Runs inner loop of FilterSSE<Order>::process(), optimized for
     * `*dest_` and `src` both being 16-byte aligned.
     *
     * Returns the value of CXY for the next iteration.
     */
    template <short Order>
    static inline __m128d* inner_loop_aligned_4_stride( CSAMPLE** dest_,
                                                        const CSAMPLE* src /* begin */,
                                                        const CSAMPLE* src_end,
                                                        unsigned short& k,
                                                        const unsigned short _k_mask,
                                                        const __m128d GAIN,
                                                        __m128d * const XY,
                                                        __m128d * CXY,
                                                        __m128d * const CXY_start,
                                                        __m128d * const CXY_end )
    {
        __m128d *xy;
        CSAMPLE *dest = (*dest_);

        assert( aligned_16(src) );
        assert( aligned_16(dest) );
        assert( aligned_16(src_end) );

        for( ; src != src_end ; src += 4 )
        {
            v2df in0, in1, out0, out1;
            read_4_samples_aligned(in0, in1, src);
            PROCESS_ONE_SAMPLE(in0, out0);
            PROCESS_ONE_SAMPLE(in1, out1);
            write_4_samples_aligned(dest, out0, out1);
            dest += 4;
        }
        (*dest_) = dest;
        return CXY;
    }

    /* Runs inner loop of FilterSSE<Order>::process(), optimized for
     * `*dest_` and `src` both being 4-byte aligned.  However, if they
     * are both 16-byte aligned, you should use
     * inner_loop_aligned_4_stride().
     *
     * Returns the value of CXY for the next iteration.
     */
    template <short Order>
    static inline __m128d* inner_loop_par_aligned_4_stride( CSAMPLE** dest_,
                                                            const CSAMPLE* src /* begin */,
                                                            const CSAMPLE* src_end,
                                                            unsigned short& k,
                                                            const unsigned short _k_mask,
                                                            const __m128d GAIN,
                                                            __m128d * const XY,
                                                            __m128d * CXY,
                                                            __m128d * const CXY_start,
                                                            __m128d * const CXY_end,
                                                            const unsigned short alignment_in,
                                                            const unsigned short alignment_out,
                                                            double& leftover_in,
                                                            v4sf& pending_ )
    {
        __m128d *xy;
        v4sf pending = pending_;
        CSAMPLE *dest = (*dest_);

        assert( aligned_16(src) );
        assert( (((intptr_t)dest) & 0x3) == 0 ); // 4-byte aligned
        assert( aligned_16(src_end) );

        /* first run */
        {
            v2df in0, in1, out0, out1;
            if( (alignment_in == 0) || (alignment_in == 2) ) {
                read_4_samples_aligned(in0, in1, src);
            } else {
                read_4_samples_par_aligned(in0, in1, src, leftover_in);
            }
            PROCESS_ONE_SAMPLE(in0, out0);
            PROCESS_ONE_SAMPLE(in1, out1);
            write_4_samples_par_aligned_init(dest, out0, out1, alignment_out, pending);
            dest += (4 - alignment_out);
            assert( aligned_16(dest) );
            src += 4;
        }
        for( ; src != src_end ; src += 4 )
        {
            v2df in0, in1, out0, out1;
            if( (alignment_in == 0) || (alignment_in == 2) ) {
                read_4_samples_aligned(in0, in1, src);
            } else {
                read_4_samples_par_aligned(in0, in1, src, leftover_in);
            }
            PROCESS_ONE_SAMPLE(in0, out0);
            PROCESS_ONE_SAMPLE(in1, out1);
            write_4_samples_par_aligned(dest, out0, out1, alignment_out, pending);
            dest += 4;
        }
        (*dest_) = dest;
        pending_ = pending;
        return CXY;
    }

    /* Runs inner loop of FilterSSE<Order>::process(), assuming that
     * `*dest_` or `src` are 1-byte aligned.
     *
     * Returns the value of CXY for the next iteration.
     */
    template <short Order>
    static inline __m128d* inner_loop_lame_4_stride( CSAMPLE** dest_,
                                                     const CSAMPLE* src /* begin */,
                                                     const CSAMPLE* src_end,
                                                     unsigned short& k,
                                                     const unsigned short _k_mask,
                                                     const __m128d GAIN,
                                                     __m128d * const XY,
                                                     __m128d * CXY,
                                                     __m128d * const CXY_start,
                                                     __m128d * const CXY_end )
    {
        __m128d *xy;
        CSAMPLE *dest = (*dest_);
        v4sf pending = {{0, 0, 0, 0}};

        for( ; src != src_end ; src += 4 )
        {
            v2df in0, in1, out0, out1;

            read_4_samples_lame(in0, in1, src);
            PROCESS_ONE_SAMPLE(in0, out0);
            PROCESS_ONE_SAMPLE(in1, out1);
            write_4_samples_par_aligned(dest, out0, out1, UNALIGNED, pending);
            dest += 4;
        }

        (*dest_) = dest;
        return CXY;
    }

    /* DetailsEngineFilterIIR::FilterSSE<Order>::process()
     *
     * This is an SSE-optimized version of the IIR filter.  It does
     * the same calculations as FilterReference, but is organized to
     * make it easy for the compiler to use SIMD instructions.  If you
     * look at FilterReference::process(), it's pretty clear what
     * we're doing:
     *
     *   - There is a rolling buffer of 9 'xv' coefficients.
     *     The current input sample is multiplied by a gain
     *     and added to the end of the buffer.
     *
     *   - There is a rolling buffer of 9 'yv' coefficients.
     *     The yv[8] = fun(xv[0..8], yv[0..7], coefs[1..12])
     *
     *   - Both LEFT and RIGHT channels are calculated independently,
     *     but in parallel.
     *
     *   - Math uses double-precision floats because the
     *     filter becomes unstable using single-precision math.
     *
     * The primary optimizations for this implementation are:
     *
     *   - Calculate Left and Right channels at the same time,
     *     using "Packed Single" floating point opts.
     *
     *   - Eliminate the memory shuffle (xv1[0]=xv1[1]; xv1[1]=...)
     *     by using a ring-buffer.
     *
     *   - To facilitate the ring-buffer, the coefficients are
     *     pre-shuffled into a matrix, and the correct row
     *     selected depending on the current position of the
     *     ring buffers.
     *
     *   - The xv and yv ring buffers are adjacent in memory.
     *
     *   - Since xv and yv are adjacent in memory, their
     *     coefficients are also adjacent in the form of
     *     a partitioned matrix.
     *
     * In the case of an 8th order filter, the calculations on each
     * run are like this (in pseudocode):
     *
     *     unsigned i; // sample position
     *     xv[0..7] = xv[1..8];
     *     xv[8] = in[i]/coefs[0];
     *     yv[0..7] = yv[1..8];
     *     yv[8] = xv[0] + coefs[1] * xv[1] + coefs[2] * xv[2] + coefs[3] * xv[3] + coefs[4] * xv[4]
     *             + coefs[3] * xv[5] + coefs[2] * xv[6] + coefs[1] * xv[7] + xv[8]
     *             + coefs[5] * yv[0] + coefs[6] * yv[1] + coefs[7] * yv[2] + coefs[8] * yv[3]
     *             + coefs[9] * yv[4] + coefs[10] * yv[5] + coefs[11] * yv[6] + coefs[12] * yv[7];
     *     out[i] = yv[8];
     *
     * If you remove xv[8] and yv[8], you see that we can rearrange
     * the calculation into vector form:
     *
     * The rolling buffer:
     *
     *     XY = [ xv[0] xv[1] xv[2] ... xv[7] yv[0] yv[1] ... yv[7] ]
     *
     * The coefficients (using c[] instead of coefs[]):
     *
     *     CXY = [ 1.0 c[1] c[2] c[3] c[4] c[3] c[2] c[1]     c[5] c[6] ... coefs[12] ]
     *
     * Now the calculations take a form like this:
     *
     *     float *in, *out;
     *     xtmp = last_xtmp; ytmp = last_ytmp;
     *     for( i=0 ; i<=N ; ++i ) {
     *         shuffle(XY, xtmp, ytmp); //
     *         xtmp = in[i] / coefs[0];
     *         ytmp = xtmp + dot_product(XY, CXY);
     *         out[i] = ytmp
     *     }
     *
     * Notice that the shuffle is pre-initialized from last time.  So,
     * as long as things are initilized... we can move the shuffle to
     * the /end/ of the loop and get the same results.  Like this:
     *
     *     float *in, *out;
     *     int i;
     *     if(first_run) { xtmp = 0.0; ytmp = 0.0 }
     *     for( i=0 ; i<=N ; ++i ) {
     *         xtmp = in[i] / coefs[0];
     *         ytmp = xtmp + dot_product(XY, CXY);
     *         out[i] = ytmp
     *         shuffle(XY, xtmp, ytmp);
     *     }
     *
     * The best way to optimize the shuffle speed is to *not* shuffle.
     * Since the number of coefficients in the rolling buffer is
     * the same as the order... and always a power of 2... a ring
     * buffer is a good choice.  So, we change XY into two partitioned
     * ring buffers[1] (uxing x and y instead of xv and yv):
     *
     *     int k; // Ring buffer position
     *     if(k>7) k = 0;
     *     if(k==0) XY = [ x[0] x[1] x[2] x[3] x[4] x[5] x[6] x[7]   y[0] y[1] y[2] y[3] y[4] y[5] y[6] y[7] ]
     *     if(k==1) XY = [ x[7] x[0] x[1] x[2] x[3] x[4] x[5] x[6]   y[7] y[0] y[1] y[2] y[3] y[4] y[5] y[6] ]
     *     if(k==2) XY = [ x[6] x[7] x[0] x[1] x[2] x[3] x[4] x[5]   y[6] y[7] y[0] y[1] y[2] y[3] y[4] y[5] ]
     *     ...
     *     if(k==7) XY = [ x[1] x[2] x[3] x[4] x[5] x[6] x[7] x[0]   y[1] y[2] y[3] y[4] y[5] y[6] y[7] y[0] ]
     *
     * But when we do this, we can no longer use the dot product.
     * In order to keep using the dot product, we pre-shuffle the
     * coefficients to match the current position in the ring buffer.
     *
     * CXY = [ [   1  c[1] c[2] c[3] c[4] c[3] c[2] c[1]     c[5] c[6] c[7] c[8] c[9] c[10] c[11] c[12] ]
     *         [ c[1]   1  c[1] c[2] c[3] c[4] c[3] c[2]     c[12] c[5] c[6] c[7] c[8] c[9] c[10] c[11] ]
     *         [ c[2] c[1]   1  c[1] c[2] c[3] c[4] c[3]     c[11] c[12] c[5] c[6] c[7] c[8] c[9] c[10] ]
     *         ...
     *         [ c[1] c[2] c[3] c[4] c[3] c[2] c[1]   1      c[6] c[7] c[8] c[9] c[10] c[11] c[12] c[5] ]
     *
     * Note that there are 8 rows.  This changes our calculation to this:
     *
     *     float *in, *out;
     *     int i, k;
     *     if(first_run) { xtmp = 0.0; ytmp = 0.0; }
     *     k = k_from_last_run;
     *     for( i=0 ; i<=N ; ++i ) {
     *         xtmp = in[i] / coefs[0];
     *         ytmp = xtmp + dot_product(XY, CXY[k]);
     *         out[i] = ytmp
     *         XY[k] = xtmp;  XY[8+k] = ytmp;
     *         ++k;
     *         k &= 7; // see [2]
     *     }
     *
     * At this point, all we've done is optimize the shuffle.  To get
     * parallel caluclations, we make each x[] and y[] a VECTOR of
     * size 2 (Left and Right).  That's actually done with the union
     * v2df (defined above).  So, XY and CXY are arrays of v2df
     * instead of double.
     *
     * FINALLY, SSE optimizations on x86 hardware requires that the
     * memory addresses used be 16-byte aligned.[3][4] The next-best
     * is to work with 4-byte alignments.  However, with all the API's
     * that Mixxx uses, we have no gaurantees on alighnment.  This
     * routine is optimized for 16-byte alignment, gives good
     * performance with 4-byte aligment, and works with 1-byte
     * alignment.  To handle this, this function has 3 sections: the
     * lead-in (to get to the next aligned input pointer), the inner
     * loop (aligned strides over the length of the buffers), and the
     * tail (taking care of unaligned data at the end of the buffers).
     *
     * [1] http://en.wikipedia.org/wiki/Circular_buffer
     * [2] For buffers that have a size that is a power of 2
     *     (2, 4, 8, 16, 32, ...), these are equivalent:
     *
     *          if( k >= N ) k = 0;
     *          k &= (N-1);
     *
     * [3] That is, ((intptr_t)ptr % 16) == 0
     * [4] Actually, you can do SSE on unaligned data... but the
     *     performance penalty is usually large enough to defeat
     *     the purpose.
     */
    template< short Order >
    void FilterSSE<Order>::process(const CSAMPLE * __restrict pIn,
                                   const CSAMPLE * __restrict pOut,
                                   const int iBufferSize) __restrict
    {
        CSAMPLE * pOutput = (CSAMPLE *)pOut;
        const CSAMPLE * pInput = pIn;
        int i;
        unsigned short alignment_in;
        unsigned short alignment_out;
        double leftover_in = 0; // When alignement_in % 2 == 1

        // These variables are for local copies of the object's
        // current state.
        unsigned short k; // ring buffer position
        __m128d *CXY_start, *CXY_end;
        __m128d * __restrict CXY;
        __m128d XY[2*ORDER], *xy;
        __m128d GAIN = _gain->v;

        // Assuming that data is complete, interleaved (L,R) pairs
        assert( (iBufferSize % 2) == 0 );
        // Assuming that the buffers do not overlap. (__restrict)
        assert( (pOut >= pInput + iBufferSize) || (pInput >= pOut + iBufferSize) );

        /* POP THE CURRENT OBJECT STATE
         *
         * The state of the OBJECT is popped into local variables.
         * This increases speed and reduces memory/cache slowdowns.
         *
         * xy: The ringbuffers are copied to the stack.  This
         *     increases access speed and reduces cache misses.
         *
         * k: This needs to be a register variable, but if we
         *    use _k, then it will periodically save this->_k
         *    in memory whenever it changes.  This is a big
         *    slow-down.
         *
         * CXY: Because _CXY is a pointer to a pointer, dereferencing
         *      _CXY[k] is actually very slow.
         */
        memcpy(XY, _xv, 2*ORDER*2*sizeof(double));
        k = _k;
        xy = XY;
        assert(k < ORDER);
        CXY_start = (__m128d*)_CXY[0];
        CXY_end = (__m128d*)(_CXY[ORDER-1] + (2*ORDER));
        CXY = (__m128d*)_CXY[k];

        /* LEAD-IN AND ALIGNMENT CALCULATIONS
         */
        const CSAMPLE *pInput_end;
        const int STRIDE = 4;

        alignment_in = ((intptr_t) pInput) & 0xF;

        if( 0 == (alignment_in % 4) ) {
            alignment_in /= 4;
            pInput_end = &pInput[ ((4-alignment_in)&0x3) + STRIDE * ((iBufferSize - alignment_in)/STRIDE) ];
            assert( (iBufferSize - (intptr_t)(pInput_end - pInput)) < 4 );
        } else {
            alignment_in = UNALIGNED;
            pInput_end = pInput + iBufferSize;
        }

        // If output isn't 4-byte aligned... it will stay unaligned.
        // So there's no point in doing aligned stuff.
        if( 0 != (((intptr_t)pOutput) & 0x3) ) {
            alignment_in = UNALIGNED;
            alignment_out = UNALIGNED;
            pInput_end = pInput + iBufferSize;
        }

        // Run calculations once for all the unaligned samples
        // at the beginning.
        if(alignment_in == 3) {
            leftover_in = *pInput++;
        }

        if((alignment_in == 1) || (alignment_in == 2)) {
            v2df in, out;
            unsigned short tmp;

            in.d[0] = *pInput++;
            in.d[1] = *pInput++;
            if( alignment_in == 1 ) {
                leftover_in = *pInput++;
            }

            PROCESS_ONE_SAMPLE(in,out);
            assert( CXY < CXY_end );

            (*pOutput++) = (float)out.d[0];
            (*pOutput++) = (float)out.d[1];
        }

        alignment_out = ((intptr_t) pOutput) & 0xF;
        if( 0 == (alignment_out % 4) ) {
            alignment_out /= 4;
        } else {
            alignment_out = UNALIGNED;
        }

        /* INNER LOOP
         */
        v4sf pending;
        if( (alignment_in != UNALIGNED) && (alignment_out != UNALIGNED) ) {
            if( ((alignment_in & 1) == 0) && (alignment_out == 0) ) {
                CXY = inner_loop_aligned_4_stride<Order>( &pOutput,
                                                          pInput,
                                                          pInput_end,
                                                          k,
                                                          _k_mask,
                                                          GAIN,
                                                          XY,
                                                          CXY,
                                                          CXY_start,
                                                          CXY_end );
            } else {
                CXY = inner_loop_par_aligned_4_stride<Order>( &pOutput,
                                                              pInput,
                                                              pInput_end,
                                                              k,
                                                              _k_mask,
                                                              GAIN,
                                                              XY,
                                                              CXY,
                                                              CXY_start,
                                                              CXY_end,
                                                              alignment_in,
                                                              alignment_out,
                                                              leftover_in,
                                                              pending );
            }
        } else {
            CXY = inner_loop_lame_4_stride<Order>( &pOutput,
                                                   pInput,
                                                   pInput_end,
                                                   k,
                                                   _k_mask,
                                                   GAIN,
                                                   XY,
                                                   CXY,
                                                   CXY_start,
                                                   CXY_end );
        }
        pInput = pInput_end;

        /* TAIL
         *
         * Run calculations for all unaligned samples at the end.
         */
        int leftover = iBufferSize - ((const CSAMPLE*)pInput - (const CSAMPLE*)pIn);
        while( leftover > 0 ) {

            v2df in, out;

            switch(leftover) {
            case 2:
                in.d[0] = (double)(*pInput++);
                in.d[1] = (double)(*pInput++);
                break;
            case 1:
                in.d[0] = leftover_in;
                in.d[1] = (double)(*pInput++);
                break;
            case 3:
                in.d[0] = leftover_in;
                in.d[1] = (double)(*pInput++);
                leftover_in = (double)(*pInput++);
                break;
            default:
                assert(false);
            }
            leftover -= 2;

            PROCESS_ONE_SAMPLE(in, out);
            assert( CXY < CXY_end );

            switch(alignment_out) {
            case 0:
            case UNALIGNED:
                (*pOutput++) = (float)out.d[0];
                (*pOutput++) = (float)out.d[1];
                break;
            case 1:
                (*pOutput++) = pending.f[0];
                (*pOutput++) = (float)out.d[0];
                pending.f[0] = (float)out.d[1];
                break;
            case 2:
                (*pOutput++) = pending.f[0];
                (*pOutput++) = pending.f[1];
                pending.f[0] = (float)out.d[0];
                pending.f[1] = (float)out.d[1];
                break;
            case 3:
                (*pOutput++) = pending.f[0];
                (*pOutput++) = pending.f[1];
                (*pOutput++) = pending.f[2];
                pending.f[0] = (float)out.d[0];         
                pending.f[1] = (float)out.d[1];
                break;
            default:
                assert(false);
            }
        }

        if(alignment_out == UNALIGNED) alignment_out = 0;
        for( int i=0 ; i < alignment_out ; ++i ) {
            (*pOutput++) = pending.f[i];
        }
        assert( (intptr_t)pInput == (intptr_t)(pIn + iBufferSize) );
        assert( (intptr_t)pOutput == (intptr_t)(pOut + iBufferSize) );

        /* PUSH BACK THE CURRENT OBJECT STATE
         */
        _k = k;
        memcpy(_xv, XY, 2*ORDER*sizeof(__m128d));

        // Check for denormals
        for (i=0; i<=ORDER; ++i)
        {
            _xv[i].d[0] = zap_denormal(_xv[i].d[0]);
            _xv[i].d[1] = zap_denormal(_xv[i].d[1]);
            _yv[i].d[0] = zap_denormal(_yv[i].d[0]);
            _yv[i].d[1] = zap_denormal(_yv[i].d[1]);
        }
    }
#endif // IIR_ENABLE_SSE3

    FilterReference::FilterReference(int iOrder, const double* pCoefs)
    {
        order = iOrder;
        coefs = pCoefs;

        // Reset the yv's:
        memset(yv1, 0, sizeof(yv1));
        memset(yv2, 0, sizeof(yv2));
        memset(xv1, 0, sizeof(xv1));
        memset(xv2, 0, sizeof(xv2));
    }

    FilterReference::~FilterReference()
    {
    }

    void FilterReference::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
    {
        CSAMPLE * pOutput = (CSAMPLE *)pOut;
        double GAIN =  coefs[0];
        int i;
        for (i=0; i<iBufferSize; i+=2)
        {
            if (order==8)
            {
                //8th order:
                // Channel 1
                xv1[0] = xv1[1]; xv1[1] = xv1[2]; xv1[2] = xv1[3]; xv1[3] = xv1[4];
                xv1[4] = xv1[5]; xv1[5] = xv1[6]; xv1[6] = xv1[7]; xv1[7] = xv1[8];
                xv1[8] = pIn[i]/GAIN;
                yv1[0] = yv1[1]; yv1[1] = yv1[2]; yv1[2] = yv1[3]; yv1[3] = yv1[4];
                yv1[4] = yv1[5]; yv1[5] = yv1[6]; yv1[6] = yv1[7]; yv1[7] = yv1[8];
                yv1[8] =   (xv1[0] + xv1[8]) + coefs[1] * (xv1[1] + xv1[7]) +
                    coefs[2] * (xv1[2] + xv1[6]) +
                    coefs[3] * (xv1[3] + xv1[5]) + coefs[4] * xv1[4] +
                    (coefs[5] * yv1[0]) + ( coefs[6] * yv1[1]) +
                    (coefs[7] * yv1[2]) + ( coefs[8] * yv1[3]) +
                    (coefs[9] * yv1[4]) + ( coefs[10] * yv1[5]) +
                    (coefs[11] * yv1[6]) + ( coefs[12] * yv1[7]);
                assert(yv1[8]<100000 || yv1[8]>-100000);
                pOutput[i] = yv1[8];

                // Channel 2
                xv2[0] = xv2[1]; xv2[1] = xv2[2]; xv2[2] = xv2[3]; xv2[3] = xv2[4];
                xv2[4] = xv2[5]; xv2[5] = xv2[6]; xv2[6] = xv2[7]; xv2[7] = xv2[8];
                xv2[8] = pIn[i+1]/GAIN;
                yv2[0] = yv2[1]; yv2[1] = yv2[2]; yv2[2] = yv2[3]; yv2[3] = yv2[4];
                yv2[4] = yv2[5]; yv2[5] = yv2[6]; yv2[6] = yv2[7]; yv2[7] = yv2[8];
                yv2[8] =   (xv2[0] + xv2[8]) + coefs[1] * (xv2[1] + xv2[7]) +
                    coefs[2] * (xv2[2] + xv2[6]) +
                    coefs[3] * (xv2[3] + xv2[5]) + coefs[4] * xv2[4] +
                    (coefs[5] * yv2[0]) + ( coefs[6] * yv2[1]) +
                    (coefs[7] * yv2[2]) + ( coefs[8] * yv2[3]) +
                    (coefs[9] * yv2[4]) + ( coefs[10] * yv2[5]) +
                    (coefs[11] * yv2[6]) + ( coefs[12] * yv2[7]);
                assert(yv2[8]<100000 || yv2[8]>-100000);
                pOutput[i+1] = yv2[8];
            }
            else if (order==2)
            {
                // Second order
                xv1[0] = xv1[1]; xv1[1] = xv1[2];
                xv1[2] = pIn[i] / GAIN;
                yv1[0] = yv1[1]; yv1[1] = yv1[2];
                yv1[2] = (xv1[0] + xv1[2]) + coefs[1] * xv1[1] + ( coefs[2] * yv1[0]) + (coefs[3] * yv1[1]);
                pOutput[i] = yv1[2];

                xv2[0] = xv2[1]; xv2[1] = xv2[2];
                xv2[2] = pIn[i+1] / GAIN;
                yv2[0] = yv2[1]; yv2[1] = yv2[2];
                yv2[2] = (xv2[0] + xv2[2]) + coefs[1] * xv2[1] + ( coefs[2] * yv2[0]) + (coefs[3] * yv2[1]);
                pOutput[i+1] = yv2[2];
            }
            else
            {
                // Fourth order
                xv1[0] = xv1[1]; xv1[1] = xv1[2]; xv1[2] = xv1[3]; xv1[3] = xv1[4];
                xv1[4] = pIn[i] / GAIN;
                yv1[0] = yv1[1]; yv1[1] = yv1[2]; yv1[2] = yv1[3]; yv1[3] = yv1[4];
                yv1[4] =   (xv1[0] + xv1[4]) + coefs[1]*(xv1[1]+xv1[3]) + coefs[2] * xv1[2]
                    + ( coefs[3] * yv1[0]) + (  coefs[4] * yv1[1])
                    + ( coefs[5] * yv1[2]) + (  coefs[6] * yv1[3]);
                pOutput[i] = yv1[4];

                xv2[0] = xv2[1]; xv2[1] = xv2[2]; xv2[2] = xv2[3]; xv2[3] = xv2[4];
                xv2[4] = pIn[i+1] / GAIN;
                yv2[0] = yv2[1]; yv2[1] = yv2[2]; yv2[2] = yv2[3]; yv2[3] = yv2[4];
                yv2[4] =   (xv2[0] + xv2[4]) + coefs[1]*(xv2[1]+xv2[3]) + coefs[2] * xv2[2]
                    + ( coefs[3] * yv2[0]) + (  coefs[4] * yv2[1])
                    + ( coefs[5] * yv2[2]) + (  coefs[6] * yv2[3]);
                pOutput[i+1] = yv2[4];
            }
        }

        // Check for denormals
        for (i=0; i<=order; ++i)
        {
            xv1[i] = zap_denormal(xv1[i]);
            yv1[i] = zap_denormal(yv1[i]);
            xv2[i] = zap_denormal(xv2[i]);
            yv2[i] = zap_denormal(yv2[i]);
        }
    }

} // namespace DetailsEngineFilterIIR
