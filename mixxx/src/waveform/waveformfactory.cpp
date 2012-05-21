#include <QtDebug>

#include "waveform/waveformfactory.h"
#include "waveform/waveform.h"

// static
Waveform* WaveformFactory::loadWaveformFromAnalysis(
    TrackPointer pTrack, const AnalysisDao::AnalysisInfo& analysis) {
    if (analysis.version == WAVEFORM_2_VERSION ||
        analysis.version == WAVEFORMSUMMARY_2_VERSION) {
        Waveform* pWaveform = new Waveform(analysis.data);
        pWaveform->setId(analysis.analysisId);
        pWaveform->setVersion(analysis.version);
        pWaveform->setDescription(analysis.description);
        return pWaveform;
    }
    qDebug() << "Skipping unsupported waveform version:" << analysis.version;
    return NULL;
}

// static
QString WaveformFactory::getPreferredWaveformVersion() {
    return WAVEFORM_2_VERSION;
}

// static
QString WaveformFactory::getPreferredWaveformSummaryVersion() {
    return WAVEFORMSUMMARY_2_VERSION;
}
