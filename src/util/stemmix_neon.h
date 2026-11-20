// NEON versions of the multichannel (stem) helpers of SampleUtil (AArch64).
//
// The scalar loop of SampleUtil::mixMultichannelToStereo (4 passes over the
// output for a deck with 4 stems, 2 floats per iteration, "LOOP VECTORIZED" in
// the comment but the compiler left it scalar: fadd s0 in the disassembly) was
// 5.7 % of the audio thread with two stem decks playing (simpleperf, Snapdragon
// 8 Gen 1).
//
//   dest[2 * f + k] = (((src[N * f + k]) + src[N * f + 2 + k]) + ...) over the
//   stems that are not excluded, k = 0, 1 (left, right)
//
// The sums are done in the same order as the scalar loop (stem 0 first), so the
// result is bit identical to the scalar loop compiled without -ffast-math.
#pragma once

#include <cstddef>

#if defined(__aarch64__)
#include <arm_neon.h>
#define MIXXX_STEMMIX_NEON 1
#endif

namespace mixxx {
namespace neon {

#ifdef MIXXX_STEMMIX_NEON

// Four stems (8 channels), none excluded: two frames per round.
inline void mixFourStemsToStereo(float* dest, const float* src, std::ptrdiff_t numFrames) {
    std::ptrdiff_t f = 0;
    for (; f + 2 <= numFrames; f += 2) {
        const float* p = src + f * 8;
        // a = (L0 R0 | L1 R1), b = (L2 R2 | L3 R3) of one frame; the 64 bit halves are
        // the stems. uzp1 collects stem 0 and 2 of the two frames, uzp2 stem 1 and 3.
        const float64x2_t a0 = vreinterpretq_f64_f32(vld1q_f32(p));
        const float64x2_t b0 = vreinterpretq_f64_f32(vld1q_f32(p + 4));
        const float64x2_t a1 = vreinterpretq_f64_f32(vld1q_f32(p + 8));
        const float64x2_t b1 = vreinterpretq_f64_f32(vld1q_f32(p + 12));
        const float32x4_t s0 = vreinterpretq_f32_f64(vuzp1q_f64(a0, a1));
        const float32x4_t s1 = vreinterpretq_f32_f64(vuzp2q_f64(a0, a1));
        const float32x4_t s2 = vreinterpretq_f32_f64(vuzp1q_f64(b0, b1));
        const float32x4_t s3 = vreinterpretq_f32_f64(vuzp2q_f64(b0, b1));
        vst1q_f32(dest + f * 2, vaddq_f32(vaddq_f32(vaddq_f32(s0, s1), s2), s3));
    }
    for (; f < numFrames; f++) {
        const float* p = src + f * 8;
        float32x2_t acc = vld1_f32(p);
        acc = vadd_f32(acc, vld1_f32(p + 2));
        acc = vadd_f32(acc, vld1_f32(p + 4));
        acc = vadd_f32(acc, vld1_f32(p + 6));
        vst1_f32(dest + f * 2, acc);
    }
}

// Any even channel count (stems = channels / 2, at most 16), with the stems of
// excludeMask (bit i = stem i) left out. false: not supported, use the scalar loop.
inline bool mixStemsToStereo(float* dest,
        const float* src,
        std::ptrdiff_t numFrames,
        int numChannels,
        int excludeMask) {
    if (numChannels < 4 || (numChannels & 1) != 0 || numChannels > 32) {
        return false;
    }
    if (numChannels == 8 && excludeMask == 0) {
        mixFourStemsToStereo(dest, src, numFrames);
        return true;
    }
    const int stems = numChannels / 2;
    int offsets[16];
    int count = 0;
    for (int s = 0; s < stems; s++) {
        if (((excludeMask >> s) & 1) == 0) {
            offsets[count++] = 2 * s;
        }
    }
    for (std::ptrdiff_t f = 0; f < numFrames; f++) {
        const float* p = src + f * numChannels;
        float32x2_t acc = vdup_n_f32(0.0f);
        for (int k = 0; k < count; k++) {
            acc = vadd_f32(acc, vld1_f32(p + offsets[k]));
        }
        vst1_f32(dest + f * 2, acc);
    }
    return true;
}

// SampleUtil::copyOneStereoFromMulti: dest (stereo) = the stereo pair at
// sourceChannel of every frame
inline void copyStereoFromMulti(float* dest,
        const float* src,
        std::ptrdiff_t numFrames,
        int numChannels,
        int sourceChannel) {
    const std::ptrdiff_t n = numChannels;
    const float* p = src + sourceChannel;
    std::ptrdiff_t f = 0;
    for (; f + 4 <= numFrames; f += 4) {
        const float32x2_t a = vld1_f32(p);
        const float32x2_t b = vld1_f32(p + n);
        const float32x2_t c = vld1_f32(p + 2 * n);
        const float32x2_t d = vld1_f32(p + 3 * n);
        vst1q_f32(dest + f * 2, vcombine_f32(a, b));
        vst1q_f32(dest + f * 2 + 4, vcombine_f32(c, d));
        p += 4 * n;
    }
    for (; f < numFrames; f++) {
        vst1_f32(dest + f * 2, vld1_f32(p));
        p += n;
    }
}

// SampleUtil::insertStereoToMulti: the stereo pair at channelOffset of every frame of dest = src
inline void insertStereoToMulti(float* dest,
        const float* src,
        std::ptrdiff_t numFrames,
        int numChannels,
        int channelOffset) {
    const std::ptrdiff_t n = numChannels;
    float* p = dest + channelOffset;
    std::ptrdiff_t f = 0;
    for (; f + 4 <= numFrames; f += 4) {
        const float32x4_t a = vld1q_f32(src + f * 2);
        const float32x4_t b = vld1q_f32(src + f * 2 + 4);
        vst1_f32(p, vget_low_f32(a));
        vst1_f32(p + n, vget_high_f32(a));
        vst1_f32(p + 2 * n, vget_low_f32(b));
        vst1_f32(p + 3 * n, vget_high_f32(b));
        p += 4 * n;
    }
    for (; f < numFrames; f++) {
        vst1_f32(p, vld1_f32(src + f * 2));
        p += n;
    }
}

#endif // MIXXX_STEMMIX_NEON

} // namespace neon
} // namespace mixxx
