// EngineStemMixer — RT-safe 8ch -> stereo stem downmixer (S3).
//
// NOTA DE BUILD: este es el UNICO TU que se compila con -mavx2 -mfma
// (set_source_files_properties en CMakeLists, nunca flags globales).
// Todo el codigo AVX2 vive aqui; el header es C++20 portable para que
// enginedeck.cpp y S4 lo incluyan sin flags especiales.
//
// RT: ninguna funcion de este archivo reserva heap, toma locks ni hace
// syscalls en el path de proceso (stack + SIMD + ALU). hasAVX2()
// cachea el resultado del cpuid en un static local.

#include "engine/stems/enginestemmixer.h"

#include <cmath> // std::fma

#if defined(__AVX2__) && defined(__FMA__)
#include <immintrin.h>
#define MIXX_STEMMIXER_AVX2 1
#else
#define MIXX_STEMMIXER_AVX2 0
#endif

#if MIXX_STEMMIXER_AVX2 && defined(_MSC_VER)
#include <intrin.h>
#endif

namespace {

#if MIXX_STEMMIXER_AVX2
// always_inline: en builds sin optimizar (Debug/-O0) el compilador no
// inlinea por defecto y pasar __m256 por stack por frame destruye el
// speedup; el hot loop RT debe inlinear siempre.
#if defined(_MSC_VER)
#define MIXX_STEMMIXER_INLINE __forceinline
#else
#define MIXX_STEMMIXER_INLINE inline __attribute__((always_inline))
#endif
// Vector de ganancias por frame: lanes [g0 g0 g1 g1 g2 g2 g3 g3]
// alineados con el layout [VL VR DL DR BL BR OL OR].
MIXX_STEMMIXER_INLINE __m256 makeGainVec(float g0, float g1, float g2, float g3) noexcept {
    return _mm256_setr_ps(g0, g0, g1, g1, g2, g2, g3, g3);
}

// Reduce 8 productos [VL*g0 VR*g0 DL*g1 DR*g1 BL*g2 BR*g2 OL*g3 OR*g3]
// a [L R L R] con L = (p0+p4)+(p2+p6), R = (p1+p5)+(p3+p7).
MIXX_STEMMIXER_INLINE __m128 reduceStereo(__m256 p) noexcept {
    const __m128 lo = _mm256_castps256_ps128(p);
    const __m128 hi = _mm256_extractf128_ps(p, 1);
    const __m128 s = _mm_add_ps(lo, hi);
    const __m128 sh = _mm_shuffle_ps(s, s, _MM_SHUFFLE(1, 0, 3, 2));
    return _mm_add_ps(s, sh);
}

MIXX_STEMMIXER_INLINE __m256 rampGainVec(
        __m256 oldDup, __m256 stepDup, const float* pFrameIdx1) noexcept {
    // 1 broadcast-load (puerto de carga, no shuffle) + 1 VFMADDPS por
    // frame. Correctamente redondeado por lane, identico al std::fma
    // escalar -> bit-exactitud frente a processScalar.
    return _mm256_fmadd_ps(stepDup, _mm256_broadcast_ss(pFrameIdx1), oldDup);
}
#endif // MIXX_STEMMIXER_AVX2

} // namespace

bool EngineStemMixer::hasAVX2() noexcept {
#if MIXX_STEMMIXER_AVX2 && defined(__GNUC__) && \
        (defined(__x86_64__) || defined(__i386__))
    // __builtin_cpu_supports incluye el chequeo de OS (OSXSAVE+XCR0).
    static const bool cached = []() {
        __builtin_cpu_init();
        return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
    }();
    return cached;
#elif MIXX_STEMMIXER_AVX2 && defined(_MSC_VER)
    static const bool cached = []() {
        int info[4];
        __cpuid(info, 0);
        if (info[0] < 1) {
            return false;
        }
        __cpuid(info, 1);
        const bool fma = (info[2] & (1 << 12)) != 0;
        const bool osxsave = (info[2] & (1 << 27)) != 0;
        const bool avx = (info[2] & (1 << 28)) != 0;
        if (!fma || !osxsave || !avx) {
            return false;
        }
        if ((_xgetbv(0) & 0x6) != 0x6) {
            return false; // XMM+YMM no habilitados por el OS
        }
        __cpuidex(info, 7, 0);
        return (info[1] & (1 << 5)) != 0; // AVX2
    }();
    return cached;
#else
    return false;
#endif
}

