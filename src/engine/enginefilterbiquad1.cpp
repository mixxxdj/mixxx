#include <QLocale>
#include "engine/enginefilterbiquad1.h"

EngineFilterBiquad1Low::EngineFilterBiquad1Low(int sampleRate,
                                               double centerFreq, double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1Low::setFrequencyCorners(int sampleRate,
                                                 double centerFreq,
                                                 double Q,
                                                 double dBgain) {
    QString specification = "LsBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1Band::EngineFilterBiquad1Band(int sampleRate,
                                                 double centerFreq, double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1Band::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q,
                                                  double dBgain) {
    QString specification = "PkBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1High::EngineFilterBiquad1High(int sampleRate,
                                                 double centerFreq, double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1High::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q,
                                                  double dBgain) {
    QString specification = "HsBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}
