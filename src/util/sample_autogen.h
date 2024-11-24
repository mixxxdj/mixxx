#pragma once

struct Input {
    std::span<const CSAMPLE> data;
    CSAMPLE_GAIN gain;
};

struct InputRamped {
    std::span<const CSAMPLE> data;
    struct {
        CSAMPLE_GAIN in;
        CSAMPLE_GAIN out;
    } gain;
};

template<class... Args>
    requires(sizeof...(Args) == 0)
static constexpr std::size_t copyWithGainVariadic(std::span<CSAMPLE> dest) noexcept {
    // This is the base that just clears the buffer if there is no (audible)
    // source to copy from. the empty parameter pack is needed so it is still
    // considered during overload resolution afaict.
    std::fill(dest.begin(), dest.end(), 0);
    return dest.size();
}
template<class... CheckedArgs, class... UncheckedArgs>
// this clause is needed to ensure we only match this case when all `Input`s
// have been checked and there actually are some we should process.
    requires(sizeof...(UncheckedArgs) == 0 && sizeof...(CheckedArgs) > 0)
static constexpr std::size_t copyWithGainVariadic(std::span<CSAMPLE> dest,
        CheckedArgs&&... cSrcs,
        UncheckedArgs&&... uSrcs) noexcept {
    // This code gets called when all the checks have been emitted and we now
    // know all gains are non-zero. because we could potentially be handling
    // different buffers, we quickly find the minimum...
    std::size_t num = std::min({dest.size(), cSrcs.data.size()...});
    for (std::size_t i = 0; i < num; ++i) {
        // ...and then sum all inputs along with their gain.
        // note: loop vectorized
        dest[i] = ((cSrcs.data[i] * cSrcs.gain) + ...);
    }
    // then we return how many samples we processed.
    return num;
}
template<class... CheckedArgs, class... UncheckedArgs>
static constexpr std::size_t copyWithGainVariadic(std::span<CSAMPLE> dest,
        CheckedArgs&&... cSrcs,
        Input src,
        UncheckedArgs&&... uSrcs) noexcept {
    // The trick here is that function parameters that get deduced automatically
    // are actually in `UncheckedArgs` while the explicitly specified are part
    // of the `CheckedArgs` pack. This enables us to generate a function that
    // checks each source exactly once and doesn't consider it when inaudible.
    //
    // This checks that the first `src` is actually audible
    if (src.gain == 0) {
        // if not audible, then we remove that from the set of buffers and call
        // a version with one less Input to handle
        return copyWithGainVariadic<CheckedArgs...>(dest,
                std::forward<CheckedArgs>(cSrcs)...,
                std::forward<UncheckedArgs>(uSrcs)...);
    } else {
        // if it is actually audible, we add it to the set of CheckedArgs and
        // continue checking the rest
        return copyWithGainVariadic<CheckedArgs..., decltype(src)>(dest,
                std::forward<CheckedArgs>(cSrcs)...,
                std::move(src),
                std::forward<UncheckedArgs>(uSrcs)...);
    }
    // Due to tail-recursion the resulting binary models an if-cascade (checking
    // each `src`) with a final clause doing the copy operation implemented
    // above.
}

template<class... CheckedArgs, class... UncheckedArgs>
    requires(sizeof...(UncheckedArgs) == 0 && sizeof...(CheckedArgs) > 0)
static constexpr std::size_t copyWithGainVariadicRamped(
        std::span<CSAMPLE> dest, CheckedArgs... cSrcs, UncheckedArgs... uSrcs) {
    std::size_t num = std::min({dest.size(), cSrcs.data.size()...}) / 2;
    std::array<RampingValue<CSAMPLE>, sizeof...(CheckedArgs)> ramps = {
            RampingValue{
                    cSrcs.gain.in, cSrcs.gain.out, static_cast<int>(num)}...};
    ([&]<std::size_t... Is>(std::index_sequence<Is...>) {
        for (std::size_t i = 0; i < num; ++i) {
            dest[i * 2] = ((cSrcs.data[i * 2] * ramps[Is].getNth(num)) + ...);
            dest[i * 2 + 1] = ((cSrcs.data[i * 2 + 1] * ramps[Is].getNth(num)) + ...);
        }
    })(std::index_sequence_for<CheckedArgs...>{});
    return num;
}

