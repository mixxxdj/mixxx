#pragma once

// -------------------------------------------------------------------
// EngineStemMixer — RT-safe 8ch -> stereo stem downmixer (S3).
//
// Downmixes one 8-channel interleaved stem frame buffer into stereo:
//
//   frame layout (file order, see StemMp4Writer / SoundSourceSTEM):
//     [VL VR | DL DR | BL BR | OL OR]   (4 stereo stems, 8 CSAMPLEs)
//
//   out_L = VL*g0 + DL*g1 + BL*g2 + OL*g3
//   out_R = VR*g0 + DR*g1 + BR*g2 + OR*g3
//
// API estable (dia-1): S4 (stem ControlObjects) solo necesita incluir
// este header; la firma no cambiara.
//
// Garantias de tiempo real (audio thread, SCHED_FIFO):
//   - process()/processAVX2()/processScalar() no hacen heap, no toman
//     locks y no hacen syscalls (solo stack + loads/stores + ALU).
//   - Los buffers los pre-aloca el llamante (EngineDeck::m_stemBuffer,
//     pOut del engine). Si se necesita scratch alineado, usar
//     `alignas(32)` en stack o SampleBuffer pre-alocado fuera del
//     callback; nunca `new`/`malloc` dentro de process().
//   - La deteccion de CPU (hasAVX2) se resuelve una sola vez
//     (function-local static); llamadas posteriores son un load.
//
// SIMD:
//   - El kernel AVX2 usa __m256 _mm256_mul_ps + reduccion estereo con
//     shuffles (ver .cpp). FMA se usa en la interpolacion de la rampa
//     de ganancia (std::fma -> VFMADD213SS con -mfma).
//   - process() despacha a AVX2 solo si hasAVX2(); si no, scalar.
//     Fallback escalar: `!__builtin_cpu_supports("avx2")` o
//     `!__builtin_cpu_supports("fma")` (o CPU no-x86).
//   - Este TU se compila con -mavx2 -mfma SOLO para enginestemmixer.cpp
//     (set_source_files_properties en CMakeLists, nunca global). En x86
//     el llamante debe comprobar hasAVX2() antes de process() porque el
//     codegen de este TU puede emitir AVX2 incluso en la rama escalar.
//     EngineDeck::processStem() ya lo hace (legacy path si !hasAVX2).
//
// Rampa click-free:
//   - process() interpola linealmente por frame desde oldGains hasta
//     newGains con la misma convencion que SampleUtil::applyRampingGain
//     (frame i usa old + step*(i+1), step = (new-old)/numFrames; el
//     ultimo frame cae exactamente en new). EngineDeck pasa
//     m_stemsGainCache como oldGains y lo actualiza con newGains,
//     igual que hacia processPostFaderInPlace.
//   - Sobrecargas de ganancia constante (un solo array) para mezcla
//     directa sin rampa (firma pedida: processAVX2(out, in, gains, frames)).
// -------------------------------------------------------------------

#include <array>

#include "util/platform.h" // M_RESTRICT
#include "util/types.h"    // CSAMPLE, CSAMPLE_GAIN, SINT

class EngineStemMixer {
  public:
    static constexpr int kNumStems = 4;
    static constexpr int kNumChannels = 8; // 4 stems stereo interleaved
    static constexpr int kStereoChannels = 2;

    // Ganancia por stem [vocals, drums, bass, other] en orden de archivo.
    using GainArray = std::array<CSAMPLE_GAIN, kNumStems>;

    EngineStemMixer() = delete;

    // Deteccion runtime de AVX2+FMA (+ OS support). Barato tras la
    // primera llamada (resultado cacheado). Falso en no-x86 o si este
    // TU se compilo sin AVX2.
    static bool hasAVX2() noexcept;

    // Despacho RT: AVX2 si hasAVX2(), escalar si no. Con rampa
    // click-free oldGains -> newGains.
    static void process(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& oldGains,
            const GainArray& newGains,
            SINT numFrames) noexcept;

    // Mezcla directa con ganancia constante (sin rampa).
    static void process(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& gains,
            SINT numFrames) noexcept;

    // Kernel AVX2 (__m256 _mm256_mul_ps + reduccion estereo). Requiere
    // hasAVX2(); si el TU se compilo sin AVX2 delega en escalar.
    // Con rampa click-free oldGains -> newGains.
    static void processAVX2(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& oldGains,
            const GainArray& newGains,
            SINT numFrames) noexcept;

    // Sobrecarga de ganancia constante (firma S3).
    static void processAVX2(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& gains,
            SINT numFrames) noexcept;

    // Referencia escalar (tambien fallback runtime). Misma convencion
    // de rampa y mismo orden de suma que el kernel AVX2 para que la
    // diferencia quede en tolerancia 1e-6 (tipicamente bit-exacta).
    static void processScalar(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& oldGains,
            const GainArray& newGains,
            SINT numFrames) noexcept;

    static void processScalar(
            CSAMPLE* M_RESTRICT pOutStereo,
            const CSAMPLE* M_RESTRICT pIn8Ch,
            const GainArray& gains,
            SINT numFrames) noexcept;
};