void EngineStemMixer::process(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& oldGains,
        const GainArray& newGains,
        SINT numFrames) noexcept {
    if (hasAVX2()) {
        processAVX2(pOutStereo, pIn8Ch, oldGains, newGains, numFrames);
    } else {
        processScalar(pOutStereo, pIn8Ch, oldGains, newGains, numFrames);
    }
}

void EngineStemMixer::process(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& gains,
        SINT numFrames) noexcept {
    process(pOutStereo, pIn8Ch, gains, gains, numFrames);
}

void EngineStemMixer::processAVX2(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& oldGains,
        const GainArray& newGains,
        SINT numFrames) noexcept {
#if !MIXX_STEMMIXER_AVX2
    // TU compilado sin AVX2 (p. ej. ARM): delega en escalar.
    processScalar(pOutStereo, pIn8Ch, oldGains, newGains, numFrames);
    return;
#else
    if (numFrames <= 0) {
        return;
    }
    // Misma convencion que SampleUtil::applyRampingGain: el frame i
    // usa old + step*(i+1) con step = (new-old)/numFrames, de modo que
    // el ultimo frame cae exactamente en new (click-free).
    const float invFrames = 1.0f / static_cast<float>(numFrames);
    float step[kNumStems];
    bool uniform = true;
    for (int s = 0; s < kNumStems; ++s) {
        step[s] = (newGains[s] - oldGains[s]) * invFrames;
        uniform = uniform && (oldGains[s] == newGains[s]);
    }

    SINT i = 0;
    if (uniform) {
        // Fast path: un solo vector de ganancias para todo el buffer.
        const __m256 g = makeGainVec(
                newGains[0], newGains[1], newGains[2], newGains[3]);
        // Bloques de 2 frames: 2x__m256 in -> 1x__m128 out [L0 R0 L1 R1].
        for (; i + 1 < numFrames; i += 2) {
            const __m256 v0 = _mm256_loadu_ps(pIn8Ch + i * kNumChannels);
            const __m256 v1 = _mm256_loadu_ps(pIn8Ch + (i + 1) * kNumChannels);
            const __m128 lr0 = reduceStereo(_mm256_mul_ps(v0, g));
            const __m128 lr1 = reduceStereo(_mm256_mul_ps(v1, g));
            _mm_storeu_ps(pOutStereo + i * kStereoChannels,
                    _mm_shuffle_ps(lr0, lr1, _MM_SHUFFLE(1, 0, 1, 0)));
        }
        for (; i < numFrames; ++i) {
            const __m256 v = _mm256_loadu_ps(pIn8Ch + i * kNumChannels);
            const __m128 lr = reduceStereo(_mm256_mul_ps(v, g));
            _mm_store_ss(pOutStereo + i * kStereoChannels, lr);
            _mm_store_ss(pOutStereo + i * kStereoChannels + 1,
                    _mm_shuffle_ps(lr, lr, _MM_SHUFFLE(1, 1, 1, 1)));
        }
        return;
    }

    const __m256 oldDup = makeGainVec(
            oldGains[0], oldGains[1], oldGains[2], oldGains[3]);
    const __m256 stepDup = makeGainVec(step[0], step[1], step[2], step[3]);
    // Contador float exacto (i+1 < 2^24 siempre en buffers de audio):
    // evita 2x vcvtsi2ss por bloque; fi coincide bit a bit con
    // static_cast<float>(i+1) del escalar -> bit-exactitud intacta.
    float fi = 1.0f;
    for (; i + 1 < numFrames; i += 2, fi += 2.0f) {
        const float fi1 = fi + 1.0f;
        const __m256 v0 = _mm256_loadu_ps(pIn8Ch + i * kNumChannels);
        const __m256 v1 = _mm256_loadu_ps(pIn8Ch + (i + 1) * kNumChannels);
        const __m256 g0 = rampGainVec(oldDup, stepDup, &fi);
        const __m256 g1 = rampGainVec(oldDup, stepDup, &fi1);
        const __m128 lr0 = reduceStereo(_mm256_mul_ps(v0, g0));
        const __m128 lr1 = reduceStereo(_mm256_mul_ps(v1, g1));
        _mm_storeu_ps(pOutStereo + i * kStereoChannels,
                _mm_shuffle_ps(lr0, lr1, _MM_SHUFFLE(1, 0, 1, 0)));
    }
    for (; i < numFrames; ++i, fi += 1.0f) {
        const __m256 v = _mm256_loadu_ps(pIn8Ch + i * kNumChannels);
        const __m256 g = rampGainVec(oldDup, stepDup, &fi);
        const __m128 lr = reduceStereo(_mm256_mul_ps(v, g));
        _mm_store_ss(pOutStereo + i * kStereoChannels, lr);
        _mm_store_ss(pOutStereo + i * kStereoChannels + 1,
                _mm_shuffle_ps(lr, lr, _MM_SHUFFLE(1, 1, 1, 1)));
    }
#endif // MIXX_STEMMIXER_AVX2
}

