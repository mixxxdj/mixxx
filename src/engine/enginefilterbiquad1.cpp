#include <QLocale>
#include "engine/enginefilterbiquad1.h"

EngineFilterBiquad1LowShelving::EngineFilterBiquad1LowShelving(int sampleRate,
                                                               double centerFreq,
                                                               double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1LowShelving::setFrequencyCorners(int sampleRate,
                                                         double centerFreq,
                                                         double Q,
                                                         double dBgain) {
    QString specification = "LsBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1Peaking::EngineFilterBiquad1Peaking(int sampleRate,
                                                       double centerFreq, double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1Peaking::setFrequencyCorners(int sampleRate,
                                                     double centerFreq,
                                                     double Q,
                                                     double dBgain) {
    QString specification = "PkBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1HighShelving::EngineFilterBiquad1HighShelving(int sampleRate,
                                                                 double centerFreq,
                                                                 double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q, 0);
}

void EngineFilterBiquad1HighShelving::setFrequencyCorners(int sampleRate,
                                                          double centerFreq,
                                                          double Q,
                                                          double dBgain) {
    QString specification = "HsBq/" + QLocale().toString(Q) + "/" +
                            QLocale().toString(dBgain);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}
