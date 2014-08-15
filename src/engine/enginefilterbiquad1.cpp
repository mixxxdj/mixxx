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

EngineFilterBiquad1Low::EngineFilterBiquad1Low(int sampleRate,
                                               double centerFreq,
                                               double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1Low::setFrequencyCorners(int sampleRate,
                                                 double centerFreq,
                                                 double Q) {
    QString specification = "LpBq/" + QLocale().toString(Q);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1Band::EngineFilterBiquad1Band(int sampleRate,
                                                 double centerFreq,
                                                 double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1Band::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q) {
    QString specification = "BpBq/" + QLocale().toString(Q);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}

EngineFilterBiquad1High::EngineFilterBiquad1High(int sampleRate,
                                                 double centerFreq,
                                                 double Q) {
    setFrequencyCorners(sampleRate, centerFreq, Q);
}

void EngineFilterBiquad1High::setFrequencyCorners(int sampleRate,
                                                  double centerFreq,
                                                  double Q) {
    QString specification = "HpBq/" + QLocale().toString(Q);
    setCoefs(qPrintable(specification), sampleRate, centerFreq);
}