void EngineStemMixer::processAVX2(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& gains,
        SINT numFrames) noexcept {
    processAVX2(pOutStereo, pIn8Ch, gains, gains, numFrames);
}

// Referencia genuinamente escalar: se desactiva la autovectorizacion para
// que el bench AVX2-vs-escalar compare instrucciones SIMD contra
// instrucciones escalares (si no, con -mavx2 -O3 el compilador vectoriza
// tambien la referencia y la comparacion deja de ser "escalar").
// El fallback runtime sigue siendo correcto en cualquier CPU.
#if defined(__GNUC__) && !defined(_MSC_VER)
#pragma GCC push_options
#pragma GCC optimize("no-tree-vectorize")
#endif

void EngineStemMixer::processScalar(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& oldGains,
        const GainArray& newGains,
        SINT numFrames) noexcept {
    if (numFrames <= 0) {
        return;
    }
    const float invFrames = 1.0f / static_cast<float>(numFrames);
    float step[kNumStems];
    for (int s = 0; s < kNumStems; ++s) {
        step[s] = (newGains[s] - oldGains[s]) * invFrames;
    }
    for (SINT i = 0; i < numFrames; ++i) {
        const float fi = static_cast<float>(i + 1);
        const float g0 = std::fma(step[0], fi, oldGains[0]);
        const float g1 = std::fma(step[1], fi, oldGains[1]);
        const float g2 = std::fma(step[2], fi, oldGains[2]);
        const float g3 = std::fma(step[3], fi, oldGains[3]);
        const CSAMPLE* const f = pIn8Ch + i * kNumChannels;
        // Mismo orden de suma que reduceStereo() para maxima coincidencia.
        const float p0 = f[0] * g0;
        const float p1 = f[1] * g0;
        const float p2 = f[2] * g1;
        const float p3 = f[3] * g1;
        const float p4 = f[4] * g2;
        const float p5 = f[5] * g2;
        const float p6 = f[6] * g3;
        const float p7 = f[7] * g3;
        pOutStereo[i * kStereoChannels] = (p0 + p4) + (p2 + p6);
        pOutStereo[i * kStereoChannels + 1] = (p1 + p5) + (p3 + p7);
    }
}

void EngineStemMixer::processScalar(
        CSAMPLE* M_RESTRICT pOutStereo,
        const CSAMPLE* M_RESTRICT pIn8Ch,
        const GainArray& gains,
        SINT numFrames) noexcept {
    processScalar(pOutStereo, pIn8Ch, gains, gains, numFrames);
}

#if defined(__GNUC__) && !defined(_MSC_VER)
#pragma GCC pop_options
#endif