template<class... Args>
    requires(sizeof...(Args) == 0)
static constexpr std::size_t copyWithGainVariadicRamped(std::span<CSAMPLE> dest) noexcept {
    return copyWithGainVariadic(dest);
}

template<class... CheckedArgs, class... UncheckedArgs>
static constexpr std::size_t copyWithGainVariadicRamped(std::span<CSAMPLE> dest,
        CheckedArgs&&... cSrcs,
        InputRamped src,
        UncheckedArgs&&... uSrcs) noexcept {
    if (src.gain.in == 0 && src.gain.out == 0) {
        return copyWithGainVariadicRamped<CheckedArgs...>(dest,
                std::forward<CheckedArgs>(cSrcs)...,
                std::forward<UncheckedArgs>(uSrcs)...);
    } else {
        return copyWithGainVariadicRamped<CheckedArgs..., decltype(src)>(dest,
                std::forward<CheckedArgs>(cSrcs)...,
                std::move(src),
                std::forward<UncheckedArgs>(uSrcs)...);
    }
}

static inline void copy1WithGain(float* pDest,
        const float* pSrc0,
        float gain0,
        int iNumSamples) {
    auto dest = std::span(pDest, iNumSamples);
    Input src = {std::span(pSrc0, iNumSamples), gain0};
    copyWithGainVariadic(dest, src);
}

static inline void copy1WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        int iNumSamples) {
    auto dest = std::span(pDest, iNumSamples);
    InputRamped src = {std::span(pSrc0, iNumSamples), {gain0in, gain0out}};
    copyWithGainVariadicRamped(dest, src);
}
static inline void copy2WithGain(float* pDest,
        const float* pSrc0,
        float gain0,
        const float* pSrc1,
        float gain1,
        int iNumSamples) {
    auto dest = std::span(pDest, iNumSamples);
    Input src1 = {std::span(pSrc0, iNumSamples), gain0};
    Input src2 = {std::span(pSrc1, iNumSamples), gain1};
    copyWithGainVariadic(dest, src1, src2);
}

static inline void copy2WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        int iNumSamples) {
    std::size_t samples = iNumSamples;
    std::span<CSAMPLE> dest = {pDest, samples};
    InputRamped in1 = {{pSrc0, samples}, {gain0in, gain0out}};
    InputRamped in2 = {{pSrc1, samples}, {gain1in, gain1out}};
    copyWithGainVariadicRamped(dest, in1, in2);
}
static inline void copy3WithGain(CSAMPLE* M_RESTRICT pDest,
                                 const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0,
                                 const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1,
                                 const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2,
                                 int iNumSamples) {
    auto dest = std::span(pDest, iNumSamples);
    Input src1 = {std::span(pSrc0, iNumSamples), gain0};
    Input src2 = {std::span(pSrc1, iNumSamples), gain1};
    Input src3 = {std::span(pSrc2, iNumSamples), gain2};
    copyWithGainVariadic(dest, src1, src2, src3);
}
static inline void copy3WithRampingGain(CSAMPLE* M_RESTRICT pDest,
                                        const CSAMPLE* M_RESTRICT pSrc0, CSAMPLE_GAIN gain0in, CSAMPLE_GAIN gain0out,
                                        const CSAMPLE* M_RESTRICT pSrc1, CSAMPLE_GAIN gain1in, CSAMPLE_GAIN gain1out,
                                        const CSAMPLE* M_RESTRICT pSrc2, CSAMPLE_GAIN gain2in, CSAMPLE_GAIN gain2out,
                                        int iNumSamples) {
    std::size_t samples = iNumSamples;
    std::span<CSAMPLE> dest = {pDest, samples};
    InputRamped in1 = {{pSrc0, samples}, {gain0in, gain0out}};
    InputRamped in2 = {{pSrc1, samples}, {gain1in, gain1out}};
    InputRamped in3 = {{pSrc2, samples}, {gain2in, gain2out}};
    copyWithGainVariadicRamped(dest, in1, in2, in3);
}
