#include "engine/enginefilterbutterworth4.h"
#define MIXXX
#include "fidlib.h"


EngineFilterButterworth4Low::EngineFilterButterworth4Low(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4Low::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    memcpy(m_oldCoef, m_coef, 5 * sizeof(double));
    m_coef[0] = fid_design_coef(m_coef + 1, 4, "LpBu4", sampleRate,
                               freqCorner1, 0, 0);
    initBuffers();
    m_doRamping = true;
}

EngineFilterButterworth4Band::EngineFilterButterworth4Band(int sampleRate, double freqCorner1,
                                         double freqCorner2) {
    setFrequencyCorners(sampleRate, freqCorner1, freqCorner2);
}

void EngineFilterButterworth4Band::setFrequencyCorners(int sampleRate,
                                             double freqCorner1,
                                             double freqCorner2) {
    // Copy the old coefficients into m_oldCoef
    memcpy(m_oldCoef, m_coef, 9 * sizeof(double));
    m_coef[0] = fid_design_coef(m_coef + 1, 8, "BpBu4", sampleRate,
                               freqCorner1, freqCorner2, 0);
    initBuffers();
    m_doRamping = true;
}


EngineFilterButterworth4High::EngineFilterButterworth4High(int sampleRate, double freqCorner1) {
    setFrequencyCorners(sampleRate, freqCorner1);
}

void EngineFilterButterworth4High::setFrequencyCorners(int sampleRate,
                                             double freqCorner1) {
    // Copy the old coefficients into m_oldCoef
    memcpy(m_oldCoef, m_coef, 5 * sizeof(double));
    m_coef[0] = fid_design_coef(m_coef + 1, 4, "HpBu4", sampleRate,
                               freqCorner1, 0, 0);
    initBuffers();
    m_doRamping = true;
}
