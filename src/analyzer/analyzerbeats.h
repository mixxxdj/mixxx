/* Beat Tracking test via vamp-plugins
 * analyzerbeats.h
 *
 *  Created on: 16/mar/2011
 *      Author: Vittorio Colao
 */

#ifndef ANALYZER_ANALYZERBEATS_H
#define ANALYZER_ANALYZERBEATS_H

#include <QHash>

#include "analyzer/analyzer.h"
#include "analyzer/vamp/vampanalyzer.h"
#include "preferences/usersettings.h"

class AnalyzerBeats: public Analyzer {
  public:
    AnalyzerBeats(UserSettingsPointer pConfig);
    virtual ~AnalyzerBeats();

    bool initialize(TrackPointer tio, int sampleRate, int totalSamples) override;
    bool loadStored(TrackPointer tio) const override;
    void process(const CSAMPLE *pIn, const int iLen) override;
    void cleanup(TrackPointer tio) override;
    void finalize(TrackPointer tio) override;

  private:
    static QHash<QString, QString> getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis);
    QVector<double> correctedBeats(QVector<double> rawbeats);

    UserSettingsPointer m_pConfig;
    VampAnalyzer* m_pVamp;
    QString m_pluginId;
    bool m_bPreferencesReanalyzeOldBpm;
    bool m_bPreferencesFixedTempo;
    bool m_bPreferencesOffsetCorrection;
    bool m_bPreferencesFastAnalysis;

    int m_iSampleRate, m_iTotalSamples;
    int m_iMinBpm, m_iMaxBpm;
};

#endif /* ANALYZER_ANALYZERBEATS_H */
